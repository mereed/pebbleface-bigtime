/*
Copyright (C) 2015 Mark Reed

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "pebble.h"
#include "effect_layer.h"

static AppSync sync;
static uint8_t sync_buffer[256];

enum WeatherKey {
  BLUETOOTHVIBE_KEY = 0x0,
  HOURLYVIBE_KEY = 0x1,
  INVERT_COLOR_KEY = 0x2,
  FLIP_KEY = 0x3,
  FONT_KEY = 0x4,
  BLINK_KEY = 0x5
};

//EffectLayer  *effect_layer;
EffectLayer  *effect_layer_mask;
static EffectMask mask;

static int bluetoothvibe;
static int hourlyvibe;
static int invert;
static int flip;
static int font;
static int blink;

static Window *window;
static Layer *window_layer;

static GBitmap *bluetooth_image;
static BitmapLayer *bluetooth_layer;

static Layer *new_layer;
static Layer *blockout_layer;

static bool appStarted = false;

BitmapLayer *layer_batt_img;
GBitmap *img_battery_100;
GBitmap *img_battery_80;
GBitmap *img_battery_60;
GBitmap *img_battery_40;
GBitmap *img_battery_20;
GBitmap *img_battery_00;
GBitmap *img_battery_charge;
int charge_percent = 0;

#define TOTAL_TIME_DIGITS 4  //90 counter-clockwise
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

const int BIG_DIGIT_SQ_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_SQ0,
  RESOURCE_ID_IMAGE_NUM_SQ1,
  RESOURCE_ID_IMAGE_NUM_SQ2,
  RESOURCE_ID_IMAGE_NUM_SQ3,
  RESOURCE_ID_IMAGE_NUM_SQ4,
  RESOURCE_ID_IMAGE_NUM_SQ5,
  RESOURCE_ID_IMAGE_NUM_SQ6,
  RESOURCE_ID_IMAGE_NUM_SQ7,
  RESOURCE_ID_IMAGE_NUM_SQ8,
  RESOURCE_ID_IMAGE_NUM_SQ9
};

#define TOTAL_FLIP_DIGITS 4 //90 clockwise
static GBitmap *ftime_digits_images[TOTAL_FLIP_DIGITS];
static BitmapLayer *ftime_digits_layers[TOTAL_FLIP_DIGITS];

const int FLIP_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_M0,
  RESOURCE_ID_IMAGE_NUM_M1,
  RESOURCE_ID_IMAGE_NUM_M2,
  RESOURCE_ID_IMAGE_NUM_M3,
  RESOURCE_ID_IMAGE_NUM_M4,
  RESOURCE_ID_IMAGE_NUM_M5,
  RESOURCE_ID_IMAGE_NUM_M6,
  RESOURCE_ID_IMAGE_NUM_M7,
  RESOURCE_ID_IMAGE_NUM_M8,
  RESOURCE_ID_IMAGE_NUM_M9
};

const int FLIP_DIGIT_SQ_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_MSQ0,
  RESOURCE_ID_IMAGE_NUM_MSQ1,
  RESOURCE_ID_IMAGE_NUM_MSQ2,
  RESOURCE_ID_IMAGE_NUM_MSQ3,
  RESOURCE_ID_IMAGE_NUM_MSQ4,
  RESOURCE_ID_IMAGE_NUM_MSQ5,
  RESOURCE_ID_IMAGE_NUM_MSQ6,
  RESOURCE_ID_IMAGE_NUM_MSQ7,
  RESOURCE_ID_IMAGE_NUM_MSQ8,
  RESOURCE_ID_IMAGE_NUM_MSQ9
};

#define TOTAL_NORMAL_DIGITS 4  //normal
static GBitmap *normal_time_digits_images[TOTAL_NORMAL_DIGITS];
static BitmapLayer *normal_time_digits_layers[TOTAL_NORMAL_DIGITS];

const int NORMAL_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_S0,
  RESOURCE_ID_IMAGE_NUM_S1,
  RESOURCE_ID_IMAGE_NUM_S2,
  RESOURCE_ID_IMAGE_NUM_S3,
  RESOURCE_ID_IMAGE_NUM_S4,
  RESOURCE_ID_IMAGE_NUM_S5,
  RESOURCE_ID_IMAGE_NUM_S6,
  RESOURCE_ID_IMAGE_NUM_S7,
  RESOURCE_ID_IMAGE_NUM_S8,
  RESOURCE_ID_IMAGE_NUM_S9,
};

const int NORMAL_SQ_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_NSQ0,
  RESOURCE_ID_IMAGE_NUM_NSQ1,
  RESOURCE_ID_IMAGE_NUM_NSQ2,
  RESOURCE_ID_IMAGE_NUM_NSQ3,
  RESOURCE_ID_IMAGE_NUM_NSQ4,
  RESOURCE_ID_IMAGE_NUM_NSQ5,
  RESOURCE_ID_IMAGE_NUM_NSQ6,
  RESOURCE_ID_IMAGE_NUM_NSQ7,
  RESOURCE_ID_IMAGE_NUM_NSQ8,
  RESOURCE_ID_IMAGE_NUM_NSQ9,
};

static GBitmap *month_image;
static BitmapLayer *month_layer;

const int MONTH_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_MONTH_JAN,
  RESOURCE_ID_IMAGE_MONTH_FEB,
  RESOURCE_ID_IMAGE_MONTH_MAR,
  RESOURCE_ID_IMAGE_MONTH_APR,
  RESOURCE_ID_IMAGE_MONTH_MAY,
  RESOURCE_ID_IMAGE_MONTH_JUN,
  RESOURCE_ID_IMAGE_MONTH_JUL,
  RESOURCE_ID_IMAGE_MONTH_AUG,
  RESOURCE_ID_IMAGE_MONTH_SEP,
  RESOURCE_ID_IMAGE_MONTH_OCT,
  RESOURCE_ID_IMAGE_MONTH_NOV,
  RESOURCE_ID_IMAGE_MONTH_DEC
};

static GBitmap *r_month_image;
static BitmapLayer *r_month_layer;

const int MONTH_RIGHT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_RMONTH_JAN,
  RESOURCE_ID_IMAGE_RMONTH_FEB,
  RESOURCE_ID_IMAGE_RMONTH_MAR,
  RESOURCE_ID_IMAGE_RMONTH_APR,
  RESOURCE_ID_IMAGE_RMONTH_MAY,
  RESOURCE_ID_IMAGE_RMONTH_JUN,
  RESOURCE_ID_IMAGE_RMONTH_JUL,
  RESOURCE_ID_IMAGE_RMONTH_AUG,
  RESOURCE_ID_IMAGE_RMONTH_SEP,
  RESOURCE_ID_IMAGE_RMONTH_OCT,
  RESOURCE_ID_IMAGE_RMONTH_NOV,
  RESOURCE_ID_IMAGE_RMONTH_DEC
};

static GBitmap *l_month_image;
static BitmapLayer *l_month_layer;

const int MONTH_LEFT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_LMONTH_JAN,
  RESOURCE_ID_IMAGE_LMONTH_FEB,
  RESOURCE_ID_IMAGE_LMONTH_MAR,
  RESOURCE_ID_IMAGE_LMONTH_APR,
  RESOURCE_ID_IMAGE_LMONTH_MAY,
  RESOURCE_ID_IMAGE_LMONTH_JUN,
  RESOURCE_ID_IMAGE_LMONTH_JUL,
  RESOURCE_ID_IMAGE_LMONTH_AUG,
  RESOURCE_ID_IMAGE_LMONTH_SEP,
  RESOURCE_ID_IMAGE_LMONTH_OCT,
  RESOURCE_ID_IMAGE_LMONTH_NOV,
  RESOURCE_ID_IMAGE_LMONTH_DEC
};

#define TOTAL_DATE_DIGITS 2	
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

const int DATE_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};

#define TOTAL_LDATE_DIGITS 2	
static GBitmap *l_date_digits_images[TOTAL_LDATE_DIGITS];
static BitmapLayer *l_date_digits_layers[TOTAL_LDATE_DIGITS];

const int LDATE_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_LDATENUM_0,
  RESOURCE_ID_IMAGE_LDATENUM_1,
  RESOURCE_ID_IMAGE_LDATENUM_2,
  RESOURCE_ID_IMAGE_LDATENUM_3,
  RESOURCE_ID_IMAGE_LDATENUM_4,
  RESOURCE_ID_IMAGE_LDATENUM_5,
  RESOURCE_ID_IMAGE_LDATENUM_6,
  RESOURCE_ID_IMAGE_LDATENUM_7,
  RESOURCE_ID_IMAGE_LDATENUM_8,
  RESOURCE_ID_IMAGE_LDATENUM_9
};

#define TOTAL_RDATE_DIGITS 2	
static GBitmap *r_date_digits_images[TOTAL_RDATE_DIGITS];
static BitmapLayer *r_date_digits_layers[TOTAL_RDATE_DIGITS];

const int RDATE_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_RDATENUM_0,
  RESOURCE_ID_IMAGE_RDATENUM_1,
  RESOURCE_ID_IMAGE_RDATENUM_2,
  RESOURCE_ID_IMAGE_RDATENUM_3,
  RESOURCE_ID_IMAGE_RDATENUM_4,
  RESOURCE_ID_IMAGE_RDATENUM_5,
  RESOURCE_ID_IMAGE_RDATENUM_6,
  RESOURCE_ID_IMAGE_RDATENUM_7,
  RESOURCE_ID_IMAGE_RDATENUM_8,
  RESOURCE_ID_IMAGE_RDATENUM_9
};


static void handle_tick(struct tm *tick_time, TimeUnits units_changed);


void rotate () {
			  
  switch (flip) {
  
	case 0:  //normal aspect		
	    layer_set_hidden(blockout_layer, true);
	  	layer_set_hidden( bitmap_layer_get_layer(r_month_layer), true);
	  	layer_set_hidden( bitmap_layer_get_layer(l_month_layer), true);
	  
		for (int i = 0; i < TOTAL_RDATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(r_date_digits_layers[i]), true);
			}	  
	  
		for (int i = 0; i < TOTAL_LDATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(l_date_digits_layers[i]), true);
			}	

		for (int i = 0; i < TOTAL_FLIP_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(ftime_digits_layers[i]), true);
			}

 		for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(time_digits_layers[i]), true);
			}
	  
 		for (int i = 0; i < TOTAL_NORMAL_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(normal_time_digits_layers[i]), false);
			}
	  
	    layer_set_hidden( bitmap_layer_get_layer(month_layer), false);
	  
		for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(date_digits_layers[i]), false);
			}	  
	break;
		  
	case 1: //90 ccw	
        layer_set_hidden( bitmap_layer_get_layer(month_layer), true);
	  	layer_set_hidden( bitmap_layer_get_layer(r_month_layer), true);
	  	layer_set_hidden( bitmap_layer_get_layer(l_month_layer), false);

		for (int i = 0; i < TOTAL_RDATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(r_date_digits_layers[i]), true);
			}	  
	  
		for (int i = 0; i < TOTAL_LDATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(l_date_digits_layers[i]), false);
			}	

	  	  
		for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(date_digits_layers[i]), true);
			}	  
		  
		for (int i = 0; i < TOTAL_NORMAL_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(normal_time_digits_layers[i]), true);
			}

	    layer_set_hidden(blockout_layer, false);

		for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(time_digits_layers[i]), false);
			}
	break;
		  
	case 2: // 90 cw
	  
	     layer_set_hidden(blockout_layer, true);
	  	 layer_set_hidden( bitmap_layer_get_layer(r_month_layer), false);
	  	 layer_set_hidden( bitmap_layer_get_layer(l_month_layer), true);
         layer_set_hidden( bitmap_layer_get_layer(month_layer), true);

		for (int i = 0; i < TOTAL_RDATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(r_date_digits_layers[i]), false);
			}	  
	  
		for (int i = 0; i < TOTAL_LDATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(l_date_digits_layers[i]), true);
			}	

		for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(date_digits_layers[i]), true);
			}	  
	  
		for (int i = 0; i < TOTAL_NORMAL_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(normal_time_digits_layers[i]), true);
			}
	
		for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(time_digits_layers[i]), true);
			}
				
		for (int i = 0; i < TOTAL_FLIP_DIGITS; ++i) {
        layer_set_hidden( bitmap_layer_get_layer(ftime_digits_layers[i]), false);
			}
	break;

  }
		// refresh time display.
  		time_t now = time(NULL);
  		struct tm *tick_time = localtime(&now);  
  		handle_tick(tick_time, MONTH_UNIT + DAY_UNIT + HOUR_UNIT + MINUTE_UNIT );
 
}

static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
#ifdef PBL_PLATFORM_BASALT
  GRect bitmap_bounds = gbitmap_get_bounds((*bmp_image));
#else
  GRect bitmap_bounds = (*bmp_image)->bounds;
#endif
  GRect frame = GRect(origin.x, origin.y, bitmap_bounds.size.w, bitmap_bounds.size.h);
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}
/*
void set_invert_color(bool invert) {
  if (invert && effect_layer == NULL) {
    // Add inverter layer
    Layer *window_layer = window_get_root_layer(window);

    effect_layer = effect_layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_layer, effect_layer_get_layer(effect_layer));
  } else if (!invert && effect_layer != NULL) {
    // Remove Inverter layer
    layer_remove_from_parent(effect_layer_get_layer(effect_layer));
    effect_layer_destroy(effect_layer);
    effect_layer = NULL;
  }
  // No action required
}
*/
static void sync_tuple_changed_callback(const uint32_t key,
                                        const Tuple* new_tuple,
                                        const Tuple* old_tuple,
                                        void* context) {

  // App Sync keeps new_tuple in sync_buffer, so we may use it directly
  switch (key) {
	  
    case BLUETOOTHVIBE_KEY:
      bluetoothvibe = new_tuple->value->uint8 != 0;
	  persist_write_bool(BLUETOOTHVIBE_KEY, bluetoothvibe);
      break;      
	  
    case HOURLYVIBE_KEY:
      hourlyvibe = new_tuple->value->uint8 != 0;
	  persist_write_bool(HOURLYVIBE_KEY, hourlyvibe);	  
      break;	   
	  
	case FLIP_KEY:
      flip = new_tuple->value->uint8;
	  persist_write_bool(FLIP_KEY, flip);
	  rotate();
	break;
	  
	case FONT_KEY:
      font = new_tuple->value->uint8 != 0;
	  persist_write_bool(FONT_KEY, font);
	break;
	  
	case INVERT_COLOR_KEY:
      invert = new_tuple->value->uint8;
	  persist_write_bool(INVERT_COLOR_KEY, invert);
     // set_invert_color(invert);
    break;	
	
	case BLINK_KEY:
      blink = new_tuple->value->uint8;
	  persist_write_bool(BLINK_KEY, blink);
      tick_timer_service_unsubscribe();
      if(blink) {
        tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
      }
      else {
        tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	  }
   break;	
  }
}

