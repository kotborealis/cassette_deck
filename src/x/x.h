/* See LICENSE for licence details. */
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "../3rd_party/gifenc/gifenc.h"
#include <sys/time.h>

long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

extern uint32_t color_list[COLORS];
extern uint8_t color_list_u8[COLORS * 3];

#define XK_NO_MOD UINT_MAX

struct xwindow_t {
	Display *display;
	Window window;
	Pixmap pixbuf;
	Colormap cmap;
	GC gc;
	int width, height;
	int screen;
	unsigned long color_palette[COLORS];
};

#define PXCACHE_SIZE (512)
struct pxcache_entry {
	uint32_t color;
	unsigned long pixel;
};
static struct pxcache_entry pxcache[PXCACHE_SIZE];
static int pxcache_top = 0;

bool search_pxcache(uint32_t color, unsigned long *pixel)
{
	int i;
	/* usually faster to search recently allocated colors first */
	for (i = pxcache_top - 1; i >= 0; i--) {
		if (pxcache[i].color == color) {
			*pixel = pxcache[i].pixel;
			return true;
		}
	}
	return false;
}


unsigned long color2pixel(struct xwindow_t *xw, uint32_t color)
{
#ifndef HEADLESS
	unsigned long pixel;
	if (search_pxcache(color, &pixel)) {
		return pixel;
	} else {
		XColor xc;

		xc.red   = ((color >> 16) & bit_mask[8]) << 8;
		xc.green = ((color >>  8) & bit_mask[8]) << 8;
		xc.blue  = ((color >>  0) & bit_mask[8]) << 8;

		if (!XAllocColor(xw->display, xw->cmap, &xc)) {
			logging(WARN, "could not allocate color\n");
			return BlackPixel(xw->display, DefaultScreen(xw->display));
		} else {
			if (pxcache_top == PXCACHE_SIZE) {
				logging(WARN, "pixel cache is full. starting over\n");
				pxcache_top = 0;
			}
			pxcache[pxcache_top].color = color;
			pxcache[pxcache_top].pixel = xc.pixel;
			pxcache_top++;
			return xc.pixel;
		}
	}
#else
	return 0;
#endif
}

bool xw_init(struct xwindow_t *xw, uint16_t width, uint16_t height)
{
#ifndef HEADLESS
	XTextProperty xtext = {.value = (unsigned char *) "yaftx",
		.encoding = XA_STRING, .format = 8, .nitems = 5};

	if ((xw->display = XOpenDisplay(NULL)) == NULL) {
		logging(ERROR, "XOpenDisplay() failed\n");
		return false;
	}

	if (!XSupportsLocale())
		logging(WARN, "X does not support locale\n");

	if (XSetLocaleModifiers("") == NULL)
		logging(WARN, "cannot set locale modifiers\n");

	xw->cmap = DefaultColormap(xw->display, xw->screen);
	for (int i = 0; i < COLORS; i++)
		xw->color_palette[i] = color2pixel(xw, color_list[i]);

	xw->screen = DefaultScreen(xw->display);
	xw->window = XCreateSimpleWindow(xw->display, DefaultRootWindow(xw->display),
		0, 0, width, height, 0, xw->color_palette[DEFAULT_FG], xw->color_palette[DEFAULT_BG]);
	XSetWMProperties(xw->display, xw->window, &xtext, NULL, NULL, 0, NULL, NULL, NULL); /* return void */

	xw->gc = XCreateGC(xw->display, xw->window, 0, NULL);
	XSetGraphicsExposures(xw->display, xw->gc, False);

	xw->width  = DisplayWidth(xw->display, xw->screen);
	xw->height = DisplayHeight(xw->display, xw->screen);
	xw->pixbuf = XCreatePixmap(xw->display, xw->window,
		xw->width, xw->height, XDefaultDepth(xw->display, xw->screen));

	XSelectInput(xw->display, xw->window,
		ExposureMask | KeyPressMask | StructureNotifyMask);

	XMapWindow(xw->display, xw->window);
#endif
	return true;
}

void xw_die(struct xwindow_t *xw)
{
#ifndef HEADLESS
	XFreeGC(xw->display, xw->gc);
	XFreePixmap(xw->display, xw->pixbuf);
	XDestroyWindow(xw->display, xw->window);
	XCloseDisplay(xw->display);
#endif
}

