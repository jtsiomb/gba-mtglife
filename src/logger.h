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
#ifndef LOGGER_H_
#define LOGGER_H_

enum {
	LOG_SCREEN = 1,
	LOG_DBG = 2
};
#define LOG_ALL		0xffff

void logmsg(unsigned short where, const char *fmt, ...);

#endif	/* LOGGER_H_ */
