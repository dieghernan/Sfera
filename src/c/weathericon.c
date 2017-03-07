#include <pebble.h>
#include "weathericon.h"
#include "string.h"

static const char* const yahoomap[]={ 
"a", //tornado
"b", //tropical storm
"c", //hurricane
"d", //severe thunderstorms
"d", //thunderstorms
"e", //mixed rain and snow
"e", //mixed rain and sleet
"e", //mixed snow and sleet
"f", //freezing drizzle
"g", //drizzle
"f", //freezing rain
"g", //showers
"g", //showers
"h", //snow flurries
"h", //light snow showers
"h", //blowing snow
"h", //snow
"f", //hail
"e", //sleet
"i", //dust
"j", //foggy
"k", //haze
"m", //smoky
"l", //blustery
"l", //windy
"h", //cold
"n", //cloudy
"p", //mostly cloudy (night)
"o", //mostly cloudy (day)
"p", //partly cloudy (night)
"o", //partly cloudy (day)
"q", //clear (night)
"r", //sunny
"s", //fair (night)
"t", //fair (day)
"e", //mixed rain and hail
"u", //hot
"b", //isolated thunderstorms
"b", //scattered thunderstorms
"b", //scattered thunderstorms
"g", //scattered showers
"h", //heavy snow
"h", //scattered snow showers
"h", //heavy snow
"t", //partly cloudy
"b", //thundershowers
"h", //snow showers
"b", //isolated thundershowers
 "", //Null value 
};

void get_conditions_yahoo(int yahoocond, char *conditionnow ){
    strcpy(conditionnow, yahoomap[yahoocond]);
}

