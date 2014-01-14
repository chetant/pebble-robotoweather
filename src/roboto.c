#include "pebble.h"

#include "util.h"
#include "weather_layer.h"
#include "time_layer.h"

#define TIME_FRAME      (GRect(0, 6, 144, 168-6))
#define DATE_FRAME      (GRect(0, 62, 144, 168-62))

static Window * window;          /* main window */
static TextLayer * date_layer;   /* layer for the date */
static TimeLayer * time_layer;   /* layer for the time */

static GFont * font_date;        /* font for date (normal) */
static GFont * font_hour;        /* font for hour (bold) */
static GFont * font_minute;      /* font for minute (thin) */

static WeatherLayer * weather_layer;

void set_uninit_weather()
{
  weather_layer_set_icon(weather_layer, WEATHER_ICON_NO_WEATHER);
  WeatherData * wd = layer_get_data(weather_layer);
  text_layer_set_text(wd->temp_layer, "---°");
}

static bool updateNow = false;

enum WeatherKey
{
  WEATHER_ICON_KEY = 0x0,
  WEATHER_TEMPERATURE_KEY = 0x1,
};

static void ping_phone_app()
{
  Tuplet dummy = TupletInteger(0, 0);
  DictionaryIterator * iter;
  app_message_outbox_begin(&iter);
  dict_write_tuplet(iter, &dummy);
  app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Sync ping");
}

static void msg_inbox_cb(DictionaryIterator * iter, void * cxt)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App inbox msg recv!");

  uint8_t icon;
  int16_t temperature;
  int keys_read = 0;

  Tuple * tuple = dict_read_first(iter);
  while(NULL != tuple)
  {
    switch(tuple->key)
    {
    case WEATHER_ICON_KEY:
      icon = tuple->value[0].uint8;
      ++keys_read;
      break;
    case WEATHER_TEMPERATURE_KEY:
      temperature = tuple->value[0].int16;
      ++keys_read;
      break;
    }
    tuple = dict_read_next(iter);
  }
  if(weather_layer == NULL || keys_read < 2)
    return;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting weather: %d, %i", icon, temperature);
  if(temperature > 999)
  {
    set_uninit_weather();
    // this is used as notification that the app is up
    // ping the phone now to get the real weather
    ping_phone_app();
  }
  else
  {
    weather_layer_set_icon(weather_layer, icon);
    weather_layer_set_temperature(weather_layer, temperature);
  }
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

// TODO: add link monitoring: easy to get udpates now, but how to show weather is stale?
// TODO: 1. mark as stale immediately, 2. mark as stale only after a certain time elapses without connection
}

/* Update the time and date.
*/
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  display_time(tick_time, units_changed);
  if(units_changed & HOUR_UNIT)
  {
    // time to update weather
    ping_phone_app();
    updateNow = false;
  }
}

static void window_load(Window * window)
{
  ResHandle res_h;
  ResHandle res_m;

  Layer *window_layer = window_get_root_layer(window);

  res_h = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49);
  res_m = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_THIN_SUBSET_49);

  font_date = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
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
  /* app_sync_deinit(&sync); */

  tick_timer_service_unsubscribe();
  time_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  weather_layer_destroy(weather_layer);
}

// init application
static void init()
{
  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_register_inbox_received(msg_inbox_cb);
  app_message_open(inbound_size, outbound_size);

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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App initio!!");
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
