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


function locationSuccessloc(pos){
  var langtouse=translate(navigator.language);
  var ran=-1*Math.random()*8;
  // Get JSON from Geolocation
  var urlgeocities = 'https://query.yahooapis.com/v1/public/yql?q=' +
      'select locality1.content,locality2.content from geo.places(1) where ' +
      'text=\'(' + pos.coords.latitude + ',' + pos.coords.longitude + ')\' and ' +
// Placeholder  'text= \'(40.398897,-3.710222700000031)\' and ' +
      'lang=\'' + langtouse +'\'&format=json'; 
       console.log("Geocities is " + urlgeocities);
  
  console.log("Requesting location");
 // Send request to Yahoo for weather
  xhrRequest(encodeURI(urlgeocities), 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var jsonloc = JSON.parse(responseText);
      console.log("lang is " + langtouse );
      
// Location
      var location=jsonloc.query.results.place.locality2;
      var city=jsonloc.query.results.place.locality1;
      console.log("location is " + location);
      console.log("city is "+ city);
      
     
    
      // Assemble dictionary using our keys
      var dictionary = {
        "NameLocation": location,
        "NameCity": city};

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
  




function locationSuccess(pos) {
  var settings = JSON.parse(localStorage.getItem('clay-settings')) || {};
  var units = unitsToString(settings.WeatherUnit);
  var ran=-1*Math.random()*8;
  
  // Construct URL
  //Get JSON from Yahoo Weather
   var urlyahoo = 'https://query.yahooapis.com/v1/public/yql?q=select item.condition, ' +
      'location from weather.forecast(1) where woeid in ' +
      '(select woeid from geo.places(1) where ' +
      'text=\'(' + pos.coords.latitude + ',' + pos.coords.longitude + ')\') and ' +
// Placeholder 'text= \'(40.398897,-3.710222700000031)\') and ' +
      'u=\'' + units + '\'&format=json';   
      console.log("Url is " + urlyahoo);
  
  

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
      
     
    
      // Assemble dictionary using our keys
      var dictionary = {
        "WeatherTemp": temperature,
        "WeatherCond": conditions};

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
}

function locationError(err) {
  console.log("Error requesting weather!");
  // Send blanks
  var temperature="";
  var conditions=49;    //Return a null value
     // Assemble dictionary using our keys
      var dictionary = {
        "WeatherTemp": temperature,
        "WeatherCond": conditions};
  // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Sending blank values for Weather to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
  );  
}
function locationErrorloc(err) {
  console.log("Error requesting location!");
  // Send blanks
    var location="";
    var city="";
     // Assemble dictionary using our keys
    var dictionary = {
        "NameLocation": location,
        "NameCity": city};

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Sending blank values for Location to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending location info to Pebble!");
        }
  );  
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function getlocation(){
    navigator.geolocation.getCurrentPosition(
    locationSuccessloc,
    locationErrorloc,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    
    // Get the initial weather
    getWeather();
    getlocation();
    }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
    getlocation();
   }                     
);

// Listen for when the Config app changes
Pebble.addEventListener('webviewclosed',
  function(e) {
    console.log("Updating config!");
       getWeather();
       getlocation(); 
    }                     
);   
               
              
                        
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