void update_battery(BatteryChargeState charge_state) {

    if (charge_state.is_charging) {
        bitmap_layer_set_bitmap(layer_batt_img, img_battery_charge);

    } else {
		  if (charge_state.charge_percent <= 05) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_00);
    	} else if (charge_state.charge_percent <= 20) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_20);	
        } else if (charge_state.charge_percent <= 40) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_40);
        } else if (charge_state.charge_percent <= 60) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_60);
		} else if (charge_state.charge_percent <= 80) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_80);
		} else if (charge_state.charge_percent <= 100) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);	
    }
    charge_percent = charge_state.charge_percent;
    }
}

static void toggle_bluetooth_icon(bool connected) {
  if(appStarted && !connected && bluetoothvibe) {
    //vibe!
    vibes_short_pulse();
  }
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), connected);
}


void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

static void update_days(struct tm *tick_time) {
  set_container_image(&month_image, month_layer, MONTH_IMAGE_RESOURCE_IDS[tick_time->tm_mon], GPoint(0, 158));
  set_container_image(&r_month_image, r_month_layer, MONTH_RIGHT_IMAGE_RESOURCE_IDS[tick_time->tm_mon], GPoint(135, 0));
  set_container_image(&l_month_image, l_month_layer, MONTH_LEFT_IMAGE_RESOURCE_IDS[tick_time->tm_mon], GPoint(0, 0));

  set_container_image(&date_digits_images[0], date_digits_layers[0], DATE_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(40, 157));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATE_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(48, 157));
	
  set_container_image(&l_date_digits_images[0], l_date_digits_layers[0], LDATE_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(0, 60));
  set_container_image(&l_date_digits_images[1], l_date_digits_layers[1], LDATE_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(0, 68));
	
  set_container_image(&r_date_digits_images[0], r_date_digits_layers[0], RDATE_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(135, 108));
  set_container_image(&r_date_digits_images[1], r_date_digits_layers[1], RDATE_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(135, 100));
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void update_hours(struct tm *tick_time) {

	if (appStarted && hourlyvibe) {
    //vibe!
    vibes_short_pulse();
	}
	
  unsigned short display_hour = get_display_hour(tick_time->tm_hour);
if (font) {
  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(12, 1));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(12, 37));

  set_container_image(&ftime_digits_images[0], ftime_digits_layers[0], FLIP_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(1, 128));
  set_container_image(&ftime_digits_images[1], ftime_digits_layers[1], FLIP_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(1, 91));

  set_container_image(&normal_time_digits_images[0], normal_time_digits_layers[0], NORMAL_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(0, 4));
  set_container_image(&normal_time_digits_images[1], normal_time_digits_layers[1], NORMAL_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(34, 4));

} else {
	
  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_SQ_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(12, 2));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_SQ_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(12, 40));

  set_container_image(&ftime_digits_images[0], ftime_digits_layers[0], FLIP_DIGIT_SQ_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(2, 131));
  set_container_image(&ftime_digits_images[1], ftime_digits_layers[1], FLIP_DIGIT_SQ_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(2, 93));

  set_container_image(&normal_time_digits_images[0], normal_time_digits_layers[0], NORMAL_SQ_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(3, 4));
  set_container_image(&normal_time_digits_images[1], normal_time_digits_layers[1], NORMAL_SQ_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(36, 4));
}
	
	if (!clock_is_24h_style()) {
	
	if (flip == 0 && display_hour/10 == 0) {		
		 	layer_set_hidden(bitmap_layer_get_layer(normal_time_digits_layers[0]), true); 
		  } else  if (flip == 0) {
		    layer_set_hidden(bitmap_layer_get_layer(normal_time_digits_layers[0]), false);
            	}
	if (flip == 1 && display_hour/10 == 0) {		
		 	layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true); 
		  } else  if (flip == 1) {
		    layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
            	}
	if (flip == 2 && display_hour/10 == 0) {		
		 	layer_set_hidden(bitmap_layer_get_layer(ftime_digits_layers[0]), true); 
		  } else  if (flip == 2) {
		    layer_set_hidden(bitmap_layer_get_layer(ftime_digits_layers[0]), false);
            	}
	}		
}

