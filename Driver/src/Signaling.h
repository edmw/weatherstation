#ifndef __SIGNALING_H__
#define __SIGNALING_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Operating Support:
// Class to produces output to a digital pin. Which most often will be a LED.
///////////////////////////////////////////////////////////////////////////////////////////////////

class Signaling {
public:
    Signaling(int ledpin);

    // Begin handling output. Must be called before any other method.
    bool begin(bool production);

    // Turns the output off.
    void signal_off(void);
    // Toggles the output’s state.
    void signal_toggle(void);

    // Signals a failure by blinking output once.
    void signal_failure_once(int ms);
    // Signals a failure by blinking a number of counts once.
    void signal_failure_count_once(uint8_t num);
    // Signals a failure by blinking output forever. This method does not return.
    void signal_failure_forever(int ms);
    // Signals a failure by blinking a number of counts forever. This method does not return.
    void signal_failure_count_forever(uint8_t num);

private:
    int ledpin;

    bool production;
};

#endif
