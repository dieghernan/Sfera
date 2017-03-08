// Clay Config: see https://github.com/pebble/clay


module.exports = [
  {
    "type": "heading",
    "defaultValue": "Settings"
  },

  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "General settings"
      },
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "0xFFFFFF",
        "label": "Background"
      },
      {
        "type": "color",
        "messageKey": "ForegroundColor",
        "defaultValue": "0x000055",
        "label": "Text"
      },
      {
        "type": "toggle",
        "messageKey": "DisplayDate",
        "label": "Date",
        "defaultValue": false      
      }
    ]
  },
   {
    "type": "section",
    "items": [ 
      {
        "type": "heading",
        "defaultValue": "Grpahics settings"
      },
       {
        "type": "toggle",
        "messageKey": "DisplayDots",
        "label": "Display Dots",
        "defaultValue": false      
      },
      {
        "type": "color",
        "messageKey": "DotsColor",
        "defaultValue": "0x55AAFF",
        "label": "Dots Color"
      },  
       {
        "type": "toggle",
        "messageKey": "DisplayBattery",
        "label": "Display Battery Level",
        "defaultValue": false      
      },
      {
        "type": "color",
        "messageKey": "BatteryColor",
        "defaultValue": "0xFF0000",
        "label": "Battery Bar Color"
      }    
    ]
 },

  {
    "type": "section",
    "items": [ 
      {
        "type": "heading",
        "defaultValue": "Connection settings"
      },
      
      {
        "type": "toggle",
        "messageKey": "DisplayTemp",
        "label": "Weather",
        "defaultValue": false      
      },
        {
        "type": "toggle",
        "messageKey": "WeatherUnit",
        "label": "Temperature in Fahrenheit",
        "defaultValue": false,
        "description": "Applicable if Weather toggle is active"
      },
      {
        "type": "toggle",
        "messageKey": "DisplayLoc",
        "label": "Location",
        "defaultValue": false,
        "description": "When available, neighborhood is displayed instead of city"
      },
    ]
 },
 //Yahoo disclaimer
  {
      "type": "heading",
      "defaultValue": "Weather and Location info provided by <a href=https://www.yahoo.com/?ilc=401>Yahoo.com </a>",
      "size":6
  },
      
 {
    "type": "submit",
    "defaultValue":"Ok"
  },
  {
      "type": "heading",
      "defaultValue": "Made in Madrid, Spain",
      "size":6
  }    
];