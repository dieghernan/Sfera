#include <pebble.h>
#include "main.h"
#include "weekday.h"
#include "weathericon.h"

//Static and initial vars
static GFont FontHour, FontMinute,FontDate, FontTemp, FontCond, FontCiti, FontSymbol;
char tempstring[44], condstring[44], citistring[44];
static Window *s_window;
static Layer *s_canvas;
static int s_hours, s_minutes, s_weekday, s_day,s_battery_level,s_loop, s_countdown;
static int32_t get_angle_dot(int dot) {  
  // Progress through 12 hours, out of 360 degrees
  return (dot * 360) / 12;
}
static int get_angle_for_minutes(int minutes){
  int floor5=minutes/5;
  return (floor5*5 * 360) / 60;  
};
static int get_angle_for_battery(int battery){
  int floor30=(100-battery)*360/3000;
  return (floor30*30);  
};

static int hourtodraw(bool hourformat, int hournow){
  if (hourformat){
    return hournow;    
  }
  else {
    if (hournow==0){
      return 12;      
    }
    else if(hournow<=12){
      return hournow;
    }
    else {
      return hournow-12;
    }
  }
};

static int xdaterect(bool is24, GRect hourect, GRect inner, GRect minrect){
  if (is24){
    return hourect.origin.x+hourect.size.w+1;
  }
  else {
    return hourect.origin.x-(inner.size.w/2-hourect.size.w/2-minrect.size.w/2+3);    
  }  
}


static GColor ColorSelect(bool isactive, bool gpsstate, bool isnight, GColor ColorDay, GColor ColorNight){
  if (isactive && isnight && gpsstate){
    return ColorNight;   
  }
  else {
    return ColorDay;
  }  
}



static GTextAlignment AlignDate(bool is24){
  if (is24){
    return GTextAlignmentLeft;
  }
  else {
    return GTextAlignmentRight;
  }
}

// Callback for js request
void request_watchjs(){
  //Starting loop at 0
  s_loop=0;
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);
  // Send the message!
  app_message_outbox_send(); 
}

//////Init Configuration///
//Init Clay
ClaySettings settings;
// Initialize the default settings
static void prv_default_settings() { 
  settings.BackgroundColor = GColorWhite;
  settings.ForegroundColor  = GColorOxfordBlue;
  settings.DotsColor        = GColorBlueMoon;
  settings.BatteryColor      =GColorPictonBlue;
  
  settings.BackgroundColorNight   = GColorPictonBlue;
  settings.ForegroundColorNight  = GColorOrange;
  settings.DotsColorNight        = GColorOrange;
  settings.BatteryColorNight      =GColorOrange;
  
  settings.WeatherUnit       = false;
  settings.WeatherCond       =0;
  settings.UpSlider          =30;
  settings.HourSunrise    =600;
  settings.HourSunset    =1700;
  settings.DisplayLoc        =false;
  settings.DisplayDate    =false;
  settings.DisplayLoc    =false;
  settings.DisplayDots   =false;
  settings.DisplayBattery =false;
  settings.BTOn=true;
  settings.GPSOn=false;
  settings.NightTheme=true;
  settings.IsNightNow=false;
  
 }
//////End Configuration///
///////////////////////////

///BT Connection
static void bluetooth_callback(bool connected) {
   settings.BTOn=connected;
 }
static void onreconnection(bool before, bool now){
  if (!before && now){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "BT reconnected, requesting weather at %d", s_minutes);
  
     request_watchjs();  
  }  
}

