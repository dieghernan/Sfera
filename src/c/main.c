#include <pebble.h>
#include "main.h"


static GFont FontHour, FontMinute,FontDate, FontTemp, FontCond;
static Window *s_window;
static Layer *s_canvas;
static int s_hours, s_minutes, s_weekday, s_day;
static int32_t get_angle_dot(int dot) {  
  // Progress through 12 hours, out of 360 degrees
  return (dot * 360) / 12;
}

static int get_angle_for_minutes(int minutes){
  int floor5=minutes/5;
  return (floor5*5 * 360) / 60;  
};


///////////////////////////
//////Init Configuration///
///////////////////////////
//Init Clay
ClaySettings settings;
// Initialize the default settings
static void prv_default_settings() { 
  settings.BackgroundColor = GColorBlack;
  settings.ForegroundColor  = GColorWhite;
  settings.DotsColor        = GColorRed;
 }
///////////////////////////
//////End Configuration///
///////////////////////////




static void layer_update_proc(Layer *layer, GContext *ctx) {

  // Colors
  graphics_context_set_fill_color(ctx, settings.DotsColor);
  graphics_context_set_stroke_color(ctx, settings.DotsColor);
  graphics_context_set_text_color(ctx, settings.ForegroundColor);
  // Adjust geometry variables for inner ring
  GRect bounds = layer_get_bounds(layer);
  GRect frame = grect_inset(bounds, GEdgeInsets(7));
 
  int minute_angle=get_angle_for_minutes(s_minutes);
   // Create dots 
  for(int i = 0; i < 12; i++) {
    int dot_angle = get_angle_dot(i);
    GPoint pos = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(dot_angle));
    // Leave the current minute without dot
    if (i != minute_angle/30){ 
      graphics_fill_circle(ctx, pos, 2);
   }
  }
          
  // Create minute display
  GRect inner=grect_inset(frame, GEdgeInsets(12));
  char minutenow[4];
  snprintf(minutenow, sizeof(minutenow), "%02d", s_minutes);
  GRect minrect = grect_centered_from_polar(inner, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minute_angle),GSize(36,28));
  graphics_draw_text(ctx, minutenow, 
      FontMinute,
      minrect, 
      GTextOverflowModeFill, GTextAlignmentCenter, NULL
    );

 
  // Create hour display
  GRect hourect =grect_centered_from_polar(GRect(bounds.size.h/2,bounds.size.w/2,0,0), GOvalScaleModeFitCircle, 0, GSize(50,42));
  char hournow[4];
  snprintf(hournow, sizeof(hournow), "%02d", s_hours);
  graphics_draw_text(ctx, hournow, 
      FontHour,
      hourect,
      GTextOverflowModeFill, GTextAlignmentCenter, NULL
  );

  // Create date display
  GRect daterect=GRect(hourect.origin.x+hourect.size.w+1,
                   hourect.origin.y,
                   inner.size.w/2-hourect.size.w/2-minrect.size.w/2-2,
                   hourect.size.h);
  char datenow[4]="Lun";
  char convertday[4];
  snprintf(convertday, sizeof(convertday), " %02d", s_day);
  // Concatenate date
  strcat(datenow, convertday);
    graphics_draw_text(ctx, datenow, 
      FontDate,
      daterect,
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL
  );
  
  // Create temp display
    GRect temprect=GRect(hourect.origin.x,
                   hourect.origin.y+hourect.size.h+1,
                   hourect.size.w/2-1,
                   (inner.size.h/2-hourect.size.h/2-minrect.size.h/2)/2);
  char tempnow[44]="-12°c";
  graphics_draw_text(ctx, tempnow, 
      FontTemp,
      temprect,
      GTextOverflowModeWordWrap, GTextAlignmentRight, NULL
  );
    
  // Create condition display
  GRect condrect=GRect(hourect.origin.x+hourect.size.w/2+2,
                   hourect.origin.y+hourect.size.h+1,
                   hourect.size.w/2-1,
                   (inner.size.h/2-hourect.size.h/2-minrect.size.h/2)/2);
  char condnow[44]="a";
  graphics_draw_text(ctx, condnow, 
      FontCond, 
      condrect,
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL
  );

  // Dev
 
  
  graphics_draw_rect(ctx, hourect);
  graphics_draw_rect(ctx, minrect);
  graphics_draw_rect(ctx, temprect);
  graphics_draw_rect(ctx, condrect);
  
}


/////////////////////////////////////////
////Init: Handle Settings and Weather////
/////////////////////////////////////////
// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}
// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update date
}
// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Background Color
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  if (bg_color_t) {
    settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);
	}

  // Foreground Color
 	Tuple *fg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColor);
  if (fg_color_t) {
    settings.ForegroundColor = GColorFromHEX(fg_color_t->value->int32);
  }  
  
  // Dots Color
 	Tuple *dt_color_t = dict_find(iter, MESSAGE_KEY_DotsColor);
  if (dt_color_t) {
    settings.DotsColor = GColorFromHEX(dt_color_t->value->int32);
  }  
  
  //Update colors
	layer_mark_dirty(s_canvas);
  window_set_background_color(s_window, settings.BackgroundColor);

 
  // Save the new settings to persistent storage
  prv_save_settings();
 
}



static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, layer_update_proc);
  layer_add_child(window_layer, s_canvas);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas);
  window_destroy(s_window);
}

void main_window_push() {
  s_window = window_create();
  window_set_background_color(s_window, settings.BackgroundColor);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

void main_window_update(int hours, int minutes, int weekday, int day) {
  s_hours = hours;
  s_minutes = minutes;
  s_day=day;
  s_weekday=weekday;
  layer_mark_dirty(s_canvas);
}

static void tick_handler(struct tm *time_now, TimeUnits changed) {
  main_window_update(time_now->tm_hour, time_now->tm_min, time_now->tm_wday, time_now->tm_mday);
}

static void init() {
  prv_load_settings();
   // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);
  
  
  // Load Fonts
  FontHour  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GBOLD_34));
  FontMinute=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GBOLD_22));
  FontDate  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GLIGHT_12));
  FontTemp  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GLIGHT_10));
  FontCond  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WICON_22));
  
  main_window_push();

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);  
}

static void deinit() {
  tick_timer_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
