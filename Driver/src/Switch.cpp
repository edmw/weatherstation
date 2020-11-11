#include "Switch.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

class Switch::Implementation {
public:
    Implementation(uint8_t pin, PullUp enablePullUp, uint32_t debounceTime)
        : pin(pin), pinPullUp(bool(enablePullUp)), debounceTime(debounceTime) {
        state = false;
        stateMillis = millis();
        stateChanged = false;
        stateChangedMillis = stateMillis;
        lastState = state;
    }

    bool begin(void) {
        if (pin > 0) {
            if (pinPullUp != 0) {
                pinMode(pin, INPUT_PULLUP);
            }
            else {
                pinMode(pin, INPUT);
            }

            state = !(digitalRead(pin) == HIGH);
        }
        return true;
    }

    bool read(void) {
        if (pin > 0) {
            uint32_t ms = millis();
            if (stateMillis - stateChangedMillis >= debounceTime) {
                lastState = state;

                state = !(digitalRead(pin) == HIGH);
                stateChanged = (state != lastState);
                if (stateChanged) {
                    stateChangedMillis = ms;
                }
            }
            else {
                stateChanged = false;
            }
            stateMillis = ms;
        }
        return state;
    }

    bool isClosed(void) {
        return state;
    }

    bool isOpen(void) {
        return !state;
    }

private:
    uint8_t pin;
    uint8_t pinPullUp;
    uint32_t debounceTime;

    bool state;
    bool stateChanged;
    uint32_t stateMillis;
    uint32_t stateChangedMillis;

    bool lastState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

Switch::Switch(uint8_t pin, PullUp enablePullUp, uint32_t debounceTime)
    : impl(new Implementation(pin, enablePullUp, debounceTime)) {
}

Switch::~Switch() {
    delete impl;
}

bool Switch::begin() {
    return impl->begin();
}

bool Switch::read() {
    return impl->read();
}

bool Switch::isClosed() {
    return impl->isClosed();
}

bool Switch::isOpen(void) {
    return impl->isOpen();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
