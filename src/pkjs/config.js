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
        "defaultValue": "Color settings"
      },
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "0x000000",
        "label": "Background"
      },
      {
        "type": "color",
        "messageKey": "ForegroundColor",
        "defaultValue": "0xFFFFFF",
        "label": "Text"
      },
      {
        "type": "color",
        "messageKey": "DotsColor",
        "defaultValue": "0xFFFFFF",
        "label": "Dots"
      }      
    ]
  },
  
 //Yahoo disclaimer
  {
      "type": "heading",
      "defaultValue": "Weather info provided by <a href=https://www.yahoo.com/?ilc=401>Yahoo.com </a>",
      "size":6
  },
      
 {
    "type": "submit",
    "defaultValue": "Guardar"
  }
];