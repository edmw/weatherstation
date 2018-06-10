#include <Wire.h>

#include "Notification.h"

extern const bool PRODUCTION;
extern Notification notification;

void i2c_setup() {
    #if defined(I2C_SDA) && defined(I2C_SCL)
    Wire.begin(I2C_SDA, I2C_SCL);
    #else
    Wire.begin();
    #endif
}

void i2c_scan() {
    int nDevices;

    notification.info(F("*I2C: Scanning ..."));

    nDevices = 0;
    for(byte address = 1; address < 127; address++ ) {
        Wire.beginTransmission(address);
        byte result = Wire.endTransmission();
        if (result == 0) {
            notification.info(F("*I2C: Device found at address "), address);
            nDevices++;
        }
        else if (result == 4) {
            notification.info(F("*I2C: Unknown error at address "), address);
        }
        else {
            notification.info(F("*I2C: No device at address "), address);
        }
        delay(100);
    }
    notification.info(F("*I2C: Number of devices found "), nDevices);
}
