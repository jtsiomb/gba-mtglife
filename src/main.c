#include <stdio.h>
#include <stdlib.h>
#include "gbasys.h"
#include "gfx.h"
#include "sincos.h"
#include "logger.h"
#include "data.h"
#include "dynarr.h"

struct vertex {
	int x, y;
};

static int init(void);
static int reset(void);
static void draw_scr_main(void);
static void draw_scr_graph(void);
static void fill_rect(struct pixel_buffer *pbuf, int x, int y, int width, int height, unsigned short color);
static void draw_number(int font, int x, int y, int num, unsigned int r, unsigned int g, unsigned int b);
static void draw_number_half(int font, int x, int y, int num, unsigned int r, unsigned int g, unsigned int b);
static void keypress_scr_main(unsigned short key);
static void keypress_scr_graph(unsigned short key);
static void keyb_intr(void);
static void build_curves(void);

static int starting_life = 20;
static int score[2];
static int delta;
static int edit;
static int *history[2];
static int hist_edit;
static int hist_sel;
static int show_graph;
static struct vertex *curve[2];

#define CURVES_BASELINE	10

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

	{
		static unsigned short dbg_input[] = {KEY_DOWN, KEY_LEFT, KEY_DOWN, KEY_RIGHT,
			KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_DOWN, KEY_DOWN, KEY_RIGHT,
			KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_LEFT,	KEY_DOWN, KEY_RIGHT,
			KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_RIGHT,
			KEY_DOWN, KEY_DOWN, KEY_LEFT,
			KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_DOWN, KEY_RIGHT,
			KEY_ALL
		};
		int i;
		for(i=0; dbg_input[i] != KEY_ALL; i++) {
			keypress_scr_main(dbg_input[i]);
		}
	}

	for(;;) {
		/* process events */
		while(evhead != evtail) {
			unsigned short ev = events[evhead];
			evhead = (evhead + 1) % EVQ_SIZE;

			if(show_graph) {
				keypress_scr_graph(ev);
			} else {
				keypress_scr_main(ev);
			}
		}

		keystate = get_key_state(KEY_ALL);

		if(show_graph) {
			draw_scr_graph();
		} else {
			draw_scr_main();
		}
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

	return reset();
}

static int reset(void)
{
	int i;

	score[0] = score[1] = starting_life;
	delta = 0;
	edit = -1;
	hist_edit = hist_sel = -1;
	show_graph = 0;

	for(i=0; i<2; i++) {
		dynarr_free(history[i]);
		history[i] = dynarr_alloc(0, sizeof *history[i]);
		if(!history[i]) {
			logmsg(LOG_ALL, "failed to allocate memory\n");
			return -1;
		}
	}
	return 0;
}

static void draw_scr_main(void)
{
	int16_t sintm;
	int i;
	static int xpos[2] = {50, 240 - 50};
	unsigned long msec = get_millisec();

	copy_buffer(&pbuf_bg, back_buffer);

	sintm = sin_int(msec / 4);

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

		if(edit == i) {
			color = (sintm + SINLUT_SCALE) / 2 * 128 / SINLUT_SCALE + 128;
		} else {
			color = 255;
		}
		if(hist_edit == i && hist_sel >= 0) {
			int hsz = dynarr_size(history[i]);
			value = history[i][hsz - hist_sel - 1];
		} else {
			value = score[i];
		}
		draw_number(FONT_LARGE, xpos[i], 100, value, hist_edit == i ? color / 4 : color, color, color);
	}

	if(edit == -1 && delta != 0) {
		int pulse = (sintm + SINLUT_SCALE) / 2 * 64 / SINLUT_SCALE + 192;
		int r = (delta < 0 ? 255 : 64) * pulse >> 8;
		int g = (delta > 0 ? 255 : 64) * pulse >> 8;
		int b = (32 * pulse) >> 8;
		draw_number(FONT_SMALL, 120, 25, delta, r, g, b);
	}

	flip();
}

static void draw_scr_graph(void)
{
	static const unsigned short col[2] = { RGB(255, 75, 64), RGB(64, 255, 75) };
	static const unsigned short bgcol = 0;/*RGB(128, 128, 128);*/
	static const unsigned short axis_col = RGB(160, 160, 160);
	unsigned short *pixels = back_buffer->pixels;
	int i, j, k, baseline_y = 160 - CURVES_BASELINE;

	clear_buffer(back_buffer, bgcol);

	set_text_writebg(1);
	set_text_color(col[0], bgcol);
	draw_string("Player 1", 10, 5, back_buffer);
	set_text_color(col[1], bgcol);
	draw_string("Player 2", 160, 5, back_buffer);

	dma_fill16(3, pixels + baseline_y * 240 + 20, axis_col, 200);
	dma_fill16(3, pixels + curve[0][0].y * 240 + 20, RGB(80, 80, 80), 200);
	draw_line(20, baseline_y, 20, 20, axis_col, back_buffer);

	for(i=0; i<2; i++) {
		int hsz = dynarr_size(history[i]) + 1;	/* history & current score */

		for(j=0; j<hsz; j++) {
			struct vertex v0 = curve[i][j];

			if(v0.y > 0 && v0.y < back_buffer->y - 1) {
				unsigned short *pptr = pixels + v0.y * back_buffer->x + v0.x - 1;
				for(k=0; k<3; k++) {
					*pptr = pptr[-back_buffer->x] = pptr[back_buffer->x] = col[i];
					++pptr;
				}
			}

			if(j < hsz - 1) {
				struct vertex v1 = curve[i][j + 1];
				if(clip_line(&v0.x, &v0.y, &v1.x, &v1.y, 0, 0, 240, 158)) {
					draw_line(v0.x, v0.y, v1.x, v1.y, col[i], back_buffer);
				}
			}

		}
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

static void keypress_scr_main(unsigned short key)
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
		reset();
		break;

	case KEY_SELECT:
		build_curves();
		show_graph = 1;
		break;
	}
}

static void keypress_scr_graph(unsigned short key)
{
	switch(key) {
	case KEY_SELECT:
		show_graph = 0;
	}
}

static void build_curves(void)
{
	static const int maxy = 160;
	int i, j, range, yscale, max_score = starting_life;

	for(i=0; i<2; i++) {
		if(!(history[i] = dynarr_push(history[i], &score[i]))) {
			panic("build_curves: failed to push score");
		}

		free(curve[i]);
		if(!(curve[i] = malloc(dynarr_size(history[i]) * sizeof *curve[i]))) {
			panic("build_curves: failed to allocate curve");
		}
	}

	for(i=0; i<2; i++) {
		int hsz = dynarr_size(history[i]);
		for(j=0; j<hsz; j++) {
			int s = history[i][j];
			if(s > max_score)
				max_score = s;
		}
	}

	range = max_score + CURVES_BASELINE;
	yscale = 200 / range;

	for(i=0; i<2; i++) {
		int hsz = dynarr_size(history[i]);
		int dx = 200 / (hsz - 1);

		int x = 20;
		for(j=0; j<hsz; j++) {
			struct vertex *v = curve[i] + j;

			v->x = x;
			v->y = maxy - CURVES_BASELINE - history[i][j] * yscale;
			x += dx;
		}
	}

	history[0] = dynarr_pop(history[0]);
	history[1] = dynarr_pop(history[1]);
}


static void keyb_intr(void)
{
	int i;
	unsigned short prev_keystate = keystate;
	unsigned short diff;
	static unsigned int prev_intr;

	unsigned int msec = get_millisec();
	if(msec - prev_intr < 128) {
		return;
	}
	prev_intr = msec;

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
