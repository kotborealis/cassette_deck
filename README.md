# cassete-deck

Record terminal commands into a GIF file.

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

* X11 for GUI preview;
* [lecram/gifenc](https://github.com/lecram/gifenc) to write GIF files;
* [uobikiemukot/yaft](https://github.com/uobikiemukot/yaft) used as a simple terminal emulator implementation.

## Build

Clone this repo with submodules, build using cmake 3.0+ and C/CPP compiler
with C++17 support:
![build.tape](./demo/build.tape.gif)

## Usage

CLI usage:

```sh
./build/cassette_deck ./demo/hello.tape.gif
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

## TODO

* headless mode without X11 dependency;
* more commands from VHS, compatibility with VHS tapes;
* colors, themes.