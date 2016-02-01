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
