/*
GBA mtglife - Magic the gathering life counter for the GameBoy Advance
Copyright (C) 2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <alloca.h>
#include "gbasys.h"
#include "logger.h"

static void putchr(char c);
static void putstr(const char *str);
static void agbprint(const char *str);

static int curx, cury;
static int font_width = 8, font_height = 8;
static int ncols = 20, nrows = 16;

void logmsg(unsigned short where, const char *fmt, ...)
{
	va_list ap;
	int sz;
	char *buf;

	va_start(ap, fmt);
	sz = vsnprintf(0, 0, fmt, ap);
	va_end(ap);

	buf = alloca(sz + 1);
	va_start(ap, fmt);
	vsnprintf(buf, sz + 1, fmt, ap);
	va_end(ap);

	if(where & LOG_SCREEN) {
		putstr(buf);
	}
	if(where & LOG_DBG) {
		agbprint(buf);
	}
}

static void putchr(char c)
{
	switch(c) {
	case '\n':
		curx = 0;
		if(cury >= nrows - 1) {
			int row_size = font_height * 160 * 2;
			memmove(front_buffer->pixels, (char*)front_buffer->pixels + row_size,
					(nrows - 1) * row_size);
			memset((char*)front_buffer->pixels + (nrows - 1) * row_size, 0, row_size);
		} else {
			++cury;
		}
		break;

	case '\r':
		curx = 0;
		break;

	default:
		if(isprint((int)c)) {
			draw_glyph(c, curx * font_width, cury * font_height, front_buffer);
			if(++curx >= ncols) {
				putchr('\n');
			}
		}
	}
}

static void putstr(const char *str)
{
	while(*str) putchr(*str++);
}

#ifdef __ARM__
static void agbprint(const char *str)
{
	asm volatile (
		"\n\tmov r0, %0"
		"\n\tswi 0xff0000"
		:
		: "r" (str)
		: "r0");
}
#else
/* fake version */
static void agbprint(const char *str)
{
	fputs(str, stdout);
	fflush(stdout);
}
#endif