static inline void draw_line(struct xwindow_t *xw, struct terminal_t *term, int line, ge_GIF* img)
{
	int bdf_padding, glyph_width, margin_right;
	int col, w, h;
	struct color_pair_t color_pair;
	struct cell_t *cellp;
	const struct glyph_t *glyphp;

#ifndef HEADLESS
	/* at first, fill all pixels of line in background color */
	XSetForeground(xw->display, xw->gc, xw->color_palette[DEFAULT_BG]);
	XFillRectangle(xw->display, xw->pixbuf, xw->gc, 0, line * CELL_HEIGHT, term->width, CELL_HEIGHT);
#endif

	if(img) {
		for(int h = line * CELL_HEIGHT; h < line * CELL_HEIGHT + CELL_HEIGHT; h++) {
			for(int w = 0; w < term->width; w++) {
				img->frame[img->w * h + w] = 0;
			}
		}
	}

	for (col = term->cols - 1; col >= 0; col--) {
		margin_right = (term->cols - 1 - col) * CELL_WIDTH;

		/* skip sixel pixmaps */
		cellp = &term->cells[line][col];
		if (cellp->has_pixmap) {
			continue;
		}

		/* get color and glyph */
		color_pair = cellp->color_pair;
		glyphp     = cellp->glyphp;

		/* check wide character or not */
		glyph_width = (cellp->width == HALF) ? CELL_WIDTH: CELL_WIDTH * 2;
		bdf_padding = my_ceil(glyph_width, BITS_PER_BYTE) * BITS_PER_BYTE - glyph_width;
		if (cellp->width == WIDE)
			bdf_padding += CELL_WIDTH;

		/* check cursor position */
		if ((term->mode & MODE_CURSOR && line == term->cursor.y)
			&& (col == term->cursor.x
			|| (cellp->width == WIDE && (col + 1) == term->cursor.x)
			|| (cellp->width == NEXT_TO_WIDE && (col - 1) == term->cursor.x))) {
			color_pair.fg = DEFAULT_BG;
			color_pair.bg = (!vt_active && BACKGROUND_DRAW) ? PASSIVE_CURSOR_COLOR: ACTIVE_CURSOR_COLOR;
		}

		for (h = 0; h < CELL_HEIGHT; h++) {
			/* if UNDERLINE attribute on, swap bg/fg */
			if ((h == (CELL_HEIGHT - 1)) && (cellp->attribute & attr_mask[ATTR_UNDERLINE]))
				color_pair.bg = color_pair.fg;

			for (w = 0; w < CELL_WIDTH; w++) {
				int x = term->width - 1 - margin_right - w;
				int y = line * CELL_HEIGHT + h;
				/* set color palette */
				if (glyphp->bitmap[h] & (0x01 << (bdf_padding + w))) {
#ifndef HEADLESS
					XSetForeground(xw->display, xw->gc, xw->color_palette[color_pair.fg]);
#endif
					img->frame[img->w * y + x] = color_pair.fg;
				}
				else if (color_pair.bg != DEFAULT_BG) {
#ifndef HEADLESS
					XSetForeground(xw->display, xw->gc, xw->color_palette[color_pair.bg]);
#endif
					img->frame[img->w * y + x] = color_pair.bg;
				}
				else /* already draw */
					continue;

				/* update copy buffer */
#ifndef HEADLESS
				XDrawPoint(xw->display, xw->pixbuf, xw->gc,
					term->width - 1 - margin_right - w, line * CELL_HEIGHT + h);
#endif
			}
		}
	}
}

long long last_frame_timing = 0;

void refresh(struct xwindow_t *xw, struct terminal_t *term, ge_GIF* img)
{
	int line, update_from, update_to;

	update_from = update_to = -1;
	for (line = 0; line < term->lines; line++) {
		draw_line(xw, term, line, img);

		if (update_from == -1)
			update_from = update_to = line;
		else
			update_to = line;
	}

#ifndef HEADLESS
	/* actual display update: vertical synchronizing */
	if (update_from != -1)
		XCopyArea(xw->display, xw->pixbuf, xw->window, xw->gc, 0, update_from * CELL_HEIGHT,
			term->width, (update_to - update_from + 1) * CELL_HEIGHT, 0, update_from * CELL_HEIGHT);
#endif

	long long now = timeInMilliseconds();
	if(last_frame_timing > 0) {
		long long diff = (now - last_frame_timing) / 10;
		ge_add_frame(img, diff);
	}

	last_frame_timing = now;
}
