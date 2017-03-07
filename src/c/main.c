#include <pebble.h>
#include "main.h"
#include "weekday.h"
#include "weathericon.h"


static GFont FontHour, FontMinute,FontDate, FontTemp, FontCond, FontCiti;
static char tempstring[44], condstring[44], citistring[44];
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

//static bool watchconnected, GPSOK;




///////////////////////////
//////Init Configuration///
///////////////////////////
//Init Clay
ClaySettings settings;
// Initialize the default settings
static void prv_default_settings() { 
  settings.BackgroundColor = GColorWhite;
  settings.ForegroundColor  = GColorOxfordBlue;
  settings.DotsColor        = GColorPictonBlue;
  settings.WeatherUnit       = false;
  settings.WeatherCond       =0;
  settings.DisplayLoc        =false;
  settings.DisplayDate    =false;
  settings.DisplayLoc    =false;
  settings.BTOn=true;
  settings.GPSOn=true;
 }
///////////////////////////
//////End Configuration///
///////////////////////////

///BT Connection

static void bluetooth_callback(bool connected) {
   settings.BTOn=connected;
 }

static void onreconnection(bool before, bool now){
  if (!before && now){
      // Begin dictionary
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Update info on reconnect at %d",s_minutes);
      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      if (!iter) {
        // Error creating outbound message
        return;
      }    
      int value = 1;
      dict_write_int(iter, 1, &value, sizeof(int), true);
      dict_write_end(iter);
      app_message_outbox_send();    
  }  
}