static void update_minutes(struct tm *tick_time) {
	if (font) {
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(12, 91));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(12, 128));
			
  set_container_image(&ftime_digits_images[2], ftime_digits_layers[2], FLIP_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(1, 36));
  set_container_image(&ftime_digits_images[3], ftime_digits_layers[3], FLIP_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(1, 1));

  set_container_image(&normal_time_digits_images[2], normal_time_digits_layers[2], NORMAL_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(77, 4));
  set_container_image(&normal_time_digits_images[3], normal_time_digits_layers[3], NORMAL_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(111, 4));
	
	} else {
		
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_SQ_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(12, 91));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_SQ_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(12, 129));
			
  set_container_image(&ftime_digits_images[2], ftime_digits_layers[2], FLIP_DIGIT_SQ_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(2, 40));
  set_container_image(&ftime_digits_images[3], ftime_digits_layers[3], FLIP_DIGIT_SQ_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(2, 2));

  set_container_image(&normal_time_digits_images[2], normal_time_digits_layers[2], NORMAL_SQ_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(79, 4));
  set_container_image(&normal_time_digits_images[3], normal_time_digits_layers[3], NORMAL_SQ_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(112, 4));		
	}
}

static void update_seconds(struct tm *tick_time) {
  if(blink) {
    layer_set_hidden(bitmap_layer_get_layer(layer_batt_img), tick_time->tm_sec%5);
  }
  else {
    if(layer_get_hidden(bitmap_layer_get_layer(layer_batt_img))) {
      layer_set_hidden(bitmap_layer_get_layer(layer_batt_img), false);
    }
  }
}

