# CHIP8 #

This is an an interpreter for the CHIP-8 language which ran on certain hobbyist 
computers in the 1970's and a virtual machine which emulates the hardware in
those computers.

## Compilation ##

### Linux

To compile this program you will need:

1. GNU make.  Possibly other make programs might work but they have not been
   tested.

2. A C++17 comformant compiler. (It has been tested with g++ 10.2 and clang++ 10.0
   on Linux.)

3. The X11, OpenGL, and PulseAudio libraries and their development headers,
   libraries etc.  On Debian GNU/Linux and derivatives such as Ubuntu, these are
   the packages: libx11-dev, libgl-dev and libpulse-dev.
 
Then change to either the `debug` (to include debug information in the binary
or `release` (for an optimized binary.) directories and run `make`.

### Windows

Solution and project files for Visual Studio 2022 have been included in this 
source distribution.

## Installation ##

### Linux

From the top level directory, run `make install`.  By default, the program will
be installed into `/usr/local/bin` but the standard `DESTDIR`, `PREFIX` and 
`BINDIR` variables can be set to alter this.

### Windows

Simply take the compiled chip8.exe file and place it somewhere.  You can run it
from a DOS prompt or Power Shell.

## Usage ##

You can run `chip8` by itself but it won't do anything.  If you give the path to
a ROM file as an argument, `chip8` will load and run it.  You can find suitable
ROMs at the sites linked to below.

CHIP-8 keys are mapped to the following:

| CHIP-8 | Keyboard |
|:------:|:--------:|
|   0    |    x     |
|   1    |    1     |
|   2    |    2     |
|   3    |    3     |
|   4    |    q     |
|   5    |    w     |
|   6    |    e     |
|   7    |    a     |
|   8    |    s     |
|   9    |    d     |
|   A    |    z     |
|   B    |    c     |
|   C    |    4     |
|   D    |    r     |
|   E    |    f     |
|   F    |    v     |

## Resources ##

The following web sites were useful to me in learning about CHIP-8 and
implementing this program.

* [Awesome CHIP-8](https://github.com/tobiasvl/awesome-chip-8).
* [Building a CHIP-8 Emulator [C++]](https://austinmorlan.com/posts/chip8_emulator/).
* [CHIP-8 Documentation](https://github.com/trapexit/chip-8_documentation).
* [CHIP-8 on the Cosmac VIP](https://laurencescotford.com/chip-8-on-the-cosmac-vip-index/).
* [CHIP-8 Research Facility](https://chip-8.github.io).
* [CHIP-8 Test Suite](https://github.com/Timendus/chip8-test-suite).
* [CHIP-8 Variant Opcode Table](https://games.gulrak.net/cadmium/chip8-opcode-table.html)
* [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM).
* [Instructables: CHIP-8 Computer](https://www.instructables.com/CHIP-8-Computer/).
* [Mastering CHIP-8](http://mattmik.com/files/chip8/mastering/chip8.html).
* [CHIP-8 Wiki](https://chip8.fandom.com/)

These web sites contain ROMS you can load into the emulator.

* [CHIP-8 Archive](https://github.com/JohnEarnest/chip8Archive)
* [dmatlack/chip8 on Github](https://github.com/dmatlack/chip8/tree/master/roms/programs).
* [Zophar's Domain](https://www.zophar.net/pdroms/chip8.html).