static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Colors
  graphics_context_set_fill_color(ctx, settings.DotsColor);
  graphics_context_set_stroke_color(ctx, settings.DotsColor);
  graphics_context_set_text_color(ctx, settings.ForegroundColor); 
  // Local language
  const char* sys_locale = i18n_get_system_locale();
    
  // Adjust geometry variables for dots
  GRect bounds = layer_get_bounds(layer);
  GRect frame = grect_inset(bounds, GEdgeInsets(7));
  int minute_angle=get_angle_for_minutes(s_minutes);
  // Create dots 
  for(int i = 0; i < 12; i++) {
    int dot_angle = get_angle_dot(i);
    GPoint pos = gpoint_from_polar(frame, 
                                   GOvalScaleModeFitCircle, 
                                   DEG_TO_TRIGANGLE(dot_angle)
                                  );
    // Leave the current minute without dot
    if (i != minute_angle/30){
      graphics_fill_circle(ctx, pos, 4);
    }
  }
          
  // Create minute display
  GRect inner=grect_inset(frame, GEdgeInsets(12));
  char minutenow[4];
  snprintf(minutenow, sizeof(minutenow), "%02d", s_minutes);
  GRect minrect = grect_centered_from_polar(inner, 
                                            GOvalScaleModeFitCircle, 
                                            DEG_TO_TRIGANGLE(minute_angle),
                                            GSize(36,28)
                                           );
  graphics_draw_text(ctx, minutenow, FontMinute,minrect, 
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL
                    );

 
  // Create hour display
  GRect hourect =grect_centered_from_polar(GRect(bounds.size.h/2,bounds.size.w/2,0,0),
                                           GOvalScaleModeFitCircle,
                                           0, 
                                           GSize(50,42)
                                          );
  char hournow[4];
  snprintf(hournow, sizeof(hournow), "%02d", s_hours);
  graphics_draw_text(ctx, hournow,
                     FontHour,hourect,
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL
                    );

  // Create date display
  if (settings.DisplayDate){
    GRect daterect=GRect(hourect.origin.x+hourect.size.w+1,
                   hourect.origin.y+8,
                   inner.size.w/2-hourect.size.w/2-minrect.size.w/2+2,
                   hourect.size.h
                        );
    char datenow[44];
    fetchwday(s_weekday, sys_locale, datenow);
    char convertday[4];
    snprintf(convertday, sizeof(convertday), " %02d", s_day);
    // Concatenate date
    strcat(datenow, convertday);
    graphics_draw_text(ctx, datenow, 
                       FontDate,daterect,
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL
                      );
  }
  
  //For weather and loc check wheter the app is connected
  //If it was disconnected fetch new values
  onreconnection(settings.BTOn, connection_service_peek_pebble_app_connection());

  // Update connection toggle
  bluetooth_callback(connection_service_peek_pebble_app_connection()); 
  
  //If Bluetooth off and some info was requested display warning
  if (!settings.BTOn){
    if (settings.DisplayLoc || settings.DisplayTemp){
      GRect warnrect=GRect(hourect.origin.x-17, 
                          hourect.origin.y-20,
                          hourect.size.w+34, 
                          25);
      graphics_draw_text(ctx, "BT off",
                         FontCiti, 
                         warnrect,
                         GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL
                        );
    }    
  }
  //If connected but GPS off then display warning
  else {
    if (!settings.GPSOn){
      if (settings.DisplayLoc || settings.DisplayTemp){
        GRect warnrect=GRect(hourect.origin.x-17, 
                             hourect.origin.y-20,
                             hourect.size.w+34, 
                             25);
        graphics_draw_text(ctx, "GPS fault", 
                           FontCiti, 
                           warnrect,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL
                          );
      }    
    }
    // If BT on and GPS on display according with preferences
    else{
      //If weather active create text
      if (settings.DisplayTemp){
        // Create temp display
        GRect temprect=GRect(hourect.origin.x-10,
                             hourect.origin.y+hourect.size.h+1,
                             hourect.size.w/2+9,
                             (inner.size.h/2-hourect.size.h/2)/2);
        graphics_draw_text(ctx, tempstring, 
                           FontTemp,temprect,
                           GTextOverflowModeWordWrap, GTextAlignmentRight, NULL
                          );
        // Create condition display
        GRect condrect=GRect(hourect.origin.x+hourect.size.w/2+1,
                             hourect.origin.y+hourect.size.h+1,
                             hourect.size.w/2-1,
                             (inner.size.h/2-hourect.size.h/2)/2);
        graphics_draw_text(ctx, condstring, 
                           FontCond,condrect,
                           GTextOverflowModeWordWrap, GTextAlignmentRight, NULL
                          );
      }
      //If location was selected display
      if (settings.DisplayLoc){
        GRect locrect=GRect(hourect.origin.x-17, 
                            hourect.origin.y-20,
                            hourect.size.w+34, 
                            25);
        graphics_draw_text(ctx, citistring, 
                           FontCiti, locrect,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL
                          );
      }      
    }    
  }
  //End update layer
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
  
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "At init temp is %s Cond is %s and City is %s", tempstring,condstring,citistring);
  

  
  // Weather Cond
  Tuple *wcond_t=dict_find(iter, MESSAGE_KEY_WeatherCond );

  if (wcond_t){ 
    get_conditions_yahoo((int)wcond_t->value->int32, condstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Now cond is %s",condstring);
  }
  
   // Weather Temp
  Tuple *wtemp_t=dict_find(iter, MESSAGE_KEY_WeatherTemp);
 if (wtemp_t){ 
    snprintf(tempstring, sizeof(tempstring), "%s", wtemp_t->value->cstring);
      APP_LOG(APP_LOG_LEVEL_DEBUG,"Now temp is %s",tempstring);
  }
  
  // Location
  Tuple *neigh_t=dict_find(iter, MESSAGE_KEY_NameLocation);
  Tuple *citi_t=dict_find(iter,MESSAGE_KEY_NameCity);
  
  if (neigh_t){
    snprintf(citistring,sizeof(citistring),"%s",neigh_t->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Citistring is neighborhood %s", citistring);
  }
  
  else if (citi_t){
    snprintf(citistring,sizeof(citistring),"%s",citi_t->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Citistring is city %s", citistring);
  }
  int lens=strlen(citistring);
  // Evaluate GPS status                                                                                                                      
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Len citisting is %d", lens);
  if (lens==0){
    APP_LOG(APP_LOG_LEVEL_DEBUG,"GPS disconnected at %d",s_minutes);
    settings.GPSOn=false;
  }
  else {
    settings.GPSOn=true;
    APP_LOG(APP_LOG_LEVEL_DEBUG,"GPS active at %d",s_minutes);
  };
    
  
  // Get display handlers
  Tuple *disdate_t=dict_find(iter,MESSAGE_KEY_DisplayDate);
  if (disdate_t){
    if (disdate_t->value->int32==0){
      settings.DisplayDate=false;
    }
    else settings.DisplayDate=true;
  }
  
  Tuple *distemp_t=dict_find(iter,MESSAGE_KEY_DisplayTemp);
  if (distemp_t){
    if (distemp_t->value->int32==0){
      settings.DisplayTemp=false;
    }
    else settings.DisplayTemp=true;
  }
  
  Tuple *disloc_t=dict_find(iter,MESSAGE_KEY_DisplayLoc);
  if (disloc_t){
    if (disloc_t->value->int32==0){
      settings.DisplayLoc=false;
    }
    else settings.DisplayLoc=true;
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
  
  // Get weather update every 30 minutes if needed
  if(minutes % 30 == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Tick at %d",minutes);
    if (settings.DisplayTemp || settings.DisplayLoc){   
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Update weather at %d", minutes);
      // Begin dictionary
      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      if (!iter) {
        // Error creating outbound message
        return;
      }    
      int value = 1;
      dict_write_int(iter, 1, &value, sizeof(int), true);
      dict_write_end(iter);
      app_message_outbox_send();
    }  
	}
}

static void tick_handler(struct tm *time_now, TimeUnits changed) {
  main_window_update(time_now->tm_hour, time_now->tm_min, time_now->tm_wday, time_now->tm_mday);
}

static void init() {
  prv_load_settings();
   // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 256);
  
  // Load Fonts
  FontHour  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GBOLD_34));
  FontMinute=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GBOLD_22));
  FontDate  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GLIGHT_12));
  FontTemp  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GLIGHT_14));
  FontCond  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WICON_22));
  FontCiti  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GLIGHT_10));
  
  main_window_push();
  
  // Register with Event Services
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler); 
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
}

static void deinit() {
  tick_timer_service_unsubscribe();
 	app_message_deregister_callbacks();    //Destroy the callbacks for clean up
}

int main() {
  init();
  app_event_loop();
  deinit();
}
