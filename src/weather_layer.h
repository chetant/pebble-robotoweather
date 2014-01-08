#ifndef WEATHER_LAYER_H
#define WEATHER_LAYER_H

typedef struct {
	/* Layer layer; */
	BitmapLayer * icon_layer;
        GBitmap * icon;
	TextLayer * temp_layer;
	bool has_weather_icon;
	char temp_str[10];
} WeatherData;

typedef Layer WeatherLayer;

typedef enum {
	WEATHER_ICON_CLEAR_DAY = 0,
	WEATHER_ICON_CLEAR_NIGHT,
	WEATHER_ICON_RAIN,
	WEATHER_ICON_SNOW,
	WEATHER_ICON_SLEET,
	WEATHER_ICON_WIND,
	WEATHER_ICON_FOG,
	WEATHER_ICON_CLOUDY,
	WEATHER_ICON_PARTLY_CLOUDY_DAY,
	WEATHER_ICON_PARTLY_CLOUDY_NIGHT,
	WEATHER_ID_ICON_THUNDER,
	WEATHER_ID_ICON_RAIN_SNOW,
	WEATHER_ID_ICON_SNOW_SLEET,
	WEATHER_ID_ICON_COLD,
	WEATHER_ID_ICON_HOT,	
	WEATHER_ICON_NO_WEATHER,
	WEATHER_ICON_COUNT
} WeatherIcon;

WeatherLayer* weather_layer_create(GPoint pos);
void weather_layer_destroy(WeatherLayer* weather_layer);
void weather_layer_set_icon(WeatherLayer* weather_layer, WeatherIcon icon);
void weather_layer_set_temperature(WeatherLayer* weather_layer, int16_t temperature);

#endif
