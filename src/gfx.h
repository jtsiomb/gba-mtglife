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
