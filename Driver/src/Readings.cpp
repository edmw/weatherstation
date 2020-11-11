
#include <Arduino.h>

#include <float.h>

#include "Readings.h"

#include "SERIAL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

Readings::Readings(void) { }

///////////////////////////////////////////////////////////////////////////////////////////////////

void Readings::clear(void) {
    readings.timestamp = 0;
    readings.temperature = NAN;
    readings.temperature_external = NAN;
    readings.pressure = NAN;
    readings.humidity = NAN;
    readings.illuminance = NAN;
    readings.uvintensity = NAN;
    readings.voltage = NAN;
    for (int i = 0; i <= READING_TYPE_MAX; i++) {
        sensor_ids[i] = String();
    }
}

/// Stores the specified value for the specified type of reading. If a value is stored for that
/// reading type already, does nothing. So, be sure to clear the readings before trying to store new
/// readings.
void Readings::store(float value, reading_type type, String sensor_id) {
    bool stored = false;
    switch (type) {
    case voltage:
        if (isnan(readings.voltage)) {
            readings.voltage = value;
            stored = true;
        }
        break;
    case temperature:
        if (isnan(readings.temperature)) {
            readings.temperature = value;
            stored = true;
        }
        break;
    case temperature_external:
        if (isnan(readings.temperature_external)) {
            readings.temperature_external = value;
            stored = true;
        }
        break;
    case pressure:
        if (isnan(readings.pressure)) {
            readings.pressure = value;
            stored = true;
        }
        break;
    case humidity:
        if (isnan(readings.humidity)) {
            readings.humidity = value;
            stored = true;
        }
        break;
    case illuminance:
        if (isnan(readings.illuminance)) {
            readings.illuminance = value;
            stored = true;
        }
        break;
    case uvintensity:
        if (isnan(readings.uvintensity)) {
            readings.uvintensity = value;
            stored = true;
        }
        break;
    }
    if (stored) {
        sensor_ids[type] = sensor_id;
    }
}
void Readings::store(float value, reading_type type) {
    store(value, type, String());
}

float Readings::retrieve(reading_type type, String &sensor_id) {
    sensor_id = sensor_ids[type];
    return retrieve(type);
}
float Readings::retrieve(reading_type type) {
    switch (type) {
    case voltage:
        return readings.voltage;
    case temperature:
        return readings.temperature;
    case temperature_external:
        return readings.temperature_external;
    case pressure:
        return readings.pressure;
    case humidity:
        return readings.humidity;
    case illuminance:
        return readings.illuminance;
    case uvintensity:
        return readings.uvintensity;
    }
    return NAN;
}

void Readings::print(reading_type type) {
    String sensor_id;
    float value = retrieve(type, sensor_id);
    if (!isnan(value)) {
        switch (type) {
        case voltage:
            SERIAL_PRINTF(value/1000, 1);
            SERIAL_PRINT(F(" V"));
            break;
        case temperature:
        case temperature_external:
            SERIAL_PRINTF(value, 1);
            SERIAL_PRINT(F(" °C"));
            break;
        case pressure: // in pascal
            SERIAL_PRINTF(value/100, 2);
            SERIAL_PRINT(F(" hPa"));
            break;
        case humidity: // in percent
            SERIAL_PRINTF(value, 0);
            SERIAL_PRINT(F(" \045"));
            break;
        case illuminance: // in lux
            SERIAL_PRINTF(value, 1);
            SERIAL_PRINT(F(" lm/m^2"));
            break;
        case uvintensity: // in mW/cm^2
            SERIAL_PRINTF(value * 10, 1);
            SERIAL_PRINT(F(" W/m^2"));
            break;
        }
        String s = " (" + sensor_id + ")";
        SERIAL_PRINT(s);
    }
    else {
        SERIAL_PRINT(F("N/A"));
    }
}

void Readings::print(void) {
    SERIAL_PRINT(F("Voltage:                "));
    print(voltage);
    SERIAL_PRINTLN();

    SERIAL_PRINT(F("Temperature:            "));
    print(temperature);
    SERIAL_PRINTLN();

    SERIAL_PRINT(F("Temperature (external): "));
    print(temperature_external);
    SERIAL_PRINTLN();

    SERIAL_PRINT(F("Pressure:               "));
    print(pressure);
    SERIAL_PRINTLN();

    SERIAL_PRINT(F("Humidity:               "));
    print(humidity);
    SERIAL_PRINTLN();

    SERIAL_PRINT(F("Illuminance:            "));
    print(illuminance);
    SERIAL_PRINTLN();

    SERIAL_PRINT(F("UV Intensity:           "));
    print(uvintensity);
    SERIAL_PRINTLN();
}
