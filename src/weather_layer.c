#include "pebble.h"
#include "util.h"
#include "weather_layer.h"

static uint8_t WEATHER_ICONS[] = {
	RESOURCE_ID_ICON_CLEAR_DAY,
	RESOURCE_ID_ICON_CLEAR_NIGHT,
	RESOURCE_ID_ICON_RAIN,
	RESOURCE_ID_ICON_SNOW,
	RESOURCE_ID_ICON_SLEET,
	RESOURCE_ID_ICON_WIND,
	RESOURCE_ID_ICON_FOG,
	RESOURCE_ID_ICON_CLOUDY,
	RESOURCE_ID_ICON_PARTLY_CLOUDY_DAY,
	RESOURCE_ID_ICON_PARTLY_CLOUDY_NIGHT,
	RESOURCE_ID_ICON_THUNDER,
	RESOURCE_ID_ICON_RAIN_SNOW,
	RESOURCE_ID_ICON_SNOW_SLEET,
	RESOURCE_ID_ICON_COLD,
	RESOURCE_ID_ICON_HOT,
	RESOURCE_ID_ICON_ERROR,
};

WeatherLayer* weather_layer_create(GPoint pos)
{
  WeatherLayer* wl = layer_create_with_data(GRect(pos.x, pos.y, 144, 68), sizeof(WeatherData));
  WeatherData* wd = layer_get_data(wl);
  // Add temperature layer
  wd->temp_layer = text_layer_create(GRect(70, 13, 68, 68));
  text_layer_set_text_alignment(wd->temp_layer, GTextAlignmentCenter);
  text_layer_set_font(wd->temp_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_background_color(wd->temp_layer, GColorBlack);
  text_layer_set_text_color(wd->temp_layer, GColorWhite);		
  layer_add_child(wl, text_layer_get_layer(wd->temp_layer));
  // Note absence of icon layer
  wd->has_weather_icon = false;
  return wl;
}

void weather_layer_set_icon(WeatherLayer* wl, WeatherIcon icon)
{
  WeatherData* wd = layer_get_data(wl);
  if(wd->has_weather_icon)
  {
    layer_remove_from_parent(bitmap_layer_get_layer(wd->icon_layer));
    bitmap_layer_destroy(wd->icon_layer);
    gbitmap_destroy(wd->icon);
    wd->has_weather_icon = false;
  }
  wd->icon_layer = bitmap_layer_create(GRect(10, 4, 60,60));
  wd->icon = gbitmap_create_with_resource(WEATHER_ICONS[icon]);
  bitmap_layer_set_bitmap(wd->icon_layer, wd->icon);
  layer_add_child(wl, bitmap_layer_get_layer(wd->icon_layer));
  wd->has_weather_icon = true;
}

void weather_layer_set_temperature(WeatherLayer* wl, int16_t t)
{
  WeatherData* wd = layer_get_data(wl);
  snprintf(wd->temp_str, 10, "%i", t);
  int degree_pos = strlen(wd->temp_str);
  memcpy(&wd->temp_str[degree_pos], "°", 3);
  text_layer_set_text(wd->temp_layer, wd->temp_str);
}

void weather_layer_destroy(WeatherLayer* wl)
{
  WeatherData* wd = layer_get_data(wl);
  if(wd->has_weather_icon)
  {
    layer_remove_from_parent(bitmap_layer_get_layer(wd->icon_layer));
    bitmap_layer_destroy(wd->icon_layer);
    gbitmap_destroy(wd->icon);
  }
  layer_destroy(text_layer_get_layer(wd->temp_layer));
}
