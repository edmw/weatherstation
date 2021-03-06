#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Weather Device driver which will
// * collect sensor measurements from various sensors
// * send sensor measurements to a server via wifi
// * go to deep sleep for a defined amount of time to save power
// https://github.com/edmw/weatherstation
//
// This driver is written to work on a ESP8266 and ESP32 controller chip.
// It will run with limited functionality on an Arduino system.
//
// This driver can send sensor measurements to an InfluxDB server using InfluxDB’s Line Protocol.
//
// Copyright (c) 2017-2020 Michael Baumgärtner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////
// Ideas for improvement
// * collect sensor measurements and store locally before sending to a server
//   + less power consumption
//   + more robust against wifi errors
//   - additional complexity
//   - needs local storage
//   - needs real time clock for timestamps
// * read pressure on mean sea level for location from internet service
//   + allows calculation of altitude from pressure measurement
//   - additional dependency
//   - could be done on server
// * check if there is a new driver version and perform an update over wifi when indicated
//   + no more cable
//   - requires some server side action
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

// Operating Support

#include "Signaling.h"
#include "Notification.h"

#include "Files.h"
#include "Values.h"

#include "Network.h"

#include "Clock.h"
#include "Switch.h"

#include "System.h"

#include "OTA.h"

// Hardware Support

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_VEML6070.h>
#include <DHT.h>
#include <SHTSensor.h>
#include <SPI.h>

// Weather Device

#include "Readings.h"
#include "Transport.h"

#include "I2C.h"
#include "I2CExtender.h"

// Configuration

#include "Driver.h"

#ifdef PRIVATE
#include "private.h"
#endif

// General

#include "SERIAL.h"
#include "millis.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

// Define identifier for a specific weather device. Derived from the ESP8266 or ESP32 chip id.
// This is used to identify a station on local network and as source identifier for measurements.
// Note: If compiled for an Arduino system statically set to a fixed string.
#if defined(ESP8266)
const String DEVICE_ID = "ESP" + String(ESP.getChipId());
#elif defined(ESP32)
const uint64_t macAddress = ESP.getEfuseMac();
const uint64_t macAddressTrunc = macAddress << 40;
const uint32_t chipId = macAddressTrunc >> 40;
const String DEVICE_ID = "ESP" + String(chipId);
#else
const String DEVICE_ID = "Arduino";
#endif

extern const int SKETCH_VERSION;

///////////////////////////////////////////////////////////////////////////////////////////////////

// Global toggle for production or development mode.
extern const bool PRODUCTION = PRODUCTION_MODE;

// Define measuring interval and minimum delay between measurements.
const long MEASURING_INTERVAL = PRODUCTION ? 5 * 60 * 1000 : 1 * 60 * 1000; // milliseconds
const long MEASURING_INTERVAL_DELAY_MIN = PRODUCTION ? 60 * 1000 : 10 * 1000; // milliseconds

///////////////////////////////////////////////////////////////////////////////////////////////////

Signaling signaling = Signaling(SIGNALING_LED);
Notification notification = Notification();

Files files = Files();
Values values = Values();

#ifdef PRIVATE
Network driver_network = Network(DEVICE_ID, NETWORK_SSID, NETWORK_PASS);
#else
Network driver_network = Network(DEVICE_ID);
#endif

Clock driver_clock = Clock(Clock::off);

#ifdef OTA_ON
OTA ota = OTA();
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C

// PIN for enabling I2C extender (see schematics, use -1 to disable extender).
// The driver will use this to minimize power consumption of extended I2C bus.

#ifdef I2C_EXTENDER_ON
#define I2C_EXTENDER_ENABLE D7 // GPIO13
#else
#define I2C_EXTENDER_ENABLE -1
#endif

I2CExtender i2c_extender(I2C_EXTENDER_ENABLE);

///////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT

#if defined(ESP8266)
ADC_MODE(ADC_VCC);
#endif

// Switch for selecting test mode (different from PRODUCTION flag)
// Must be connected to ground and closed to activate live mode.
#ifdef TEST_SWITCH_ON
#if !defined(TEST_SWITCH_PIN)
#define TEST_SWITCH_PIN D5
#endif
#else
#define TEST_SWITCH_PIN -1
#endif

Switch testSwitch(TEST_SWITCH_PIN, Switch::PullUp::Enable, 0);

