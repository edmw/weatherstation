
#include <Arduino.h>
#include <Wire.h>

#include "I2CExtender.h"

#include "SERIAL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

I2CExtender::I2CExtender(int enablepin) {
    this->enablepin = enablepin;
}

bool I2CExtender::begin() {
    if (enablepin >= 0) {
        pinMode(enablepin, OUTPUT);
        digitalWrite(enablepin, LOW);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// returns true, if i2c extender has been activated
bool I2CExtender::activate(void) {
    if (enablepin >= 0) {
        int state = digitalRead(enablepin);
        if (state != HIGH) {
            digitalWrite(enablepin, HIGH);
            delay(1000);
            return true;
        }
    }
    return false;
}
void I2CExtender::deactivate(void) {
    if (enablepin >= 0) {
        digitalWrite(enablepin, LOW);
    }
}

void I2CExtender::scan() {
    if (enablepin < 0) return;

    int state = digitalRead(enablepin);
    SERIAL_PRINT(F("I2C Scan: Extender="));
    SERIAL_PRINTB(state);
    SERIAL_PRINTLN();

    byte error;
    byte address;

    int count = 0;

    SERIAL_PRINTLN(F("I2C Scan..."));

    Wire.begin();

    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            SERIAL_PRINT(F("I2C Scan: Device found at address "));
            SERIAL_PRINT_BYTE(address);
            SERIAL_PRINTLN();
            count++;
        }
        else if (error == 4) {
            SERIAL_PRINT(F("I2C Scan: Error "));
            SERIAL_PRINT(error);
            SERIAL_PRINT(F(" at address "));
            SERIAL_PRINT_BYTE(address);
            SERIAL_PRINTLN();
        }
        delay(10);
    }
    if (count == 0) {
        SERIAL_PRINTLN(F("I2C Scan: No devices found"));
    }
    else {
        SERIAL_PRINTLN(F("I2C Scan: Done"));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
