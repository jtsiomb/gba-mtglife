#include <stdio.h>
#include <stdlib.h>
#include "gbasys.h"
#include "gfx.h"
#include "sincos.h"
#include "logger.h"
#include "data.h"
#include "dynarr.h"

static int init(void);
static void draw(void);
static void draw_number(int font, int x, int y, int num, unsigned int r, unsigned int g, unsigned int b);
static void draw_number_half(int font, int x, int y, int num, unsigned int r, unsigned int g, unsigned int b);
static void keypress(unsigned short key);
static void keyb_intr(void);

static int score[2] = {20, 20};
static int delta;
static int edit = -1;
static int *history[2];
static int hist_edit = -1;
static int hist_sel = -1;

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

	history[0] = dynarr_alloc(0, sizeof *history[0]);
	history[1] = dynarr_alloc(0, sizeof *history[1]);
	if(!history[0] || !history[1]) {
		logmsg(LOG_ALL, "failed to allocate memory\n");
		return -1;
	}
	return 0;
}

static void draw(void)
{
	int16_t sintm;
	int i, pulse;
	static int xpos[2] = {50, 240 - 50};
	unsigned long msec = get_millisec();

	copy_buffer(&pbuf_bg, back_buffer);

	sintm = sin_int(msec / 4);
	pulse = (sintm + SINLUT_SCALE) / 2 * 128 / SINLUT_SCALE + 128;

	for(i=0; i<2; i++) {
		unsigned int color;
		int j, hsz, x, y, value;
		int hist_dir_sign = i >= 1 ? 1 : -1;

		hsz = dynarr_size(history[i]);

		x = xpos[i];
		y = 60;
		for(j=0; j<3; j++) {
			int idx = hsz - j - 1;
			if(hist_edit == i) {
				idx -= hist_sel + 1;
			}
			if(idx < 0) break;
			color = 128 - 32 * j;
			draw_number_half(FONT_SMALL, x, y, history[i][idx], color, color, hist_edit == i ? 32 : color);
			x += 5 * hist_dir_sign;
			y -= 16;
		}

		color = edit == i ? pulse : 255;
		if(hist_edit == i && hist_sel >= 0) {
			int hsz = dynarr_size(history[i]);
			value = history[i][hsz - hist_sel - 1];
		} else {
			value = score[i];
		}
		draw_number(FONT_LARGE, xpos[i], 100, value, hist_edit == i ? color / 4 : color, color, color);
	}

	if(edit == -1 && delta != 0) {
		int r = (delta < 0 ? 255 : 64) * pulse >> 8;
		int g = (delta > 0 ? 255 : 64) * pulse >> 8;
		int b = (32 * pulse) >> 8;
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

static void draw_number_half(int font, int x, int y, int num, unsigned int r, unsigned int g, unsigned int b)
{
	int i;
	int font_width = pbuf_num[font].x / 10;
	int font_height = pbuf_num[font].y;
	int dcount = count_digits(num);
	int width = dcount * font_width;
	int neg = num < 0;
	num = abs(num);

	if(dcount < 1) dcount = 1;

	x += width / 4;
	y -= font_height / 4;

	for(i=0; i<dcount; i++) {
		int digit = num % 10;
		num /= 10;

		x -= font_width / 2;
		blit_color_half(pbuf_num + font, digit * font_width, 0, font_width, font_height,
				back_buffer, x, y, r, g, b);
	}

	if(neg) {
		fill_rect(back_buffer, x - 8, y + font_height / 2 - 1, 5, 2, RGB(r, g, b));
	}
}

static void keypress(unsigned short key)
{
	static int idx, prev;

	switch(key) {
	case KEY_A:
	case KEY_B:
		if(hist_edit >= 0) {	/* in history edit mode */
			if(key == KEY_A && hist_sel >= 0) {
				/* confirm history edit */
				int idx = hist_edit;
				int hsz = dynarr_size(history[idx]);
				score[idx] = history[idx][hsz - hist_sel - 1];
				history[idx] = dynarr_resize(history[idx], hsz - hist_sel - 1);
				hist_edit = hist_sel = -1;
			} else {
				/* cancel history edit */
				hist_edit = hist_sel = -1;
			}

		} else if(edit >= 0) {	/* in direct score edit mode */
			if(score[edit] != prev) {
				history[edit] = dynarr_push(history[edit], &prev);
			}
			edit = -1;
		} else {
			edit = key == KEY_A ? 1 : 0;
			prev = score[edit];
		}
		break;

	case KEY_UP:
		if(edit >= 0) {	/* in direct score edit mode */
			++score[edit];
		} else if(hist_edit >= 0) {	/* in history edit mode */
			if(hist_sel < dynarr_size(history[hist_edit]) - 1) {
				++hist_sel;
			}
		} else {
			++delta;
		}
		break;

	case KEY_DOWN:
		if(edit >= 0) {	/* in direct score edit mode */
			--score[edit];
		} else if(hist_edit >= 0) {	/* in history edit mode */
			if(hist_sel >= 0) {
				--hist_sel;
			}
		} else {
			--delta;
		}
		break;

	case KEY_LEFT:
	case KEY_RIGHT:
		if(delta != 0) {
			idx = key == KEY_RIGHT ? 1 : 0;
			history[idx] = dynarr_push(history[idx], &score[idx]);
			score[idx] += delta;
			delta = 0;
		}
		break;

	case KEY_L:
	case KEY_R:
		idx = key == KEY_R ? 1 : 0;
		if(hist_edit == -1) {
			/* start history edit (if there is any) */
			if(!dynarr_empty(history[idx])) {
				hist_edit = idx;
				hist_sel = -1;
			}
		} else {
			/* cancel history edit */
			hist_edit = hist_sel = -1;
		}
		break;

	case KEY_START:
		score[0] = score[1] = 20;
		history[0] = dynarr_clear(history[0]);
		history[1] = dynarr_clear(history[1]);
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


