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
#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "gbasys.h"

void blend_alpha(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
		struct pixel_buffer *dst, int dst_x, int dst_y, unsigned int r, unsigned int g,
		unsigned int b, unsigned int alpha);

void blit_color(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
		struct pixel_buffer *dst, int dst_x, int dst_y, unsigned int r, unsigned int g, unsigned int b);

void blit_color_half(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
		struct pixel_buffer *dst, int dst_x, int dst_y, unsigned int r, unsigned int g, unsigned int b);


#endif	/* GRAPHICS_H_ */