static void canvas_update_proc(Layer *new_layer, GContext *ctx) {
//  GRect bounds = layer_get_bounds(new_layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect (0,0,144,168),  0,  0);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	
 if (units_changed & MONTH_UNIT) {
    update_days(tick_time);
  }
  if (units_changed & DAY_UNIT) {
    update_days(tick_time);
  }
  if (units_changed & HOUR_UNIT) {
   update_hours(tick_time);
  }
  if (units_changed & MINUTE_UNIT) {
   update_minutes(tick_time);
  }
  if (units_changed & SECOND_UNIT) {
    update_seconds(tick_time);
  }			
}

void force_update(void) {
    update_battery(battery_state_service_peek());
    toggle_bluetooth_icon(bluetooth_connection_service_peek());
}

static void init(void) {
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
	
  memset(&ftime_digits_layers, 0, sizeof(ftime_digits_layers));
  memset(&ftime_digits_images, 0, sizeof(ftime_digits_images));

  memset(&normal_time_digits_layers, 0, sizeof(normal_time_digits_layers));
  memset(&normal_time_digits_images, 0, sizeof(normal_time_digits_images));
	
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
	
  memset(&l_date_digits_layers, 0, sizeof(l_date_digits_layers));
  memset(&l_date_digits_images, 0, sizeof(l_date_digits_images));
	
  memset(&r_date_digits_layers, 0, sizeof(r_date_digits_layers));
  memset(&r_date_digits_images, 0, sizeof(r_date_digits_images));
	
	
  // Setup messaging
  const int inbound_size = 256;
  const int outbound_size = 256;
  app_message_open(inbound_size, outbound_size);	
	
  window = window_create();
  if (window == NULL) {
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }
  window_stack_push(window, true /* Animated */);
  window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorBlack);


  GRect dummy_frame = { {0, 0}, {0, 0} };

	for (int i = 0; i < TOTAL_FLIP_DIGITS; ++i) {
    ftime_digits_layers[i] = bitmap_layer_create(dummy_frame);  
    layer_add_child(window_layer, bitmap_layer_get_layer(ftime_digits_layers[i]));
  }

  // Create Layer
  blockout_layer = layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  layer_add_child(window_layer, blockout_layer);

  // Set the update_proc
  layer_set_update_proc(blockout_layer, canvas_update_proc);
	
	
	for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }

	for (int i = 0; i < TOTAL_NORMAL_DIGITS; ++i) {
    normal_time_digits_layers[i] = bitmap_layer_create(dummy_frame);
   layer_add_child(window_layer, bitmap_layer_get_layer(normal_time_digits_layers[i]));
  }	
 
	
	month_layer = bitmap_layer_create(dummy_frame);
   layer_add_child(window_layer, bitmap_layer_get_layer(month_layer));
	r_month_layer = bitmap_layer_create(dummy_frame);
   layer_add_child(window_layer, bitmap_layer_get_layer(r_month_layer));	
	l_month_layer = bitmap_layer_create(dummy_frame);
   layer_add_child(window_layer, bitmap_layer_get_layer(l_month_layer));	
	
    for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }	
	
    for (int i = 0; i < TOTAL_LDATE_DIGITS; ++i) {
    l_date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(l_date_digits_layers[i]));
  }		
	
    for (int i = 0; i < TOTAL_RDATE_DIGITS; ++i) {
    r_date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(r_date_digits_layers[i]));
  }	
	
	
  mask.bitmap_background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MASK);

	
	 // ** { begin setup mask for MASK effect
  mask.text = NULL;
  mask.bitmap_mask = NULL;
