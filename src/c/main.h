#pragma once
#include <pebble.h>

#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {
  GColor BackgroundColor;
  GColor ForegroundColor;
  GColor DotsColor;
  bool   WeatherUnit;
  int    WeatherCond;
  char*  WeatherTemp;
  bool DisplayDate;
  bool DisplayTemp;
  bool DisplayLoc;
  bool BTOn;
  bool GPSOn;  
    
} __attribute__((__packed__)) ClaySettings;


