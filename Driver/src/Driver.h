#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "Arduino.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

// DEVICE CONFIGURATION

#if defined(WEATHER_STATION)

///////////////////
// WeatherStation

const String PROBE_LOCATION {"terrace"};

#define PRODUCTION_MODE true

// PIN for signaling LED (see schematics, use -1 to disable status led)
// The driver will use this to signal failure states in execution.

#define SIGNALING_LED LED_BUILTIN

// Enable deep sleep mode: Undef to pause execution instead of entering deep sleep.
// Note: GPIO16 must be connected to RST for deep sleep mode to work.
// Note: This will have no effect at all if compiled for an Arduino system.
#define DEEPSLEEP_ON

// Enable transport to InfluxDB server: Undef to disable sending measurements.
#define TRANSPORT_ON

// Enable I2C extender: Undef to disable extender.
#define I2C_EXTENDER_ON

// Sensors
#undef ADS1115_ON
#define DS18B20_ON
#undef BMP280_ON
#define BME280_ON
#undef DHT22_ON
#undef TSL2561_ON
#undef VEML6070_ON
#undef ML8511_ON

#elif defined(WEATHER_STICK)

/////////////////
// WeatherStick

#ifdef WEATHER_BEDROOM
const String PROBE_LOCATION { "bedroom" };
#else
const String PROBE_LOCATION { "unknown" };
#endif

#define PRODUCTION_MODE false

#define SIGNALING_LED -1

#undef DEEPSLEEP_ON
#define TRANSPORT_ON
#undef I2C_EXTENDER_ON

#undef ADS1115_ON
#undef DS18B20_ON
#undef BMP280_ON
#define BME280_ON
#undef DHT22_ON
#undef TSL2561_ON
#undef VEML6070_ON
#undef ML8511_ON

#else
#error "No Device configuration available!"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

// TRANSPORT CONFIGURATION

// Define transport to InfluxDB server
const String TRANSPORT_SERVER   = "192.168.178.111";
const int    TRANSPORT_PORT     = 18086;
const String TRANSPORT_DATABASE = "homemonitor";

#endif