//Update main layer
static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Colors
  graphics_context_set_text_color(ctx, ColorSelect(settings.NightTheme, settings.GPSOn, settings.IsNightNow, settings.ForegroundColor, settings.ForegroundColorNight)); 
  // Local language
  const char* sys_locale = i18n_get_system_locale();
 
    
  // Adjust geometry variables for dots
  GRect bounds = layer_get_bounds(layer);
  GRect frame = grect_inset(bounds, GEdgeInsets(7));
  
  
 //Prepare battery display
  if (settings.DisplayBattery){ 
    s_battery_level=battery_state_service_peek() .charge_percent;
    
    graphics_context_set_stroke_width(ctx, 4);
    graphics_context_set_stroke_color(ctx, ColorSelect(settings.NightTheme, settings.GPSOn, settings.IsNightNow, settings.BatteryColor, settings.BatteryColorNight)); 
    //Battery below 20 is set red
    if (s_battery_level<=20){
      graphics_context_set_stroke_color(ctx, GColorRed);
    }
    graphics_draw_arc(ctx, 
                      frame, 
                      GOvalScaleModeFitCircle, 
                      DEG_TO_TRIGANGLE(get_angle_for_battery(s_battery_level)), 
                      DEG_TO_TRIGANGLE(360));
  }
  int minute_angle=get_angle_for_minutes(s_minutes);
  // Create dots only if active
  if (settings.DisplayDots){
      graphics_context_set_fill_color(ctx, ColorSelect(settings.NightTheme, settings.GPSOn, settings.IsNightNow, settings.DotsColor, settings.DotsColorNight)); 
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
  }
   
 
  // Create minute display
  //Offset for minutes gt 34
  int offset;
  if (s_minutes>34 && s_minutes < 60){
    offset=6;
  }
  else {
    offset=0;
       }
  //Offset for Loc Layer
  int offsetloc;
  if (s_minutes>49 && s_minutes < 55){
    offsetloc=offset;
  }
  else {
    offsetloc=0;
       }
  GRect inner=grect_inset(frame, GEdgeInsets(12));
  char minutenow[4];
  snprintf(minutenow, sizeof(minutenow), "%02d", s_minutes);
  GRect minrect_init = grect_centered_from_polar(inner, 
                                            GOvalScaleModeFitCircle, 
                                            DEG_TO_TRIGANGLE(minute_angle),
                                            GSize(36,28)
                                           );
  GRect minrect=GRect(minrect_init.origin.x+offset,
                      minrect_init.origin.y, 
                      minrect_init.size.w, 
                      minrect_init.size.h);
  
  graphics_draw_text(ctx, minutenow, FontMinute,minrect, 
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL
                    );

  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 1);

  // Create hour display
  GRect hourect =grect_centered_from_polar(GRect(bounds.size.h/2,bounds.size.w/2,0,0),
                                           GOvalScaleModeFitCircle,
                                           0, 
                                           GSize(50,42)
                                          );
  char hournow[4];
  int hourtorect=hourtodraw(clock_is_24h_style(), s_hours);
  snprintf(hournow, sizeof(hournow), "%02d", hourtorect);
  graphics_draw_text(ctx, hournow,
                     FontHour,hourect,
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL
                    );

  //Put AM or PM format
  if (!clock_is_24h_style()){
    char ampm[2];
    if (s_hours<12){
     strcpy(ampm, "am");
    }
    else {
      strcpy(ampm, "pm");
    }
    //Create Rect and display
     GRect ampmrect=GRect(hourect.origin.x+hourect.size.w+1,
                   hourect.origin.y+8,
                   inner.size.w/2-hourect.size.w/2-minrect.size.w/2+2,
                   hourect.size.h
                        );
    graphics_draw_text(ctx, ampm, 
                       FontDate,ampmrect,
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL
                      );
  }
  
  // Create date display
  if (settings.DisplayDate){
    char datenow[44];
    fetchwday(s_weekday, sys_locale, datenow);
    char convertday[4];
    snprintf(convertday, sizeof(convertday), " %02d", s_day);
    // Concatenate date
    strcat(datenow, convertday);
    int newx=xdaterect(clock_is_24h_style(), hourect, inner, minrect);
    GRect daterect=GRect(newx,
                         hourect.origin.y+8,
                         inner.size.w/2-hourect.size.w/2-minrect.size.w/2+2,
                         hourect.size.h
                        );
    graphics_draw_text(ctx, datenow, 
                       FontDate,daterect,
                       GTextOverflowModeWordWrap, AlignDate(clock_is_24h_style()), NULL
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
      graphics_draw_text(ctx, "a",
                         FontSymbol, 
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
        graphics_draw_text(ctx, "b", 
                           FontSymbol, 
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
        GRect locrect=GRect(hourect.origin.x-17+offsetloc/2, 
                            hourect.origin.y-20-offsetloc/2,
                            hourect.size.w+34-offsetloc, 
                            25+offsetloc/2);
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
 s_loop=s_loop+1;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loop is %d", s_loop);
  if (s_loop==1){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Cleaning vars in loop %d", s_loop);
      //Clean vars  
        strcpy(tempstring, "");
        strcpy(condstring, "");
        strcpy(citistring, "");
  }
    
  // Background Color
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  if (bg_color_t) {
    settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);
	}

  Tuple *nbg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColorNight);
  if (nbg_color_t) {
    settings.BackgroundColorNight = GColorFromHEX(nbg_color_t->value->int32);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Cached BKCOLNIGHT");
	}
  
  // Foreground Color
 	Tuple *fg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColor);
  if (fg_color_t) {
    settings.ForegroundColor = GColorFromHEX(fg_color_t->value->int32);
  }  
  Tuple *nfg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColorNight);
  if (nfg_color_t) {
     settings.ForegroundColorNight = GColorFromHEX(nfg_color_t->value->int32);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Cached FORCOLNIGHT");
	}
  
  // Dots Color
 	Tuple *dt_color_t = dict_find(iter, MESSAGE_KEY_DotsColor);
  if (dt_color_t) {
    settings.DotsColor = GColorFromHEX(dt_color_t->value->int32);
  }  
  
  Tuple *ndt_color_t = dict_find(iter, MESSAGE_KEY_DotsColorNight);
  if (ndt_color_t) {
    settings.DotsColorNight = GColorFromHEX(ndt_color_t->value->int32);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Cached dotCOLNIGHT");
	}
  
  
  //Battery Color
   	Tuple *batt_color_t = dict_find(iter, MESSAGE_KEY_BatteryColor);
  if (batt_color_t) {
    settings.BatteryColor = GColorFromHEX(batt_color_t->value->int32);
  }  
    Tuple *nbatt_color_t = dict_find(iter, MESSAGE_KEY_BatteryColorNight);
  if (nbatt_color_t) {
    settings.BatteryColorNight = GColorFromHEX(ndt_color_t->value->int32);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Cached batCOLNIGHT");
	}
  
  
  //Control of data from http
  APP_LOG(APP_LOG_LEVEL_DEBUG, "At beggining loop %d temp is %s Cond is %s and City is %s",s_loop, tempstring,condstring,citistring);
    // Weather Cond
  Tuple *wcond_t=dict_find(iter, MESSAGE_KEY_WeatherCond );

  if (wcond_t){ 
    get_conditions_yahoo((int)wcond_t->value->int32, condstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Now cond is %s",condstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Updated Cond at loop %d",s_loop);
  }
  
   // Weather Temp
  Tuple *wtemp_t=dict_find(iter, MESSAGE_KEY_WeatherTemp);
 if (wtemp_t){ 
    snprintf(tempstring, sizeof(tempstring), "%s", wtemp_t->value->cstring);
      APP_LOG(APP_LOG_LEVEL_DEBUG,"Now temp is %s",tempstring);
  }
  
    //Hour Sunrise and Sunset
  Tuple *sunrise_t=dict_find(iter, MESSAGE_KEY_HourSunrise);
  if (sunrise_t){
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Now SUNRISE is %d",(int)sunrise_t->value->int32);
    settings.HourSunrise=(int)sunrise_t->value->int32;
  }
  Tuple *sunset_t=dict_find(iter, MESSAGE_KEY_HourSunset);
  if (sunset_t){
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Now sunset is %d",(int)sunset_t->value->int32);  
    settings.HourSunset=(int)sunset_t->value->int32;
  }
  
  // Location
  Tuple *neigh_t=dict_find(iter, MESSAGE_KEY_NameLocation);
  Tuple *citi_t=dict_find(iter,MESSAGE_KEY_NameCity);
  
  if (neigh_t){
    snprintf(citistring,sizeof(citistring),"%s",neigh_t->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Citistring is neighborhood %s", citistring);
     APP_LOG(APP_LOG_LEVEL_DEBUG,"Updated Neigh at loop %d",s_loop);
  }
  
  else if (citi_t){
    snprintf(citistring,sizeof(citistring),"%s",citi_t->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Citistring is city %s", citistring);
    APP_LOG(APP_LOG_LEVEL_DEBUG,"Updated City at loop %d",s_loop);
  }
  
  //Control of data gathered for http
  APP_LOG(APP_LOG_LEVEL_DEBUG, "After loop %d temp is %s Cond is %s and City is %s", s_loop,tempstring,condstring,citistring);
  
  if (strcmp(tempstring, "") !=0 && strcmp(condstring, "") !=0 && strcmp(citistring, "")){
    APP_LOG(APP_LOG_LEVEL_DEBUG,"GPS fully working at loop %d",s_loop);
    settings.GPSOn=true;
  }  
  else {
    APP_LOG(APP_LOG_LEVEL_DEBUG,"MIssing info at loop %d, GPS false",s_loop);
    settings.GPSOn=false;
  }
  //End data gathered
  
  // Get display handlers
  Tuple *frequpdate=dict_find(iter, MESSAGE_KEY_UpSlider);
  if (frequpdate){
    settings.UpSlider=(int)frequpdate->value->int32;
  }
  
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
  
  Tuple *disdot_t=dict_find(iter,MESSAGE_KEY_DisplayDots);
  if (disdot_t){
    if (disdot_t->value->int32==0){
      settings.DisplayDots=false;
    }
    else settings.DisplayDots=true;
  }
  
  Tuple *disbatt_t=dict_find(iter,MESSAGE_KEY_DisplayBattery);
  if (disbatt_t){
    if (disbatt_t->value->int32==0){
      settings.DisplayBattery=false;
    }
    else settings.DisplayBattery=true;
  }
  
  Tuple *disntheme_t=dict_find(iter,MESSAGE_KEY_NightTheme);
  if (disntheme_t){
    if (disntheme_t->value->int32==0){
      settings.NightTheme=false;
      APP_LOG(APP_LOG_LEVEL_DEBUG,"NTHeme off");
    }
    else settings.NightTheme=true;
  }
  
  //Update colors
	layer_mark_dirty(s_canvas);
  window_set_background_color(s_window,ColorSelect(settings.NightTheme, settings.IsNightNow, settings.GPSOn,settings.BackgroundColor, settings.BackgroundColorNight));
       
  // Save the new settings to persistent storage
  prv_save_settings();
 
}
//Load main layer
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
  fonts_unload_custom_font(FontTemp);
  fonts_unload_custom_font(FontHour);
  fonts_unload_custom_font(FontMinute);
  fonts_unload_custom_font(FontCiti);
  fonts_unload_custom_font(FontCond);
  fonts_unload_custom_font(FontSymbol);
  fonts_unload_custom_font(FontDate);                           
}

