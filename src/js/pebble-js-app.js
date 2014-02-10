var RESOURCE_ID_ICON_CLEAR_DAY           = 0;
var RESOURCE_ID_ICON_CLEAR_NIGHT         = 1;
var RESOURCE_ID_ICON_RAIN                = 2;
var RESOURCE_ID_ICON_SNOW                = 3;
var RESOURCE_ID_ICON_SLEET               = 4;
var RESOURCE_ID_ICON_WIND                = 5;
var RESOURCE_ID_ICON_FOG                 = 6;
var RESOURCE_ID_ICON_CLOUDY              = 7;
var RESOURCE_ID_ICON_PARTLY_CLOUDY_DAY   = 8;
var RESOURCE_ID_ICON_PARTLY_CLOUDY_NIGHT = 9;
var RESOURCE_ID_ICON_THUNDER             = 10;
var RESOURCE_ID_ICON_RAIN_SNOW           = 11;
var RESOURCE_ID_ICON_SNOW_SLEET          = 12;
var RESOURCE_ID_ICON_COLD                = 13;
var RESOURCE_ID_ICON_HOT                 = 14;
var RESOURCE_ID_ICON_ERROR               = 15;

function iconFromWeatherId(weatherId, isNight)
{
    if (weatherId < 300) {
        return RESOURCE_ID_ICON_THUNDER;
    } else if (weatherId < 600) {
        return RESOURCE_ID_ICON_RAIN;
    } else if (weatherId < 700) {
        if(weatherId < 611) {
            return RESOURCE_ID_ICON_SNOW;
        } else if (weatherId < 615) {
            return RESOURCE_ID_ICON_SLEET;
        } else {
            return RESOURCE_ID_ICON_RAIN_SNOW;
        }
    } else if (weatherId < 800) {
        return RESOURCE_ID_ICON_FOG;
    } else if (weatherId < 900) {
        if(weatherId < 801) {
            if(isNight) { return RESOURCE_ID_ICON_CLEAR_NIGHT; } else { return RESOURCE_ID_ICON_CLEAR_DAY; }
        } else if(weatherId < 803) {
            if(isNight) { return RESOURCE_ID_ICON_PARTLY_CLOUDY_NIGHT; } else { return RESOURCE_ID_ICON_PARTLY_CLOUDY_DAY; }
        }
        else {
            return RESOURCE_ID_ICON_CLOUDY;
        }
    } else if(weatherId == 903) {
        return RESOURCE_ID_ICON_COLD;
    } else if(weatherId == 904) {
        return RESOURCE_ID_ICON_HOT;
    } else if(weatherId == 905) {
        return RESOURCE_ID_ICON_WINDY;
    } else {
        if(isNight) { return RESOURCE_ID_ICON_CLEAR_NIGHT; } else { return RESOURCE_ID_ICON_CLEAR_DAY; }
    }
}

function fetchWeather(location, units)
{
    var response;
    var req = new XMLHttpRequest();

    weather_api_uri = "http://api.openweathermap.org/data/2.5/forecast/city?q="+location+"&units="+units+"&type=hour";
    req.open('GET', 
             weather_api_uri, 
             true);
    console.log(weather_api_uri);
    req.onload = function(e)
    {
        if (req.readyState == 4)
        {
            if(req.status == 200)
            {
                // console.log(req.responseText);
                response = JSON.parse(req.responseText);
                var temperature, icon;
                if (response && response.list && response.list.length > 0)
                {
                    var weatherResult = response.list[0];
                    temperature = Math.round(weatherResult.main.temp);
                    iconStr = weatherResult.weather[0].icon
                    icon = iconFromWeatherId(weatherResult.weather[0].id, iconStr[iconStr.length-1] == "n");
                    console.log(temperature);
                    console.log(icon);
                    city = response.city.name;
                    console.log(city);
                    Pebble.sendAppMessage({"icon":icon, "temperature":temperature});
                }
            } 
            else 
            {
                console.log("Error");
            }
        }
    }
    req.send(null);
}

var settings = { "location": "Chicago,IL", 
                 "units" : "metric" };

Pebble.addEventListener("ready", function(e) {
    console.log("connect!" + e.ready);
    console.log(e.type);
    settings.location = localStorage.getItem("settings.location");
    settings.units = localStorage.getItem("settings.units");
    Pebble.sendAppMessage({"icon" : 0, "temperature" : 1000});
});

Pebble.addEventListener("appmessage", function(e) {
    console.log(e.type);
    if(settings.location == undefined || settings.units == undefined)
    {
        settings = { "location": "Chicago,IL",
                     "units" : "metric" };
    }
    localStorage.setItem("settings.location", settings.location);
    localStorage.setItem("settings.units", settings.units);
    console.log("fetch weather: " + settings.location + ", " + settings.units);
    fetchWeather(settings.location, settings.units);
});

Pebble.addEventListener("showConfiguration", function(e) {
    console.log("show configuration!");
    if(settings == undefined || 
       settings.location == undefined || 
       settings.units == undefined)
    {
        settings = { "location": "Chicago,IL",
                     "units" : "metric" };
    }
    var url = "http://chetant.info/roboto2_config.html?location="+settings.location+"&units="+settings.units;
    console.log(url);
    Pebble.openURL(url);
    console.log(e.type);
});

Pebble.addEventListener("webviewclosed", function(e) {
    console.log("webview closed");
    console.log(e.type);
    console.log(e.response);
    settings = JSON.parse(decodeURIComponent(e.response));
    console.log("got: " + settings.location + ", " + settings.units);
    localStorage.setItem("settings.location", settings.location);
    localStorage.setItem("settings.units", settings.units);
    fetchWeather(settings.location, settings.units);
});