// For sensors with analog outputs this driver can use an 16-Bit analog-to-digital converter.
// Supported sensors are
// * ML8511

// analogue input
Adafruit_ADS1115 ads(0x48);

// analogue reference 3.3V
uint16_t ads_reference;

///////////////////////////////////////////////////////////////////////////////////////////////////
// SENSORS

// Temperature
#ifdef DS18B20_ON
#define DS18B20_PIN D6 // 1-wire pin
const String DS18B20_ID = "DS18B20";
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
#define DS18B20_CALIBRATION_LO 1.4  // reference 0.01°C
#define DS18B20_CALIBRATION_HI 98.8 // reference 100°C
#define DS18B20_CALIBRATION_RANGE (DS18B20_CALIBRATION_HI - DS18B20_CALIBRATION_LO)
#endif

// Temperature + Pressure
#define BMP280_I2C 0x76
const String BMP280_ID = "BMP280";
Adafruit_BMP280 bmp280;

// Temperature + Pressure + Humidity
#define BME280_I2C 0x76
const String BME280_ID = "BME280";
Adafruit_BME280 bme280;

// Temperature + Humidity
#ifdef SHT30_ON
const String SHT30_ID = "SHT30";
SHTSensor sht(SHTSensor::SHT3X);
#endif

// Temperature + Humidity
#ifdef DHT22_ON
#define DHT22_PIN D3 // 1-wire pin
const String DHT22_ID = "DHT22";
DHT dht(DHT22_PIN, DHT22);
#endif

// Illuminance
#define TSL2561_I2C TSL2561_ADDR_FLOAT
const String TSL2561_ID = "TSL2561";
Adafruit_TSL2561_Unified tsl2561 = Adafruit_TSL2561_Unified(TSL2561_I2C, 12345);

// UV intensity
const String VEML6070_ID = "VEML6070";
Adafruit_VEML6070 veml6070 = Adafruit_VEML6070();

// UV intensity
#define ML8511_PIN D5 // enable pin
#define ML8511_ADS 1 // ads channel
const String ML8511_ID = "ML8511";

// checks

#if defined (BMP280_ON) && defined (BME280_ON)
#error You can not use BMP280 and BME280 simultaneously!
#endif

#if defined (ML8511_ON) && ! defined (ADS1115)
#error You need to have ADS1115 to use ML8511!
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// READINGS

Readings readings;

///////////////////////////////////////////////////////////////////////////////////////////////////
// VOLTAGE

bool setupVoltage(void) {
    #if defined(ESP8266)
    pinMode(A0, INPUT);
    #endif
    return true;
}