void main_window_push() {
  s_window = window_create();
  window_set_background_color(s_window, ColorSelect(settings.NightTheme, settings.GPSOn, settings.IsNightNow, settings.BackgroundColor, settings.BackgroundColorNight)); 
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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Tick at %d", time_now->tm_min);
    s_loop=0;
  
if (s_countdown==0){
   //Reset
   s_countdown=settings.UpSlider;
 }
else {
  s_countdown=s_countdown-1;  
}
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Countdown to update %d", s_countdown);
  
    // Evaluate if is day or night
  if (settings.GPSOn && settings.NightTheme){  
    int nowthehouris=s_hours*100+s_minutes;
    if (settings.HourSunrise<=nowthehouris && nowthehouris<=settings.HourSunset){
      settings.IsNightNow=false;  
    }
    else {
       settings.IsNightNow=true;        
    }
    
    //Extra catch around 1159 to gather information of today
     if (nowthehouris==1159 && s_countdown>5) {s_countdown=1;};
    // Change Color of background	
    layer_mark_dirty(s_canvas);
    window_set_background_color(s_window,ColorSelect(settings.NightTheme, settings.IsNightNow, settings.GPSOn,settings.BackgroundColor, settings.BackgroundColorNight));
    }
  
  
   // Get weather update every 30 minutes and extra request 5 minutes earlier
  if(s_countdown== 0 || s_countdown==5) {
    if (settings.DisplayTemp || settings.DisplayLoc){
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Update weather at %d", time_now->tm_min);
      request_watchjs();    
    }  
	}
  
  //If GPS was off request weather every 15 minutes
  
  if (!settings.GPSOn){
    if (settings.DisplayTemp || settings.DisplayLoc){
      if (settings.UpSlider>15){
        if(s_countdown % 15 == 0){
          APP_LOG(APP_LOG_LEVEL_DEBUG, "Attempt to request GPS on %d", time_now->tm_min);
          request_watchjs();    
        }
      }      
    }
  }  
}

static void init() {
  prv_load_settings();
   // Listen for AppMessages
  //Starting loop at 0
  s_loop=0;
  s_countdown=settings.UpSlider;
  //Clean vars
  strcpy(tempstring, "");
  strcpy(condstring, "");
  strcpy(citistring, "");
  //Register and open
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 256);
  
  // Load Fonts
  FontHour  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GBOLD_34));
  FontMinute=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GBOLD_22));
  FontDate  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GLIGHT_12));
  FontTemp  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GLIGHT_14));
  FontCond  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WICON_22));
  FontCiti  =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GLIGHT_10));
  FontSymbol =fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SYMBOL_16));
  
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
