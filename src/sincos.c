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
#include <math.h>
#include "fixed.h"
#include "sincos.h"
#include "logger.h"

void sincos_init(void)
{
	int i;

	logmsg(LOG_ALL, "calculating sin/cos lut...\n");

	for(i=0; i<SINLUT_SIZE; i++) {
		float angle = 2.0 * M_PI * ((float)i / (float)SINLUT_SIZE);
		float val = sin(angle);
		sinlut[i] = (int16_t)(val * SINLUT_SCALE);
	}
}

int16_t sin_int(int16_t norm_angle)
{
	norm_angle %= SINLUT_SIZE;
	if(norm_angle < 0) {
		norm_angle += SINLUT_SIZE;
	}
	return sinlut[norm_angle];
}

int16_t cos_int(int16_t norm_angle)
{
	return sin_int(norm_angle + SINLUT_SIZE / 4);
}

int32_t sin_x16(int32_t radians)
{
	int32_t na = x16div(radians, M_PI_X16 * 2);
	return (sin_int((na * SINLUT_SIZE) >> 16) << 16) / SINLUT_SCALE;
}

int32_t cos_x16(int32_t radians)
{
	int32_t na = x16div(radians, M_PI_X16 * 2);
	return (cos_int((na * SINLUT_SIZE) >> 16) << 16) / SINLUT_SCALE;
}