void readVoltage(Readings *readings) {
    #if defined(ESP8266)
    float v = ESP.getVcc();
    if (isnan(v)) {
        notification.warn(F("Failed to read voltage!"));
        return;
    }
    readings->store(v, Readings::voltage, "INTERNAL");
    #endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DS18B20
// datasheet: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
#ifdef DS18B20_ON

bool setupDS18B20(void) {
    ds18b20.begin();
    return true;
}

void readDS18B20(Readings *readings) {
    ds18b20.requestTemperatures();
    delay(100);
    // currently this driver only supports 1 sensor at index 0
    float t = ds18b20.getTempCByIndex(0);
    if (isnan(t)) {
        notification.warn(F("Failed to read from DS18B20 sensor!"));
        return;
    }
    // corrected temperature based on preceding calibration
    t = (((t - DS18B20_CALIBRATION_LO) * 99.99) / DS18B20_CALIBRATION_RANGE) + 0.01;
    readings->store(t, Readings::temperature_external, DS18B20_ID);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// BMP280
// datasheet: https://cdn-shop.adafruit.com/datasheets/BST-BMP280-DS001-11.pdf
//
// Weather monitoring
// Recommended filter settings for weather monitoring:
// Mode: forced
// Oversampling: ultra low power
// osrs_p: x1
// osrs_t: x1
// IIR filter: off

bool setupBMP280(void) {
    if (bmp280.begin(BMP280_I2C)) {
        return true;
    }
    TERMINATE_FATAL_BLINK(F("Failed to find a valid BMP280 sensor!"), 10);
}

void readBMP280(Readings *readings) {
    bmp280.readTemperature();
    bmp280.readPressure();
    delay(100);
    float t = bmp280.readTemperature();
    float p = bmp280.readPressure();
    if (isnan(t) || isnan(p)) {
        notification.warn(F("Failed to read from BMP280 sensor!"));
        return;
    }
    readings->store(t, Readings::temperature, BMP280_ID);
    readings->store(p, Readings::pressure, BMP280_ID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// BME280
// datasheet: https://cdn-shop.adafruit.com/datasheets/BST-BME280_DS001-10.pdf
// Normal mode: perpetual cycling of measurements and inactive periods
// Forced mode: perform one measurement, store results and return to sleep mode
// The standby time for normal mode can be set to between 0.5 and 1000 ms.
//
// Weather monitoring
// Description: Only a very low data rate is needed. Power consumption is minimal. Noise of
// pressure values is of no concern. Humidity, pressure and temperature are monitored.
// Suggested settings for weather monitoring
// Sensor mode forced mode, 1 sample / minute
// Oversampling settings pressure ×1, temperature ×1, humidity ×1
// IIR filter settings filter off
// Performance for suggested settings
// Current consumption 0.16 µA
// RMS Noise 3.3 Pa / 30 cm, 0.07 %RH
// Data output rate 1/60 Hz

bool setupBME280(void) {
    if (bme280.begin(BME280_I2C)) {
        bme280.setSampling(
            Adafruit_BME280::MODE_FORCED,
            Adafruit_BME280::SAMPLING_X1, // temperature
            Adafruit_BME280::SAMPLING_X1, // pressure
            Adafruit_BME280::SAMPLING_X1, // humidity
            Adafruit_BME280::FILTER_OFF,
            Adafruit_BME280::STANDBY_MS_10
        );
        return true;
    }
    TERMINATE_FATAL_BLINK(F("Failed to find a valid BME280 sensor!"), 11);
}

void readBME280(Readings *readings) {
  bme280.takeForcedMeasurement();
  delay(100);
  float t = bme280.readTemperature();
  float p = bme280.readPressure();
  float h = bme280.readHumidity();
  if (isnan(t) || isnan(p) || isnan(h)) {
      notification.warn(F("Failed to read from BME280 sensor!"));
      return;
  }
  readings->store(t, Readings::temperature, BME280_ID);
  readings->store(p, Readings::pressure, BME280_ID);
  readings->store(h, Readings::humidity, BME280_ID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SHT30
// datasheet: https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/2_Humidity_Sensors/Datasheets/Sensirion_Humidity_Sensors_SHT3x_Datasheet_digital.pdf
#ifdef SHT30_ON

bool setupSHT30(void) {
    sht.init();
    return true;
}

void readSHT30(Readings *readings) {
    if (sht.readSample()) {
        float t = sht.getTemperature();
        float h = sht.getHumidity();
        if (isnan(t) || isnan(h)) {
            notification.warn(F("Failed to read from SHT30 sensor!"));
            return;
        }
        readings->store(t, Readings::temperature, SHT30_ID);
        readings->store(h, Readings::humidity, SHT30_ID);
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// DHT22
// datasheet: http://akizukidenshi.com/download/ds/aosong/AM2302.pdf
#ifdef DHT22_ON

bool setupDHT22(void) {
    dht.begin();
    return true;
}

void readDHT22(Readings *readings) {
    dht.readTemperature();
    dht.readHumidity();
    delay(100);
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
        notification.warn(F("Failed to read from DHT22 sensor!"));
        return;
    }
    readings->store(t, Readings::temperature_external, DHT22_ID);
    readings->store(h, Readings::humidity, DHT22_ID);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// TSL2561
// datasheet: https://cdn-learn.adafruit.com/downloads/pdf/tsl2561.pdf

bool setupTSL2561() {
    tsl2561.enableAutoRange(true);
    tsl2561.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
    return true;
}

void readTSL2561(Readings *readings) {
    sensors_event_t event;
    tsl2561.getEvent(&event);
    float l = event.light;
    if (!l) {
        notification.warn(F("Failed to read from TSL2561 sensor!"));
        return;
    }
    readings->store(l, Readings::illuminance, TSL2561_ID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// VEML6070
// datasheet: https://cdn-learn.adafruit.com/assets/assets/000/032/482/original/veml6070.pdf

bool setupVEML6070() {
    veml6070.begin(VEML6070_1_T);
    return true;
}

void readVEML6070(Readings *readings) {
    delay(100);
    float u = veml6070.readUV();
    if (isnan(u)) {
        notification.warn(F("Failed to read from VEML6070 sensor!"));
        return;
    }
    readings->store(u, Readings::uvintensity, VEML6070_ID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ML8511
// datasheet: https://www.mcs.anl.gov/research/projects/waggle/downloads/datasheets/lightsense/ml8511.pdf

bool setupML8511() {
    return true;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void readML8511(Readings *readings) {
    delay(100);
    if (ads_reference > 0) {
        // read sensor value from analogue input
        uint16_t value = ads.readADC_SingleEnded(ML8511_ADS);
        // calculate sensor voltage with reference value for 3.3V
        float voltage = 3.3 / ads_reference * value;
        // map sensor input to uv intensity
        float u = mapfloat(voltage, 0.99, 2.9, 0.0, 15.0);
        if (isnan(u)) {
            notification.warn(F("Failed to read from ML8511 sensor!"));
            return;
        }
        readings->store(u, Readings::uvintensity, ML8511_ID);
    }
    else {
        notification.warn(F("Failed to read from ML8511 sensor (no reference)!"));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void readADS() {
    ads_reference = ads.readADC_SingleEnded(3);
    notification.info(F("ADC reference value (3V3): "), ads_reference);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void setupSensorsViaOneWire() {
    // Setup all sensors connected via 1-Wire protocol.

    #ifdef DS18B20_ON
    setupDS18B20();
    #endif

    #ifdef DHT22_ON
    setupDHT22();
    #endif
}

void setupSensorsViaI2C() {
    // Setup all sensors connected via I2C bus.

    i2c_setup();

    #ifdef I2C_DEBUG_ON
    if (!PRODUCTION) i2c_scan();
    #endif

    #ifdef BME280_ON
    setupBME280();
    #endif

    #ifdef BMP280_ON
    setupBMP280();
    #endif

    #ifdef SHT30_ON
    setupSHT30();
    #endif

    #ifdef TSL2561_ON
    setupTSL2561();
    #endif

    #ifdef VEML6070_ON
    setupVEML6070();
    #endif
}

void setupSensorsViaADS() {
    // Setup all sensors connected via analog-digital-converter.

    #ifdef ADS1115_ON
    ads.begin();
    #endif

    #ifdef ML8511_ON
    setupML8511();
    #endif
}


void setup() {
    SERIAL_BEGIN();
    //SERIAL_DEBUG();

    signaling.begin(PRODUCTION);
    notification.begin(PRODUCTION, &signaling);

    notification.info(F("Weather Device setup ... "),
        PRODUCTION ? F("PRODUCTION") : F("DEVELOPMENT")
    );

    testSwitch.begin();

    // A Files object is used to manage a file-system in Flash memory.
    if (!files.begin()) {
        TERMINATE_FATAL_BLINK(F("Failed: begin files"), 1);
    }
    // A Values object is used to manage a value-store in Flash memory.
    if (!values.begin(&files)) {
        TERMINATE_FATAL_BLINK(F("Failed: begin values"), 2);
    }

    // A Network object is used to manage local network access.
    if (!driver_network.begin(&values)) {
        TERMINATE_FATAL_BLINK(F("Failed: begin network"), 3);
    }

    #if defined(NETWORK_ON)
    if (driver_network.connect()) {

        #ifdef OTA_ON
        if (ota.begin()) {
            ota.performUpdate();
        }
        else {
            TERMINATE_FATAL_BLINK(F("Failed: begin update"), 5);
        }
        #endif

        if (driver_clock.begin()) {
            if (driver_clock.isRunning() && driver_clock.isIndeterminate()) {
                driver_clock.sync();
            }
        }
        else {
            TERMINATE_FATAL_BLINK(F("Failed: begin clock"), 6);
        }

        driver_network.disconnect();
    }
    else {
        TERMINATE_FATAL_BLINK_RESTART(F("Failed: connect to network"), 4);
    }
    #endif // NETWORK_ON

    // setup internal measurements
    setupVoltage();

    // setup OneWire sensors
    setupSensorsViaOneWire();

    // activate i2c extender if enabled
    i2c_extender.begin();
    i2c_extender.activate();

    // setup I2C sensors
    setupSensorsViaI2C();

    // setup analog sensors
    setupSensorsViaADS();
}


void deepSleepAndResetAfter(unsigned long sleep_millis);

void loop() {
    #ifdef TEST_SWITCH_ON
    bool test = testSwitch.read();
    #else
    bool test = !PRODUCTION;
    #endif

    notification.info(F("Weather Device running ... "),
        DEVICE_ID + "/" + String(SKETCH_VERSION)
    );

    notification.info(F("Get readings from sensors ..."));
    elapsed_millis get_readings_elapsed; // measure time needed for reading

    // activate i2c extender if enabled
    if (i2c_extender.activate()) {
        setupSensorsViaI2C();
        setupSensorsViaADS();
    }

    #ifdef ADS1115_ON
    readADS(); // get reference for analogue digital converter
    #endif

    // get readings from sensors
    // Note: Order is important! Later readings won't override earlier ones!
    readings.clear();

    readVoltage(&readings);

    #ifdef DS18B20_ON
    readDS18B20(&readings);
    #endif

    #if defined (SHT30_ON)
    readSHT30(&readings);
    #endif

    #if defined (BMP280_ON) && ! defined (BME280_ON)
    readBMP280(&readings);
    #endif

    #if defined (BME280_ON) && ! defined (BMP280_ON)
    readBME280(&readings);
    #endif

    #ifdef DHT22_ON
    readDHT22(&readings);
    #endif

    #ifdef TSL2561_ON
    readTSL2561(&readings);
    #endif

    #ifdef VEML6070_ON
    readVEML6070(&readings);
    #endif

    #if defined (ML8511_ON) && defined (ADS1115)
    readML8511(&readings);
    #endif

    if (!PRODUCTION) {
        readings.print();
    }

    // deactivate i2c extender if enabled
    i2c_extender.deactivate();

    unsigned long get_readings_millis = get_readings_elapsed; // get time needed for reading
    notification.info_millis(F("Done getting readings from sensors ... "), get_readings_millis);


    // push readings to server
    #if defined (NETWORK_ON) && defined (TRANSPORT_ON)

    notification.info(F("Push readings to server ..."),
        test ? F("TEST") : TRANSPORT_DATABASE
    );
    elapsed_millis push_readings_elapsed; // measure time needed for sending

    if (driver_network.connect()) {
        driver_clock.sync();

        Transport transport(
            TRANSPORT_SERVER,
            TRANSPORT_PORT,
            test ? "test" : TRANSPORT_DATABASE,
            DEVICE_ID,
            PROBE_LOCATION
        );
        if (transport.begin(&driver_network)) {
            if (transport.send(readings)) {
                notification.info(F("Sent readings!"));
            }
            else {
                notification.warn(F("Failed to send readings!"));
            }
        }
        else {
            notification.warn(F("Failed to begin transport!"));
        }

        driver_network.disconnect();
    }
    else {
        notification.warn(F("Failed to connect to network!"));
    }

    unsigned long push_readings_millis = push_readings_elapsed; // get time needed for sending
    notification.info_millis(F("Done pushing readings to server ... "), push_readings_millis);

    #else // defined (NETWORK_ON) && defined (TRANSPORT_ON)

    notification.info(F("Skip pushing readings to server ... "),
        test ? F("TEST") : TRANSPORT_DATABASE
    );

    #endif // defined (NETWORK_ON) && defined (TRANSPORT_ON)

    // loop

    long interval = MEASURING_INTERVAL - get_readings_millis - 500;
    #if defined (NETWORK_ON) && defined (TRANSPORT_ON)
    interval = interval - push_readings_millis;
    #endif
    long interval_delay = std::max(interval, MEASURING_INTERVAL_DELAY_MIN);
    #ifdef DEEPSLEEP_ON
    notification.info_millis(F("*Sleeping for "), interval_delay);
    delay(500);
    deepSleepAndResetAfter(interval_delay);
    #else
    notification.info_millis(F("*Delaying for "), interval_delay);
    delay(500);
    delay(interval_delay);
    #endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Deep sleep for the specified amount of milliseconds and start again with reset.
void deepSleepAndResetAfter(unsigned long sleep_millis) {
    #if defined(ESP8266) || defined(ESP32)
    ESP.deepSleep(sleep_millis * 1000);
    #else
    delay(sleep_millis);
    #endif
}
