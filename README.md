# cassete-deck

Record terminal commands into a GIF file.

**⚠️ This is still just a MVP/technical demo/etc. Most features are not implemented/tested. Use cautiously.**

![hello.tape](./demo/hello.tape.gif)

This project was inspired by [VHS](https://github.com/charmbracelet/vhs).

VHS requires `ffmpeg` and `chromium` to record terminal emulator GIFs.
Both terminals and GIFs were invented **many** years ago, and are
really simple and stupid.
While using modern technologies to work with them is alright,
it's not efficient to use millions SLOCs of `chromium` and `ffmpeg`
for such a simple tasks.

The aim of this project is to recreate core of VHS with minimal dependencies.
`cassete-deck` built with 2 pieces of software:

* [lecram/gifenc](https://github.com/lecram/gifenc) to write GIF files;
* [uobikiemukot/yaft](https://github.com/uobikiemukot/yaft) used as a simple terminal emulator implementation:
    * This project is essentially a fork of [yaft](https://github.com/uobikiemukot/yaft) with GIF backend and `.tape` player.

Such utility may be usefull to enrich documentation with animated GIFs which illustrate
usage of CLI programs.

![colors.tape](./demo/colors.tape.gif)

## Prior art

Of course, there's a lot of similar projects, like:

* [VHS](https://github.com/charmbracelet/vhs):
    * depends on chromium and ffmpeg.
* [uobikiemukot/recterm](https://github.com/uobikiemukot/recterm):
    * implementation by the author of [yaft](https://github.com/uobikiemukot/yaft);
    * was not designed to execute commands from file.
* [asciinema](https://github.com/asciinema/asciinema):
    * records terminal to upload and replay it in browser;
    * was not designed to execute commands from file.
* [lecram/config](https://github.com/lecram/congif):
    * renders `script` recordings as GIFs.
* [script and scriptreplay](https://man7.org/linux/man-pages/man1/scriptreplay.1.html):
    * GNU utility to record and replay terminal sessions.

If you know any other interesting utilities for recording terminal emulators, feel free
to update this list via pull request or issue.

## Build

Clone this repo with submodules, build using cmake 3.0+ and C/CPP compiler
with C++17 support:
![build.tape](./demo/build.tape.gif)

## Usage

CLI usage:

* Preview mode --- depends on X11, opens graphic window to show shell in real time.
    ```sh
    ./build/cassette_deck ./demo/hello.tape.gif
    ```
* Headless mode --- does not depend on X11 at all, does everything in background, suitable for CI and such.
    ```sh
    ./build/cassette_deck --headless ./demo/hello.tape.gif
    ```

## Tape reference

See examples in `./examples`.
Commands are modelled after [VHS's ones](https://github.com/charmbracelet/vhs#vhs-command-reference)
and should be almost compatible.

There's one command that is not implemented in VHS:
```
TYPE start_long_running_process
ENTER
AWAIT
```

`AWAIT` commands pauses tape playback until last command is complete.
This is implemented by sending `SIGUSR1` from controlled shell via
`PROMPT_COMMAND` environment variable.

![meta.tape](./demo/meta.tape.gif)

### List of implemented commands

* `OUTPUT {path}` --- specifies output GIF file. `./output.gif` if not specified.
* `WIDTH {width}` --- shell width, in pixels. 640 by default.
* `HEIGHT {height}` --- shell height, in pixels. 384 by default.
* `REQUIRE {command}` --- checks if specified command is in `PATH`. If not, throws an error.
* `KEY {key}` --- presses specified key.
* `ENTER`, `BACKSPACE`, `SPACE`, `TAB` --- aliases for specified keys.
* `SLEEP {ms}` --- sleeps for specified duration, in milliseconds.
* `TYPE {text}` --- enters sequence of keys specified by keys, sleeping 50ms between them.
* `AWAIT` --- sleeps until last executed shell command finishes.

## TODO

* more commands from VHS, compatibility with VHS tapes;
* themes;
* CI, CI actions;
* padding, decorations.
