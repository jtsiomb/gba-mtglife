#include "gfx.h"

#define MIN(a, b)	((a) < (b) ? (a) : (b))

void blend_alpha(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
		struct pixel_buffer *dst, int dst_x, int dst_y, unsigned int r, unsigned int g,
		unsigned int b, unsigned int alpha)
{
	int i, j, width, height, dstride, sstride;
	unsigned short *dptr, *sptr;

	if(src_w <= 0)
		src_w = src->x;
	if(src_h <= 0)
		src_h = src->y;

	width = MIN(src_w, MIN(src->x - src_x, dst->x - dst_x));
	height = MIN(src_h, MIN(src->y - src_y, dst->y - dst_y));

	if(width <= 0 || height <= 0)
		return;

	dptr = (unsigned short*)dst->pixels + (dst_y * dst->x + dst_x);
	sptr = (unsigned short*)src->pixels + (src_y * src->x + src_x);

	dstride = dst->x;
	sstride = src->x;

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			unsigned short dpix = *dptr;
			unsigned short spix = *sptr++;

			unsigned int a = (GET_B(spix) * alpha) >> 8;
			unsigned int inv_a = 256 - a;

			unsigned int dr = (r * a + GET_R(dpix) * inv_a) >> 8;
			unsigned int dg = (g * a + GET_G(dpix) * inv_a) >> 8;
			unsigned int db = (b * a + GET_B(dpix) * inv_a) >> 8;

			*dptr++ = RGB(dr, dg, db);
		}
		sptr += sstride - width;
		dptr += dstride - width;
	}
}

void blit_color(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
		struct pixel_buffer *dst, int dst_x, int dst_y, unsigned int r, unsigned int g, unsigned int b)
{
	int i, j, width, height, dstride, sstride;
	unsigned short *dptr, *sptr;

	if(src_w <= 0)
		src_w = src->x;
	if(src_h <= 0)
		src_h = src->y;

	width = MIN(src_w, MIN(src->x - src_x, dst->x - dst_x));
	height = MIN(src_h, MIN(src->y - src_y, dst->y - dst_y));

	if(width <= 0 || height <= 0)
		return;

	dptr = (unsigned short*)dst->pixels + (dst_y * dst->x + dst_x);
	sptr = (unsigned short*)src->pixels + (src_y * src->x + src_x);

	dstride = dst->x;
	sstride = src->x;

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			unsigned short pix = *sptr++;

			unsigned int val = GET_B(pix);

			unsigned int pr = (r * val) >> 8;
			unsigned int pg = (g * val) >> 8;
			unsigned int pb = (b * val) >> 8;

			*dptr++ = RGB(pr, pg, pb);
		}
		sptr += sstride - width;
		dptr += dstride - width;
	}
}

void blit_color_half(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
		struct pixel_buffer *dst, int dst_x, int dst_y, unsigned int r, unsigned int g, unsigned int b)
{
	int i, j, width, height, dstride, sstride;
	unsigned short *dptr, *sptr;

	if(src_w <= 0)
		src_w = src->x;
	if(src_h <= 0)
		src_h = src->y;

	width = MIN(src_w, MIN(src->x - src_x, dst->x - dst_x));
	height = MIN(src_h, MIN(src->y - src_y, dst->y - dst_y));

	if(width <= 0 || height <= 0)
		return;

	dptr = (unsigned short*)dst->pixels + (dst_y * dst->x + dst_x);
	sptr = (unsigned short*)src->pixels + (src_y * src->x + src_x);

	dstride = dst->x;
	sstride = src->x;

	for(i=0; i<height/2; i++) {
		for(j=0; j<width/2; j++) {
			unsigned short pix = *sptr;

			unsigned int val = GET_B(pix);

			unsigned int pr = (r * val) >> 8;
			unsigned int pg = (g * val) >> 8;
			unsigned int pb = (b * val) >> 8;

			sptr += 2;
			*dptr++ = RGB(pr, pg, pb);
		}
		sptr += sstride * 2 - width;
		dptr += dstride - width/2;
	}
}
