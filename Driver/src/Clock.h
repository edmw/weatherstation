#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Operating Support:
// Class to manage access to a Real-time Clock module.
// Supports DS3231 hardware clock and software RTC based on millis().
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <RTClib.h>

class Clock {
public:
    enum clock_type {
      off = 0,
      soft = 1, // software-based RTC using millis()
      real = 2, // hardware-based RTC using DS3231
      CLOCK_TYPE_MAX = real
    };

    Clock(clock_type type);

    // Begin managing a clock. Must be called before any other method.
    bool begin(void);

    // Checks if the clock is running.
    bool isRunning(void);
    // Checks if the clock is in an indeterminate state. You could call sync then.
    bool isIndeterminate(void);

    // Syncs the clock with a NTP server. Needs an active WiFi connection.
    void sync(void);

    // Returns the current time as ISO8601 formatted string.
    String formatISO8601(void);

private:
    clock_type type;

    RTC_Millis rtc0; // soft RTC
    RTC_DS3231 rtc;  // real RTC

    DateTime now(void);

    void adjust(const DateTime& dt);
};

// Returns the given DateTime object as ISO8601 formatted string.
String formatDateTimeISO8601(DateTime dt);

#endif
