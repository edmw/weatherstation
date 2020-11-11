#include <Arduino.h>

/////////////////
// WeatherStick

#ifdef WEATHER_BEDROOM
const String PROBE_LOCATION { "bedroom" };
#else
const String PROBE_LOCATION { "unknown" };
#endif

#define PRODUCTION_MODE false
#undef TEST_SWITCH_ON

#if defined(ARDUINO_M5Stick_C)
#define SIGNALING_LED 10
#else
#define SIGNALING_LED -1
#endif

#define DEEPSLEEP_ON
#define NETWORK_ON
#define TRANSPORT_ON
#undef I2C_DEBUG_ON
#undef I2C_EXTENDER_ON

#undef ADS1115_ON
#undef DS18B20_ON
#define BMP280_ON
#undef BME280_ON
#undef DHT22_ON
#undef TSL2561_ON
#undef VEML6070_ON
#undef ML8511_ON

#undef OTA_ON
