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
#ifndef SINCOS_H_
#define SINCOS_H_

#include <stdint.h>

/*#define M_PI_X16	(int32_t)(M_PI * 65536.0) */
#define M_PI_X16	(int32_t)((31416 << 16) / 10000)

#define SINLUT_SCALE	512
#define SINLUT_SIZE		256
int16_t sinlut[SINLUT_SIZE];

void sincos_init(void);

/* takes angle in [0, SINLUT_SIZE] and returns:
 * sin(2 * angle / SINLUT_SIZE / pi) * SINLUT_SCALE
 */
int16_t sin_int(int16_t norm_angle);
int16_t cos_int(int16_t norm_angle);

/* takes angle in fixed point 16.16 radians [0, 2pi << 16]
 * and returns 16.16 fixed point in [-1 << 16, 1 << 16]
 */
int32_t sin_x16(int32_t radians);
int32_t cos_x16(int32_t radians);

#endif	/* SINCOS_H_ */
