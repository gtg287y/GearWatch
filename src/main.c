#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer,*s_time_layer_shadow, *s_date_layer, *s_date_num_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_date_num_font;
static bool tock = true;
  
//only works if source bitmap is in 8 bit mode (can force if bitmap has more than 16 colors)
//also, make sure for loop is the size of your bitmap
static void turnBitmapColor(uint8_t *data, int bg_color, int color){
  for (int i=0; i < 144*168; i++) {
    if (data[i] != bg_color) {
      data[i] = color;
    }
  }
}


static void update_date(){
  static char date_buffer[16];
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  strftime(date_buffer, sizeof(date_buffer), "%A", tick_time);
  // Show the date
  text_layer_set_text(s_date_layer, date_buffer);
  
  static char date_num_buffer[16];
  strftime(date_num_buffer, sizeof(date_num_buffer), "%D", tick_time);
  // Show the date
  text_layer_set_text(s_date_num_layer, date_num_buffer);
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(s_time_layer_shadow, s_buffer);
}

static void pulse_gear(){
  tock = !tock;
  uint8_t *data =  gbitmap_get_data(s_background_bitmap);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, " Format is: %d", gbitmap_get_format(s_background_bitmap));

  int color = GColorRedARGB8;
  if(tock){
    color = GColorDarkCandyAppleRedARGB8;
  }
  turnBitmapColor(data, GColorBulgarianRoseARGB8, color);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();
  pulse_gear();
}
static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_GEAR_BITMAP);

  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(10, 10), bounds.size.w, bounds.size.h));
  s_time_layer_shadow = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(12, 12), bounds.size.w, bounds.size.h));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_background_color(s_time_layer_shadow, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer_shadow, GColorBulgarianRose);
  text_layer_set_text(s_time_layer, "00:00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_layer_shadow, "00:00:00");
  text_layer_set_text_alignment(s_time_layer_shadow, GTextAlignmentCenter);
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 80, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  
   // Create date num TextLayer
  s_date_num_layer = text_layer_create(GRect(0, 100, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_date_num_layer, GColorBlack);
  text_layer_set_background_color(s_date_num_layer, GColorClear);
  text_layer_set_text_alignment(s_date_num_layer, GTextAlignmentCenter);

  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DAVE_FONT80));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DAVE_FONT25));
  s_date_num_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DAVE_FONT25));
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_time_layer_shadow, s_time_font);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_font(s_date_num_layer, s_date_num_font);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer_shadow));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Add to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_num_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Unload GFont
  fonts_unload_custom_font(s_time_font);

  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  fonts_unload_custom_font(s_date_font);

  text_layer_destroy(s_date_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  // Use this provider to add button click subscriptions
   APP_LOG(APP_LOG_LEVEL_DEBUG, "click provider set...");
  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}



