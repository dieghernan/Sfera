var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);




var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  var settings = JSON.parse(localStorage.getItem('clay-settings')) || {};
  var units = unitsToString(settings.WeatherUnit);
  //var ran=-1*Math.random()*8;
  
  // Construct URL
  //Get JSON from Yahoo Weather
   var urlyahoo = 'https://query.yahooapis.com/v1/public/yql?q=select item.condition, ' +
      'location from weather.forecast(1) where woeid in ' +
      '(select woeid from geo.places(1) where ' +
      'text=\'(' + pos.coords.latitude + ',' + pos.coords.longitude + ')\') and ' +
      // Placeholder 'text= \'(40.398897,-3.710222700000031)\') and ' +
      'u=\'' + units + '\'&format=json';   
      console.log("WeatherUrl= " + urlyahoo);
  
  

  // Send request to Yahoo for weather
  xhrRequest(encodeURI(urlyahoo), 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log("units are " + units );      
      var temperature = Math.round(json.query.results.channel.item.condition.temp) +"°" + units;
      console.log("Temperature is " + temperature);  
      // Conditions item.condition.code
      var conditions = Math.round(json.query.results.channel.item.condition.code);      
      console.log("Conditions are " + conditions);
      var weatok=1;
      // Assemble dictionary using our keys
      var dictionary = {
        "WeatherTemp": temperature,
        "WeatherCond": conditions,
        "WeatOK":weatok
      };
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );  
  
  //Request location
  var langtouse=translate(navigator.language);
  var urlgeoplaces = 'https://query.yahooapis.com/v1/public/yql?q=' +
      'select locality1.content,locality2.content from geo.places(1) where ' +
      'text=\'(' + pos.coords.latitude + ',' + pos.coords.longitude + ')\' and ' +
      // Placeholder  'text= \'(40.398897,-3.710222700000031)\' and ' +
      'lang=\'' + langtouse +'\'&format=json'; 
      console.log("GeoPlacesUrl= " + urlgeoplaces);
  
  //Send request to GeoPlaces
   xhrRequest(encodeURI(urlgeoplaces), 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var jsonloc = JSON.parse(responseText);
      console.log("lang is " + langtouse );
      // Location
      var location=jsonloc.query.results.place.locality2;
      console.log("location is " + location);
      var city=jsonloc.query.results.place.locality1; 
      console.log("city is "+ city);
      var GPSok=1;
      // Assemble dictionary using our keys
      var dictionary = {
        "NameLocation": location,
        "NameCity": city,
        "WeatOK":GPSok
      };
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Location info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending location info to Pebble!");
        }
      );
    }      
  );  
}
function locationError(err) {
  console.log("Error requesting weather!");
}
function getinfo() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 1}
  );
}
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    // Get the initial weather
    getinfo();
    }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getinfo();
   }                     
);

// Listen for when the Config app changes
Pebble.addEventListener('webviewclosed',
  function(e) {
    console.log("Updating config!");
      getinfo();
    }                     
);   
               
//functions                        
function unitsToString(unit) {
  if (unit) {
    return 'f';
  }
  return 'c';
}

function translate(langloc){
  if (langloc==='es-ES'){
    return 'es';
  }
  else if (langloc==='fr_FR'){
    return 'fr';  
  }
  else if (langloc==='de_DE'){
    return 'de';
  }
  else if (langloc==='it_IT'){
    return 'it';
  } 
  else if (langloc==='pt_PT'){
    return 'pt';
  } 
  else {
    return 'en';
  }
}

