#include <Arduino.h>

///////////////////
// WeatherStation

const String PROBE_LOCATION { "terrace" };

#define PRODUCTION_MODE false

#define TEST_SWITCH_ON

// PIN for signaling LED (see schematics, use -1 to disable status led)
// The driver will use this to signal failure states in execution.

#define SIGNALING_LED LED_BUILTIN

// Enable deep sleep mode: Undef to pause execution instead of entering deep sleep.
// Note: GPIO16 must be connected to RST for deep sleep mode to work.
// Note: This will have no effect at all if compiled for an Arduino system.
#define DEEPSLEEP_ON

// Enable network for clock synchronisation and measurement transport:
// Undef to disable all network.
#define NETWORK_ON

// Enable transport to InfluxDB server: Undef to disable sending measurements.
#define TRANSPORT_ON

// Enable I2C debug mode: Undef to disable debug mode.
#undef I2C_DEBUG_ON

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

// Enable over-the-air updates (network must be enabled, too).
#undef OTA_ON
