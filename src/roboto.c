#include "pebble.h"

/* #include "http.h" */
#include "util.h"
#include "weather_layer.h"
#include "time_layer.h"
/* #include "link_monitor.h" */
/* #include "config.h" */

#define TIME_FRAME      (GRect(0, 6, 144, 168-6))
#define DATE_FRAME      (GRect(0, 62, 144, 168-62))

/* // POST variables */
/* #define WEATHER_KEY_LATITUDE 1 */
/* #define WEATHER_KEY_LONGITUDE 2 */
/* #define WEATHER_KEY_UNIT_SYSTEM 3 */
	
/* // Received variables */
/* #define WEATHER_KEY_ICON 1 */
/* #define WEATHER_KEY_TEMPERATURE 2 */
	
/* #define WEATHER_HTTP_COOKIE 1949327671 */
/* #define TIME_HTTP_COOKIE 1131038282 */

static Window * window;          /* main window */
static TextLayer * date_layer;   /* layer for the date */
static TimeLayer * time_layer;   /* layer for the time */

static GFont * font_date;        /* font for date (normal) */
static GFont * font_hour;        /* font for hour (bold) */
static GFont * font_minute;      /* font for minute (thin) */

/* //Weather Stuff */
/* static int our_latitude, our_longitude; */
/* static bool located = false; */

static WeatherLayer * weather_layer;

/* void request_weather(); */

/* void failed(int32_t cookie, int http_status, void* context) { */
/* 	if(cookie == 0 || cookie == WEATHER_HTTP_COOKIE) { */
/* 		weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER); */
/* 		text_layer_set_text(&weather_layer.temp_layer, "---°"); */
/* 	} */
	
/* 	link_monitor_handle_failure(http_status); */
	
/* 	//Re-request the location and subsequently weather on next minute tick */
/* 	located = false; */
/* } */

/* void success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) { */
/* 	if(cookie != WEATHER_HTTP_COOKIE) return; */
/* 	Tuple* icon_tuple = dict_find(received, WEATHER_KEY_ICON); */
/* 	if(icon_tuple) { */
/* 		int icon = icon_tuple->value->int8; */
/* 		if(icon >= 0 && icon < 16) { */
/* 			weather_layer_set_icon(&weather_layer, icon); */
/* 		} else { */
/* 			weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER); */
/* 		} */
/* 	} */
/* 	Tuple* temperature_tuple = dict_find(received, WEATHER_KEY_TEMPERATURE); */
/* 	if(temperature_tuple) { */
/* 		weather_layer_set_temperature(&weather_layer, temperature_tuple->value->int16); */
/* 	} */
	
/* 	link_monitor_handle_success(); */
/* } */

/* void location(float latitude, float longitude, float altitude, float accuracy, void* context) { */
/* 	// Fix the floats */
/* 	our_latitude = latitude * 10000; */
/* 	our_longitude = longitude * 10000; */
/* 	located = true; */
/* 	request_weather(); */
/* } */

/* void reconnect(void* context) { */
/* 	located = false; */
/* 	request_weather(); */
/* } */

/* void request_weather(); */

void set_uninit_weather()
{
  weather_layer_set_icon(weather_layer, WEATHER_ICON_NO_WEATHER);
  WeatherData * wd = layer_get_data(weather_layer);
  text_layer_set_text(wd->temp_layer, "---°");
}

