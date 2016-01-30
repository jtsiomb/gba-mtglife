#include <stdio.h>
#include <stdlib.h>
#include "gbasys.h"
#include "sincos.h"
#include "logger.h"
#include "data.h"

static int init(void);
static void draw(void);
static void draw_number(int font, int x, int y, int num, unsigned int r, unsigned int g, unsigned int b);
static void keypress(unsigned short key);
static void keyb_intr(void);
static void blend_alpha(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
		struct pixel_buffer *dst, int dst_x, int dst_y, unsigned int r, unsigned int g,
		unsigned int b, unsigned int alpha);
static void blit_color(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
		struct pixel_buffer *dst, int dst_x, int dst_y, unsigned int r, unsigned int g, unsigned int b);

static int score[2] = {20, 20};
static int delta;
static int edit = -1;

#define FONT_LARGE	0
#define FONT_SMALL	1
static struct pixel_buffer pbuf_num[2];
static struct pixel_buffer pbuf_bg;

#define EVQ_SIZE	8
volatile static unsigned short evhead, evtail;
static unsigned short events[EVQ_SIZE];
static unsigned short keystate;

int main(void)
{
	gba_init();
	interrupt(INTR_KEY, keyb_intr);

	set_video_mode(VMODE_LFB_240x160_16, 1);

	clear_buffer(front_buffer, 0);
	set_text_writebg(1);
	logmsg(LOG_ALL, "please wait...\n");

	if(init() == -1) {
		return 1;
	}

	for(;;) {
		/* process events */
		while(evhead != evtail) {
			unsigned short ev = events[evhead];
			evhead = (evhead + 1) % EVQ_SIZE;
			keypress(ev);
		}

		keystate = get_key_state(KEY_ALL);

		draw();
	}
	return 0;
}

static int init(void)
{
	logmsg(LOG_ALL, "pre-calculating sin/cos LUT...\n");
	sincos_init();

	logmsg(LOG_ALL, "almost done...\n");
	pbuf_num[FONT_LARGE].x = numlarge_width;
	pbuf_num[FONT_LARGE].y = numlarge_height;
	pbuf_num[FONT_LARGE].bpp = numlarge_bpp;
	pbuf_num[FONT_LARGE].pixels = (void*)numlarge_pixels;
	pbuf_num[FONT_SMALL].x = numsmall_width;
	pbuf_num[FONT_SMALL].y = numsmall_height;
	pbuf_num[FONT_SMALL].bpp = numsmall_bpp;
	pbuf_num[FONT_SMALL].pixels = (void*)numsmall_pixels;

	pbuf_bg.x = bg_width;
	pbuf_bg.y = bg_height;
	pbuf_bg.bpp = bg_bpp;
	pbuf_bg.pixels = (void*)bg_pixels;
	return 0;
}

static void draw(void)
{
	int16_t sintm;
	int pulse;
	unsigned long msec = get_millisec();

	copy_buffer(&pbuf_bg, back_buffer);

	sintm = sin_int(msec / 4);
	pulse = (sintm + SINLUT_SCALE) / 2 * 128 / SINLUT_SCALE + 128;

	unsigned int color = edit == 0 ? pulse : 255;
	draw_number(FONT_LARGE, 50, 90, score[0], color, color, color);
	color = edit == 1 ? pulse : 255;
	draw_number(FONT_LARGE, 240 - 50, 90, score[1], color, color, color);

	if(edit == -1 && delta != 0) {
		int r = delta < 0 ? 255 : pulse / 2;
		int g = delta > 0 ? 255 : pulse / 2;
		int b = pulse / 4;
		draw_number(FONT_SMALL, 120, 25, delta, r, g, b);
	}

	flip();
}

static int count_digits(int n)
{
	int dig;
	n = abs(n);
	if(n < 10) return 1;
	if(n < 100) return 2;

	dig = 0;
	while(n) {
		++dig;
		n /= 10;
	}
	return dig;
}

static void fill_rect(struct pixel_buffer *pbuf, int x, int y, int width, int height,
		unsigned short color)
{
	int i;
	unsigned short *pptr = (unsigned short*)pbuf->pixels + y * pbuf->x + x;

	for(i=0; i<height; i++) {
		dma_fill16(3, pptr, color, width);
		pptr += pbuf->x;
	}
}

static void draw_number(int font, int x, int y, int num, unsigned int r, unsigned int g, unsigned int b)
{
	int i;
	int font_width = pbuf_num[font].x / 10;
	int font_height = pbuf_num[font].y;
	int dcount = count_digits(num);
	int width = dcount * font_width;
	int neg = num < 0;
	num = abs(num);

	if(dcount < 1) dcount = 1;

	x += width / 2;
	y -= font_height / 2;

	for(i=0; i<dcount; i++) {
		int digit = num % 10;
		num /= 10;

		x -= font_width;
		blit_color(pbuf_num + font, digit * font_width, 0, font_width, font_height,
				back_buffer, x, y, r, g, b);
	}

	if(neg) {
		fill_rect(back_buffer, x - 16, y + font_height / 2 - 2, 10, 3, RGB(r, g, b));
	}
}

static void keypress(unsigned short key)
{
	switch(key) {
	case KEY_B:
		edit = edit == -1 ? 0 : -1;
		break;
	case KEY_A:
		edit = edit == -1 ? 1 : -1;
		break;

	case KEY_UP:
		if(edit == -1) {
			++delta;
		} else {
			++score[edit];
		}
		break;

	case KEY_DOWN:
		if(edit == -1) {
			--delta;
		} else {
			--score[edit];
		}
		break;

	case KEY_LEFT:
		score[0] += delta;
		delta = 0;
		break;

	case KEY_RIGHT:
		score[1] += delta;
		delta = 0;
		break;
	}
}

static void keyb_intr(void)
{
	int i;
	unsigned short prev_keystate = keystate;
	unsigned short diff;

	keystate = get_key_state(KEY_ALL);
	diff = keystate ^ prev_keystate;

	for(i=0; i<32; i++) {
		int key = 1 << i;

		if(diff & key) {
			events[evtail] = key;
			evtail = (evtail + 1) % EVQ_SIZE;
		}
	}
}

#define MIN(a, b)	((a) < (b) ? (a) : (b))

static void blend_alpha(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
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

static void blit_color(struct pixel_buffer *src, int src_x, int src_y, int src_w, int src_h,
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
