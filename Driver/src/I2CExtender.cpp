
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

///////////////////////////////////////////////////////////////////////////////////////////////////