//  mask.mask_colors = GColorWhite;
	
  #ifdef PBL_COLOR
    mask.mask_colors = malloc(sizeof(GColor)*4);
    mask.mask_colors[0] = GColorWhite;
    mask.mask_colors[1] = GColorLightGray;
    mask.mask_colors[2] = GColorDarkGray;
//    mask.mask_colors[3] = GColorClear;
  #else
    mask.mask_colors = malloc(sizeof(GColor)*2);
    mask.mask_colors[0] = GColorWhite;
    mask.mask_colors[1] = GColorClear;
  #endif
	  
  mask.background_color = GColorClear;
  
  // ** end setup mask }

	
  //creating effect layer
  effect_layer_mask = effect_layer_create(GRect(0,0,144,168));
  effect_layer_add_effect(effect_layer_mask, effect_mask, &mask);
  layer_add_child((window_layer), effect_layer_get_layer(effect_layer_mask));

	
	img_battery_100   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_080_100);
    img_battery_80   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_060_080);
    img_battery_60   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_040_060);
    img_battery_40   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_020_040);
    img_battery_20   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_005_020);
    img_battery_00   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_000_005);
    img_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_CHARGING);
    layer_batt_img  = bitmap_layer_create(GRect(66, 76, 13, 13));
	bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
	layer_add_child(window_layer, bitmap_layer_get_layer(layer_batt_img));
		

   bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_ICON_NOBLUETOOTH);
