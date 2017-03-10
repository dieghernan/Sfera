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
        "defaultValue": "Graphics settings"
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
        "defaultValue": "0x0055FF",
        "label": "Dots Color"
      },  
       {
        "type": "toggle",
        "messageKey": "DisplayBattery",
        "label": "Display Battery Level",
        "defaultValue": false,
        "description": "Displayed as a clockwise arc near the bezel"
      },
      {
        "type": "color",
        "messageKey": "BatteryColor",
        "defaultValue": "0x55AAFF",
        "label": "Battery Bar Color",
        "description": "On battery below 20% the color switchs to red"
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
      {
        "type": "slider",
        "messageKey": "UpSlider",
        "defaultValue": 30,
        "label": "Update frequency (minutes)",
        "description": "Weather and Location will update as frequently as you want. Note that more frequent requests will drain your phone battery faster  ",
        "min": 15,
        "max": 120,
        "step": 15},
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