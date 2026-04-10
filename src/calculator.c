#include <pebble.h>
#include "stdlib.h"

static Window *window;
static GBitmap *background;
static BitmapLayer *bitmap_layer;
static Layer *display_layer;

static unsigned char x, y, dir = 0;

static int pos_i = 0, pos_j = 3;

static signed char sign = 1;

static double a = 0.0, b = 0.0, m = 1.0;

static char state = 0, key = 0, op = 0;

static char *s;

static char str[64] = "";

static char* my_dtoa( double d ) {
  snprintf((char*)&str, 64, "%d.%08d", (int)d, (int)((d - (int)d) * 100000000) ); /*** %f is broken long live %f ***/
  return (char*)&str;
}

static void draw_sel(GContext * ctx, unsigned char x, unsigned char y) {
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, GPoint(x,y), GPoint(x+32,y));
	graphics_draw_line(ctx, GPoint(x+32,y+1), GPoint(x+32,y+19));
	graphics_draw_line(ctx, GPoint(x+31,y+19), GPoint(x+1,y+19));
	graphics_draw_line(ctx, GPoint(x,y+19), GPoint(x,y+1));
}

double eval_op(char op, double a, double b) {
	if ( op == '+' ) {
		return b + a;
	} else if ( op == '-' ) {
		return b - a;
	} else if ( op == '*' ) {
		return b * a;
	} else if ( op == '/' ) {
		if ( a != 0 ) {
			return b / a;
		}
	}
	return 0.0;
}

void do_calc() {
     if ( ( key == '-' ) && ( a == 0.0 ) && ( state == 0 ) ) {
	sign = sign * -1;
     }
     if ( ( key >= '0' ) && ( key <= '9' ) ) {
	if ( state == 0 ) {
        	a = a * 10 + (key - '0');
	} else if ( state == 1 ) {
		m /= 10;
		a += m * (key - '0');
	}
     } else if ( key == '.' ) {
	if ( state == 0 ) state = 1;
     } else if ( key == '=' ) {
	if ( op != 0 ) {
	    b = sign * b;
	    a = eval_op(op, a, b);
	    sign = 1;
	    state = 0;
	    op = 0;
	    m = 1.0;
	}
     } else {
	op = key;
	b = sign * a;
	a = 0.0;
	sign = 1;
	state = 0;
	m = 1.0;
     }
}

void display_layer_update_callback(Layer *me, GContext* ctx) {
  int xoff = 0, yoff = 0;
  #ifdef PBL_ROUND
  xoff = 19;
  yoff = -5;
  #endif
	if ( pos_i > 3 ) pos_i = 0;
	if ( pos_j > 3 ) pos_j = 0;
	if ( pos_i < 0 ) pos_i = 3;
	if ( pos_j < 0 ) pos_j = 3;
	x = 2 + xoff + ( pos_i * 36 );
	y = 57 + yoff + ( pos_j * 26 );

	if ( key > 0 ) {
		do_calc();
		key = 0;
	}

	s = my_dtoa( sign * a );

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_text_color(ctx, GColorWhite);

	graphics_draw_text(ctx, s,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(25 + xoff, 12 + yoff, 144-35, 32),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentRight,
                     NULL);


	s[0] = "^>v<"[dir];
	s[1] = 0;
	graphics_draw_text(ctx, s,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(5 + xoff, 32 + yoff, 25, 25),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentLeft,
                     NULL);


  if ( op != 0 ) {
    s[0] = op;
    graphics_draw_text(ctx, s,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(5 + xoff, 12 + yoff, 32, 25),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentLeft,
                     NULL);

    if ( b != 0 ) {
       s = my_dtoa( b );
       graphics_draw_text(ctx, s,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(25 + xoff, 32 + yoff, 144-35, 25),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentRight,
                     NULL);
    }
  }
  draw_sel(ctx, x, y);
}

void calculator_move_handler(ClickRecognizerRef recognizer, void *context) {
  if ( dir > 4 )
	dir = 0;
  if ( dir < 2 )
    if (dir == 0 )
	pos_j--;
    else
	pos_i++;
  else
    if ( dir == 2 )
	pos_j++;
    else
	pos_i--;
  layer_mark_dirty(display_layer);
}

static void calculator_press_handler(ClickRecognizerRef recognizer, void *context) {
	key = "7410852.963=/*-+"[(pos_i << 2) + pos_j];
	layer_mark_dirty(display_layer);
}

static void calculator_turn_handler(ClickRecognizerRef recognizer, void *context) {
	dir++;
	dir&=3;
	layer_mark_dirty(display_layer);
}

static void calculator_reset_handler(ClickRecognizerRef recognizer, void *context) {
	pos_i = 0; pos_j = 3;
	a = b = 0.0;
	m = 1.0;
	dir = state = key = op = 0;
	sign = 1;
	layer_mark_dirty(display_layer);
}

void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, calculator_move_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, calculator_press_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, calculator_turn_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT, 700, calculator_reset_handler, NULL);
}

void handle_init(void) {
  Layer *window_layer;
  GRect bounds;
  window = window_create();
  #ifndef PBL_COLOR
  #if defined(PBL_SDK_2)
  window_set_fullscreen(window, true);
  #endif
  #endif
  window_stack_push(window, true);

  window_set_background_color(window, GColorBlack);
  window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  bitmap_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(bitmap_layer, background);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));

  // Init the layer for the display
  display_layer = layer_create( PBL_IF_ROUND_ELSE(
    grect_inset(bounds, GEdgeInsets(10, -20, 0, 0)),
    bounds ) );
  layer_set_update_proc(display_layer, display_layer_update_callback);
  layer_add_child(window_layer, display_layer);

  window_set_click_config_provider(window, click_config_provider);
  layer_mark_dirty(display_layer);
}

static void handle_deinit() {
	layer_remove_from_parent(display_layer);
	gbitmap_destroy(background);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