#ifdef PBL_PLATFORM_BASALT
  GRect bitmap_bounds_bt_on = gbitmap_get_bounds(bluetooth_image);
#else
  GRect bitmap_bounds_bt_on = bluetooth_image->bounds;
#endif	
  GRect frame_bt = GRect(66, 76, bitmap_bounds_bt_on.size.w, bitmap_bounds_bt_on.size.h);
  bluetooth_layer = bitmap_layer_create(frame_bt);
  bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));
	
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, MONTH_UNIT + DAY_UNIT + HOUR_UNIT + MINUTE_UNIT + SECOND_UNIT);

  Tuplet initial_values[] = {
    TupletInteger(BLUETOOTHVIBE_KEY, persist_read_bool(BLUETOOTHVIBE_KEY)),
    TupletInteger(HOURLYVIBE_KEY, persist_read_bool(HOURLYVIBE_KEY)),
	TupletInteger(FLIP_KEY, persist_read_bool(FLIP_KEY)),
	TupletInteger(FONT_KEY, persist_read_bool(FONT_KEY)),
  	TupletInteger(INVERT_COLOR_KEY, persist_read_bool(INVERT_COLOR_KEY)),
	TupletInteger(BLINK_KEY, persist_read_bool(BLINK_KEY)),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values,
                ARRAY_LENGTH(initial_values), sync_tuple_changed_callback,
                NULL, NULL);

  appStarted = true;
 
	 // handlers
    battery_state_service_subscribe(&update_battery);
    bluetooth_connection_service_subscribe(&bluetooth_connection_callback);
    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

	 // draw first frame
    force_update();
}

