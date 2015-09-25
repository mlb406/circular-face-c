#include <pebble.h>
#include "math.h"

static Window *window;
/*static BitmapLayer *background_layer;
static GBitmap *background_bitmap;*/
static TextLayer *time_layer;
static Layer *circle_layer, *top_analogue, *bottom_analogue, *top_hand, *bottom_hand;

static GPath *top_path;
static GPathInfo TOP_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint[]) { {-1, 0}, {144, 0}, {144, 12}, {-1, 12} }
};

static GPath *bottom_path;
static GPathInfo BOTTOM_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint[]) { {-1, 156}, {144, 156}, {144, 168}, {-1, 168} }
};

static void layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorFromRGB(0, 0, 255));
  #else
    graphics_context_set_fill_color(ctx, GColorWhite);
  #endif

  graphics_fill_circle(ctx, GPoint(72, 84), 64);
  
  gpath_draw_filled(ctx, top_path);
  gpath_draw_filled(ctx, bottom_path);
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  float min_pos1 = t->tm_min * 2.4;
  int min_pos = ceil(min_pos1);
  
  float hour_pos1 = t->tm_hour * 6;
  int hour_pos = ceil(hour_pos1);
  
  #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, GColorFromRGB(255, 0, 0));
  #else
    graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  
  //uint8_t i = 3;
  //graphics_context_set_stroke_width(ctx, i);
  
  GPoint min_p0 = GPoint(min_pos, 0);
  GPoint min_p1 = GPoint(min_pos, 12);
  
  GPoint hour_p0 = GPoint(hour_pos, 156);
  GPoint hour_p1 = GPoint(hour_pos, 168);
  
  graphics_draw_line(ctx, min_p0, min_p1);
  graphics_draw_line(ctx, hour_p0, hour_p1);
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char buffer[] = "00\n00\n00";
  
  if (clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00\n00\n00"), "%H\n%M\n%S", tick_time);
  } else {
    strftime(buffer, sizeof("00\n00\n00"), "%I\n%M\n%S", tick_time);
  }
  text_layer_set_text(time_layer, buffer);
  
  
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load() {
  /*background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(background_layer, background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_layer));
  */
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  circle_layer = layer_create(GRect(0, 0, 144, 168));
  
  layer_set_update_proc(circle_layer, layer_update_proc);

  // Add to Window as child Layer
  layer_add_child(window_get_root_layer(window), circle_layer);
  
   // Create GPath object
  top_path = gpath_create(&TOP_PATH_INFO);

  // Create Layer that the path will be drawn on
  top_analogue = layer_create(bounds);
  layer_set_update_proc(top_analogue, layer_update_proc);
  layer_add_child(window_layer, top_analogue);
  
  //add code for top bars here
  bottom_path = gpath_create(&BOTTOM_PATH_INFO);
  bottom_analogue = layer_create(bounds);
  layer_set_update_proc(bottom_analogue, layer_update_proc);
  layer_add_child(window_layer, bottom_analogue);
  
  time_layer = text_layer_create(GRect(1, 14, 143, 140));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  #ifdef PBL_COLOR
    text_layer_set_text_color(time_layer, GColorFromRGB(255, 0, 0));
  #else
    text_layer_set_text_color(time_layer, GColorBlack);
  #endif
  text_layer_set_text(time_layer, "00\n00\n00");
  text_layer_set_background_color(time_layer, GColorClear);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
  
  top_hand = layer_create(bounds);
  layer_set_update_proc(top_hand, hands_update_proc);
  layer_add_child(window_layer, top_hand);
  
}

static void main_window_unload() {
  /*gbitmap_destroy(background_bitmap);
  
  bitmap_layer_destroy(background_layer);
  */
  text_layer_destroy(time_layer);
  
  layer_destroy(circle_layer);
  
}

static void init(void) {
  window = window_create();
  
  #ifdef PBL_COLOR
    window_set_background_color(window, GColorRed);
  #else 
    window_set_background_color(window, GColorBlack);
  #endif
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  
  window_stack_push(window, true);
  
  update_time();
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
