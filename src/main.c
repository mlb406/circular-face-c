#include <pebble.h>
#include "math.h"

#define KEY_BARS 0
#define KEY_DATE 1
#define KEY_FORMAT 2

static Window *window, *date_window;
static TextLayer *time_layer, *date_layer;
static Layer *circle_layer, *top_analogue, *bottom_analogue, *top_hand;
static int battery_level;
static void accel_tap_handler(AccelAxisType axis, int32_t direction);

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

static void date_window_init() {
  accel_tap_service_unsubscribe();
  date_window = window_create();
  #ifdef PBL_SDK_2 
    window_set_fullscreen(date_window, true);
  #endif
  window_set_background_color(date_window, GColorBlack);
  window_stack_push(date_window, true);
  
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%d/%m/%y", t);
  
  date_layer = text_layer_create(GRect(0, 50, 144, 168));
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_text(date_layer, buffer);
  
  layer_add_child(window_get_root_layer(date_window), text_layer_get_layer(date_layer));
}

static void date_window_deinit() {
  text_layer_destroy(date_layer);
  window_stack_remove(date_window, true);
  window_destroy(date_window);
  accel_tap_service_subscribe(accel_tap_handler);
}

static void app_timer_callback() {
  date_window_deinit();
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  date_window_init();
  app_timer_register(2000, app_timer_callback, NULL);
}

static void battery_callback(BatteryChargeState charge_state) {
  battery_level = charge_state.charge_percent;
  
  if (charge_state.is_charging == true) {
    #ifdef PBL_COLOR
      window_set_background_color(window, GColorFromRGB(0, 255, 0));
    #endif
  } else if (battery_level < 20) {
    #ifdef PBL_COLOR
      window_set_background_color(window, GColorFromRGB(255, 255, 0));
    #endif
  } else {
    #ifdef PBL_COLOR
      window_set_background_color(window, GColorFromRGB(255, 0, 0));
    #endif
  }
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  #ifdef PBL_COLOR
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_fill_color(ctx, GColorFromRGB(0, 0, 255));
  #else
    graphics_context_set_fill_color(ctx, GColorWhite);
  #endif


  graphics_fill_circle(ctx, center, 64);
  
  gpath_draw_filled(ctx, top_path);
  gpath_draw_filled(ctx, bottom_path);
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  //GRect bounds = layer_get_bounds(layer);
  
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

  #ifdef PBL_SDK_3
    uint8_t i = 2;
    graphics_context_set_stroke_width(ctx, i);
  #endif

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
  layer_mark_dirty(window_get_root_layer(window));
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

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);
  
  //char metar_buffer[128];
  
  while (t != NULL) {
    switch (t->key) {
      case KEY_METAR:
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Got KEY_METAR");
        //snprintf(metar_buffer, sizeof(metar_buffer), t->value->cstring, t->value->cstring);
        text_layer_set_text(metar_layer, t->value->cstring);
        persist_write_string(KEY_METAR, t->value->cstring);
        vibes_double_pulse();
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Key not found :(");
    }
    
    t = dict_read_next(iter);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init(void) {
  window = window_create();
  
  #ifdef PBL_COLOR
    window_set_background_color(window, GColorRed);
  #else 
    window_set_background_color(window, GColorBlack);
  #endif
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  battery_state_service_subscribe(battery_callback);
  accel_tap_service_subscribe(accel_tap_handler);
  
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
