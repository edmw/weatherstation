
#include <Arduino.h>

#include "Signaling.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

Signaling::Signaling(int ledpin) {
    this->ledpin = ledpin;
    this->production = false;
}

bool Signaling::begin(bool production) {
    this->production = production;

    if (ledpin >= 0) {
        pinMode(ledpin, OUTPUT);
        digitalWrite(ledpin, HIGH);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Signaling::signal_off(void) {
    if (ledpin >= 0) {
        digitalWrite(ledpin, HIGH);
    }
}
void Signaling::signal_toggle(void) {
    if (ledpin >= 0) {
        int state = digitalRead(ledpin);
        digitalWrite(ledpin, !state);
    }
}

void Signaling::signal_failure_once(int ms) {
    if (ledpin >= 0) {
        digitalWrite(ledpin, LOW);
        delay(ms);
        digitalWrite(ledpin, HIGH);
        delay(ms);
    }
}

void Signaling::signal_failure_forever(int ms) {
    while (ledpin >= 0) {
        digitalWrite(ledpin, LOW);
        delay(ms);
        digitalWrite(ledpin, HIGH);
        delay(ms);
    }
    while(1) {
        delay(1000);
    }
}

void Signaling::signal_failure_count(uint8_t num) {
    while (ledpin >= 0) {
        for (uint8_t i = 0; i < num; i++) {
            digitalWrite(ledpin, LOW);
            delay(500);
            digitalWrite(ledpin, HIGH);
            delay(500);
        }
        for (uint8_t i = 0; i < 20; i++) {
            digitalWrite(ledpin, LOW);
            delay(50);
            digitalWrite(ledpin, HIGH);
            delay(50);
        }
    }
    while(1) {
        delay(1000);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