static void display_time(struct tm *tick_time, TimeUnits units_changed)
{
  /* Need to be static because pointers to them are stored in the text
   * layers.
   */
  static char date_text[] = "XXX, XXX 00";
  static char hour_text[] = "00";
  static char minute_text[] = ":00";

  if(units_changed & DAY_UNIT)
  {
    strftime(date_text, sizeof(date_text), "%a, %b %d", tick_time);
    text_layer_set_text(date_layer, date_text);
  }

  if(clock_is_24h_style())
  {
    strftime(hour_text, sizeof(hour_text), "%H", tick_time);
  }
  else
  {
    strftime(hour_text, sizeof(hour_text), "%I", tick_time);
    if(hour_text[0] == '0')
    {
      /* This is a hack to get rid of the leading zero.
       */
      memmove(&hour_text[0], &hour_text[1], sizeof(hour_text) - 1);
    }
  }

  strftime(minute_text, sizeof(minute_text), ":%M", tick_time);
  time_layer_set_text(time_layer, hour_text, minute_text);

  /* if(!located || (t->tick_time->tm_min % 30) == initial_minute) */
  /* { */
  /*   //Every 30 minutes, request updated weather */
  /*   http_location_request(); */
  /* } */
  /* else */
  /* { */
  /*   //Every minute, ping the phone */
  /*   link_monitor_ping(); */
  /* } */
}

/* Update the time and date.
*/
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  display_time(tick_time, units_changed);
}

static void window_load(Window * window)
{
  ResHandle res_d;
  ResHandle res_h;
  ResHandle res_m;

  Layer *window_layer = window_get_root_layer(window);

  res_d = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21);
  res_h = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49);
  res_m = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_THIN_SUBSET_49);

  font_date = fonts_load_custom_font(res_d);
  font_hour = fonts_load_custom_font(res_h);
  font_minute = fonts_load_custom_font(res_m);

  // setup time layer
  time_layer = time_layer_create(TIME_FRAME);
  time_layer_set_text_color(time_layer, GColorWhite);
  time_layer_set_background_color(time_layer, GColorClear);
  time_layer_set_fonts(time_layer, font_hour, font_minute);
  layer_add_child(window_layer, time_layer);

  // setup date layer
  date_layer = text_layer_create(DATE_FRAME);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, font_date);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));

  // Add weather layer
  weather_layer = weather_layer_create(GPoint(0, 95)); //0, 100
  layer_add_child(window_layer, weather_layer);
	
  // setup unknown state for weather
  set_uninit_weather();

  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  display_time(tick_time, YEAR_UNIT | 
                          MONTH_UNIT | 
                          DAY_UNIT | 
                          HOUR_UNIT | 
                          MINUTE_UNIT | 
                          SECOND_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void window_unload(Window * window)
{
  // cleanup everything
  tick_timer_service_unsubscribe();
  time_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  weather_layer_destroy(weather_layer);
}

/* Initialize the application.
*/
static void init()
{
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  /* // TODO: put this in handlers, of in init/deinit? */
  /* window_set_window_handlers(window, (WindowHandlers) { */
  /*     .load = window_load, */
  /*     .unload = window_unload */
  /*     }); */
  window_load(window);
  window_stack_push(window, true /* Animated */);
}

/* Shut down the application
*/
static void deinit()
{
  window_unload(window);

  fonts_unload_custom_font(font_date);
  fonts_unload_custom_font(font_hour);
  fonts_unload_custom_font(font_minute);
  
  window_destroy(window);
}


/********************* Main Program *******************/

int main(void)
{
    init();
    app_event_loop();
    deinit();
    return 0;
}

/* void request_weather() { */
/* 	if(!located) { */
/* 		http_location_request(); */
/* 		return; */
/* 	} */
/* 	// Build the HTTP request */
/* 	DictionaryIterator *body; */
/* 	HTTPResult result = http_out_get("http://ofkorth.net/pebble/weather", WEATHER_HTTP_COOKIE, &body); */
/* 	if(result != HTTP_OK) { */
/* 		weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER); */
/* 		return; */
/* 	} */
/* 	dict_write_int32(body, WEATHER_KEY_LATITUDE, our_latitude); */
/* 	dict_write_int32(body, WEATHER_KEY_LONGITUDE, our_longitude); */
/* 	dict_write_cstring(body, WEATHER_KEY_UNIT_SYSTEM, UNIT_SYSTEM); */
/* 	// Send it. */
/* 	if(http_out_send() != HTTP_OK) { */
/* 		weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER); */
/* 		return; */
/* 	} */
/* } */
