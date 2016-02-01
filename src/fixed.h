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
#ifndef FIXED_H_
#define FIXED_H_

#include <stdint.h>

#define ftox16(x)		(int32_t)((x) * 65536.0f)
#define itox16(x)		(int32_t)((x) << 16)

#define x16tof(x)		(float)((x) / 65536.0f)
#define x16toi(x)		((x) >> 16)

#define x16mul(a, b)	(int32_t)(((int32_t)(a) >> 8) * ((int32_t)(b) >> 8))
/*int32_t x16div(register int32_t num, register int32_t denom);*/
#define x16div(a, b)	(int32_t)(((int64_t)(a) << 16) / (b))

#define x16sq(x)		x16mul(x, x)

#endif	/* FIXED_H_ */
