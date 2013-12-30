#include "time_layer.h"

/* Called by the graphics layers when the time layer needs to be updated.
*/
void time_layer_update_proc(TimeLayer *tl, GContext* ctx)
{
  TimeData * td = layer_get_data(tl);
  if(td->background_color != GColorClear)
  {
    graphics_context_set_fill_color(ctx, td->background_color);
    graphics_fill_rect(ctx, layer_get_bounds(tl), 0, GCornerNone);
  }
  graphics_context_set_text_color(ctx, td->text_color);

  if(td->hour_text && td->minute_text)
  {
    GSize hour_sz =
      graphics_text_layout_get_content_size(td->hour_text,
                                            td->hour_font,
                                            layer_get_bounds(tl),
                                            td->overflow_mode,
                                            GTextAlignmentLeft);
    GSize minute_sz =
      graphics_text_layout_get_content_size(td->minute_text,
                                            td->minute_font,
                                            layer_get_bounds(tl),
                                            td->overflow_mode,
                                            GTextAlignmentLeft);
    int width = minute_sz.w + hour_sz.w;
    int half = layer_get_bounds(tl).size.w / 2;
    GRect hour_bounds = layer_get_bounds(tl);
    GRect minute_bounds = layer_get_bounds(tl);

    hour_bounds.size.w = half - (width / 2) + hour_sz.w;
    minute_bounds.origin.x = hour_bounds.size.w + 1;
    minute_bounds.size.w = minute_sz.w;

    graphics_draw_text(ctx,
                       td->hour_text,
                       td->hour_font,
                       hour_bounds,
                       td->overflow_mode,
                       GTextAlignmentRight,
                       td->layout_cache);
    graphics_draw_text(ctx,
                       td->minute_text,
                       td->minute_font,
                       minute_bounds,
                       td->overflow_mode,
                       GTextAlignmentLeft,
                       td->layout_cache);
  }
}


/* Set the hour and minute text and mark the layer dirty. NOTE that the
* two strings must be static because we're only storing a pointer to them,
* not making a copy.
*/
void time_layer_set_text(TimeLayer *tl, char *hour_text, char *minute_text)
{
  TimeData * td = layer_get_data(tl);
  td->hour_text = hour_text;
  td->minute_text = minute_text;

  layer_mark_dirty(tl);
}


/* Set the time fonts. Hour and minute fonts can be different.
*/
void time_layer_set_fonts(TimeLayer *tl, GFont hour_font, GFont minute_font)
{
  TimeData * td = layer_get_data(tl);
  td->hour_font = hour_font;
  td->minute_font = minute_font;

  if (td->hour_text && td->minute_text)
  {
    layer_mark_dirty(tl);
  }
}


/* Set the text color of the time layer.
*/
void time_layer_set_text_color(TimeLayer *tl, GColor color)
{
  TimeData * td = layer_get_data(tl);
  td->text_color = color;

  if (td->hour_text && td->minute_text)
  {
    layer_mark_dirty(tl);
  }
}


/* Set the background color of the time layer.
*/
void time_layer_set_background_color(TimeLayer *tl, GColor color)
{
  TimeData * td = layer_get_data(tl);
  td->background_color = color;

  if(td->hour_text && td->minute_text)
  {
    layer_mark_dirty(tl);
  }
}


/* Initialize the time layer with default colors and fonts.
*/
TimeLayer * time_layer_create(GRect frame)
{
  // TODO: free this in window unload?
  TimeLayer * tl = layer_create_with_data(frame, sizeof(TimeData));
  TimeData * td = layer_get_data(tl);
  layer_set_update_proc(tl, time_layer_update_proc);
  td->text_color = GColorWhite;
  td->background_color = GColorClear;
  td->overflow_mode = GTextOverflowModeWordWrap;
  
  td->hour_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  td->minute_font = td->hour_font;
  return tl;
}

void time_layer_destroy(TimeLayer * tl)
{
  // TODO: do we need to clean anything else?
  /* TimeData * td = layer_get_data(tl); */
  layer_destroy(tl);
}
