MTG life counter for the GameBoy Advance
========================================

About
-----
Android life counters are nice, and most android phone screens are awesome, but
let's face it, keeping the screen on for the hours it takes to go through an
FNM, or even worse a pre-release tournament, sucks most of the life out of a
typical android phone battery.

That's why I decided to write a *GameBoy Advance* life counter.  The GBA SP in
particular is very compact, and its battery lasts forever.

Web site: http://nuclear.mutantstargoat.com/sw/mtglife

License
-------
Copyright (C) 2016 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, copy, modify, and/or
redistribute it, under the terms of the GNU General Public License v3, or any
later version published by the Free Software Foundation. See COPYING for
details.

Usage
-----
Use the direction-pad *up/down* buttons to increment or decrement the amount of
damage or life-gain, then hit either *left* or *right* to apply it to a player's
life total.

To directly edit either player's life total, press buttons *A* or *B* to
enter edit mode, then use the up/down buttons to adjust it. Hit *A* or *B*
again to apply the change and return to normal mode.

The *left/right shoulder* buttons can be used to enter undo
mode for either player. In this mode, *up*/*down* moves back and forth through
the life history of the player. *A* applies the undo, and *B* cancels.

The *start* button resets the state of the program to start a new
game.

Finally, hitting *select* at any time, brings up a graph of life totals for both
players over time.

Build instructions
------------------
This program depends on my gameboy advance system abstraction library "gbasys":
https://github.com/jtsiomb/gbasys
Make sure to clone the gbasys source under the same parent directory as
gba-mtglife, and build it before trying to build this program.

To build either of them, you're going to need a GBA cross-compiler toolchain
like the own from devkitpro (http://devkitpro.org/wiki/Getting_Started/devkitARM)
already installed.

To build just type `make`, and it should produce the mtglife.gba image. You may
type `make install` to upload the image to the GBA cartridge if you happen to
use the same link cable I'm using (flash2advance). Otherwise you might need to
modify the install target in the makefile or just upload manually. Again with
flash2advance, you can type `make run` to attempt to upload into the GBA memory
and immediately run a multiboot version of the program (assuming it fits in
RAM). Finally `make simrun` starts the program in the `vbam` emulator. Feel free
to change the `EMU` and `EMUFLAGS` variables if you wish to use a different
emulator.
