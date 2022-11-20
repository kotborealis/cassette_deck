/* See LICENSE for licence details. */
#include "../yaft.h"
#include "../conf.h"
#include "../util.h"
#include "x.h"
#include "../terminal.h"
#include "../ctrlseq/esc.h"
#include "../ctrlseq/csi.h"
#include "../ctrlseq/osc.h"
#include "../ctrlseq/dcs.h"
#include "../parse.h"
#include "../3rd_party/gifenc/gifenc.h"
#include "../tape_player/tape_player.h"

tape_player_t* player;

void sig_handler(int signo)
{
	extern volatile sig_atomic_t child_alive;

	if (signo == SIGCHLD) {
		child_alive = false;
		wait(NULL);
	}

	if(signo == SIGUSR2) {
		tape_player_awaited(player);
	}
}

bool sig_set(int signo, void (*handler)(int signo), int flags)
{
	struct sigaction sigact;

	memset(&sigact, 0, sizeof(struct sigaction));
	sigact.sa_handler = handler;
	sigact.sa_flags   = flags;

	if (esigaction(signo, &sigact, NULL) == -1) {
		logging(WARN, "sigaction: signo %d failed\n", signo);
		return false;
	}
	return true;
}

void check_fds(fd_set *fds, struct timeval *tv, int master)
{
	FD_ZERO(fds);
	FD_SET(master, fds);
	tv->tv_sec  = 0;
	tv->tv_usec = SELECT_TIMEOUT;
	eselect(master + 1, fds, NULL, NULL, tv);
}

bool fork_and_exec(int *master, const char *cmd, char *const argv[], int lines, int cols)
{
	pid_t pid;
	struct winsize ws = {.ws_row = lines, .ws_col = cols,
		/* XXX: this variables are UNUSED (man tty_ioctl),
			but useful for calculating terminal cell size */
		.ws_ypixel = CELL_HEIGHT * lines, .ws_xpixel = CELL_WIDTH * cols};

	pid_t host_pid = getpid();

	pid = eforkpty(master, NULL, NULL, &ws);
	if (pid < 0)
		return false;
	else if (pid == 0) { /* child */
		esetenv("TERM", term_name, 1);

		char* prompt_command = malloc(1024);
		sprintf(prompt_command, "kill -USR2 %d", host_pid);
		esetenv("PROMPT_COMMAND", prompt_command, 1);
		eexecvp(cmd, argv);
		/* never reach here */
		exit(EXIT_FAILURE);
	}
	return true;
}

int usage(char* binary) {
	printf("Usage: %s {path to tape file}\n", binary);
	return 1;
}

int main(int argc, char *const argv[])
{
	if(argc < 2)
		return usage(argv[0]);

	FILE *f = fopen(argv[1], "rb");

	if(f == NULL) {
		printf("Error: could not open %s\n", argv[1]);
		return 1;
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *data = malloc(fsize + 1);
	if(!fread(data, fsize, 1, f)) {
		printf("Error: could not read data from file %s\n", argv[1]);
		return 1;
	}

	fclose(f);

	data[fsize] = 0;

	player = tape_player_new(data);

	if(!player)
		return 1;

	extern const char *shell_cmd; /* defined in conf.h */
	uint8_t buf[BUFSIZE];
	ssize_t size;
	fd_set fds;
	struct timeval tv;
	struct xwindow_t xw;
	struct terminal_t term;
	XEvent ev;
	XConfigureEvent confev;
	extern volatile sig_atomic_t child_alive;

	/* init */
	if (!setlocale(LC_ALL, ""))
		logging(WARN, "setlocale falied\n");

	if (!sig_set(SIGCHLD, sig_handler, SA_RESTART))
		logging(ERROR, "signal initialize failed\n");
	if(!sig_set(SIGUSR2, sig_handler, SA_RESTART))
		logging(ERROR, "signal initialize failed\n");

	if (!xw_init(&xw, tape_player_width(player), tape_player_height(player))) {
		logging(FATAL, "xwindow initialize failed\n");
		goto xw_init_failed;
	}

	if (!term_init(&term, tape_player_width(player), tape_player_height(player))) {
		logging(FATAL, "terminal initialize failed\n");
		goto term_init_failed;
	}

	/* fork and exec shell */
	const char* cmd = "bash";
	char* args[] = {"bash", "--noprofile", "--norc", NULL};

	if (!fork_and_exec(&term.fd, cmd, args, term.lines, term.cols)) {
		logging(FATAL, "forkpty failed\n");
		goto fork_failed;
	}
	child_alive = true;

    ge_GIF *img = ge_new_gif(
        tape_player_output(player),
        tape_player_width(player), tape_player_height(player),
        color_list_u8,
        8,              /* palette depth == log2(# of colors) */
        -1,             /* no transparency */
        0               /* infinite loop */
    );

	/* main loop */
	while (child_alive) {
		long long start = timeInMilliseconds();

		while(XPending(xw.display))
			XNextEvent(xw.display, &ev);

		check_fds(&fds, &tv, term.fd);
		if (FD_ISSET(term.fd, &fds)) {
			size = read(term.fd, buf, BUFSIZE);
			if (size > 0) {
				if (DEBUG)
					ewrite(STDOUT_FILENO, buf, size);
				parse(&term, buf, size);
			}
		}

		long long diff;
		int stop = 0;
		do {
			diff = timeInMilliseconds() - start;
			if(diff < 1000 / 15) {
				usleep(20 * 1000);
			}
			if(tape_player_frame(player, ewrite, term.fd)) {
				stop = 1;
				break;
			}
		} while(diff < 1000 / 15);

		refresh(&xw, &term, img);

		if(stop)
			break;
	}

	ge_close_gif(img);

	/* die */
	term_die(&term);
	xw_die(&xw);
	sig_set(SIGCHLD, SIG_DFL, 0);
	return EXIT_SUCCESS;

fork_failed:
	term_die(&term);
term_init_failed:
	xw_die(&xw);
xw_init_failed:
	return EXIT_FAILURE;
}
