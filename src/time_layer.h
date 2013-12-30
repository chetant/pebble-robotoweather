#include "pebble.h"

/* Custom layer type for displaying time with different fonts for hour
* and minute.
*/
typedef struct _TimeData
{
    const char *hour_text;
    const char *minute_text;
    GFont * hour_font;
    GFont * minute_font;
    GTextLayoutCacheRef layout_cache;
    GColor text_color : 2;
    GColor background_color : 2;
    GTextOverflowMode overflow_mode : 2;
} TimeData;

typedef Layer TimeLayer;

void time_layer_update_proc(TimeLayer *tl, GContext* ctx);
void time_layer_set_text(TimeLayer *tl, char *hour_text, char *minute_text);
void time_layer_set_fonts(TimeLayer *tl, GFont hour_font, GFont minute_font);
void time_layer_set_text_color(TimeLayer *tl, GColor color);
void time_layer_set_background_color(TimeLayer *tl, GColor color);
TimeLayer * time_layer_create(GRect frame);
void time_layer_destroy(TimeLayer * tl);