static void deinit(void) {
  app_sync_deinit(&sync);

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();

  layer_remove_from_parent(bitmap_layer_get_layer(month_layer));
  bitmap_layer_destroy(month_layer);
  gbitmap_destroy(month_image);

  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
  gbitmap_destroy(bluetooth_image);
	
  layer_remove_from_parent(bitmap_layer_get_layer(r_month_layer));
  bitmap_layer_destroy(r_month_layer);
  gbitmap_destroy(r_month_image);
	
  layer_remove_from_parent(bitmap_layer_get_layer(l_month_layer));
  bitmap_layer_destroy(l_month_layer);
  gbitmap_destroy(l_month_image);
	
  layer_remove_from_parent(bitmap_layer_get_layer(layer_batt_img));
  bitmap_layer_destroy(layer_batt_img);
  gbitmap_destroy(img_battery_100);
  gbitmap_destroy(img_battery_80);
  gbitmap_destroy(img_battery_60);
  gbitmap_destroy(img_battery_40);
  gbitmap_destroy(img_battery_20);
  gbitmap_destroy(img_battery_00);
  gbitmap_destroy(img_battery_charge);	
	
	
	for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    time_digits_images[i] = NULL;
    bitmap_layer_destroy(time_digits_layers[i]);
	time_digits_layers[i] = NULL;
  }

	for (int i = 0; i < TOTAL_FLIP_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(ftime_digits_layers[i]));
    gbitmap_destroy(ftime_digits_images[i]);
    ftime_digits_images[i] = NULL;
    bitmap_layer_destroy(ftime_digits_layers[i]);
	ftime_digits_layers[i] = NULL;
  }

	for (int i = 0; i < TOTAL_NORMAL_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(normal_time_digits_layers[i]));
    gbitmap_destroy(normal_time_digits_images[i]);
    normal_time_digits_images[i] = NULL;
    bitmap_layer_destroy(normal_time_digits_layers[i]);
	normal_time_digits_layers[i] = NULL;
  }

	for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    date_digits_images[i] = NULL;
    bitmap_layer_destroy(date_digits_layers[i]);
	date_digits_layers[i] = NULL;
  }
	
	for (int i = 0; i < TOTAL_RDATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(r_date_digits_layers[i]));
    gbitmap_destroy(r_date_digits_images[i]);
    r_date_digits_images[i] = NULL;
    bitmap_layer_destroy(r_date_digits_layers[i]);
	r_date_digits_layers[i] = NULL;
  }
	
	for (int i = 0; i < TOTAL_LDATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(l_date_digits_layers[i]));
    gbitmap_destroy(l_date_digits_images[i]);
    l_date_digits_images[i] = NULL;
    bitmap_layer_destroy(l_date_digits_layers[i]);
	l_date_digits_layers[i] = NULL;
  }
	
 //if (effect_layer != NULL) {
//	  effect_layer_destroy(effect_layer);
//  }
	
  layer_remove_from_parent(blockout_layer);
  layer_destroy(blockout_layer);
	
  layer_remove_from_parent(new_layer);
  layer_destroy(new_layer);
	
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
	

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}