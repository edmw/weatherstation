#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////

// DEVICE CONFIGURATION

#if defined(WEATHER_STATION)
#include "Driver_WeatherStation.h"
#elif defined(WEATHER_STICK)
#include "Driver_WeatherStick.h"
#else
#error "No Device configuration available!"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

// TRANSPORT CONFIGURATION

// Define transport to InfluxDB server
const String TRANSPORT_SERVER   = "192.168.178.111";
const int    TRANSPORT_PORT     = 18086;
const String TRANSPORT_DATABASE = "homemonitor";

///////////////////////////////////////////////////////////////////////////////////////////////////

// OTA UPDATE CONFIGURATION

const String OTA_URL = "http://192.168.178.57:8080/ota/update";

#endif
