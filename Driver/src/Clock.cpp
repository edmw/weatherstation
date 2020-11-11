#include <Arduino.h>

#include <WiFiUdp.h>
#include <NTPClient.h>

#include "Clock.h"

#include "Notification.h"

#include "System.h"

extern Notification notification;

///////////////////////////////////////////////////////////////////////////////////////////////////

WiFiUDP timeUDP;

NTPClient timeClient(timeUDP, "europe.pool.ntp.org");

DateTime pastpresent(__DATE__, __TIME__);

///////////////////////////////////////////////////////////////////////////////////////////////////

Clock::Clock(clock_type type) {
    this->type = type;
}

bool Clock::begin(void) {
    notification.info(F("*CLOCK: TYPE: "), type);

    switch (type) {
    case real:
        return rtc.begin();
    case soft:
        rtc0.begin(DateTime(2000,1,1,0,0,0));
        return true;
    case off:
        return true;
    }
    return false;
}

DateTime Clock::now(void) {
    switch (type) {
    case real:
        return rtc.now();
    case soft:
        return rtc0.now();
    case off:
        return DateTime();
    }
    TERMINATE_FATAL(F("Invalid clock type!"));
}

void Clock::adjust(const DateTime& dt) {
    switch (type) {
    case real:
        rtc.adjust(dt);
    case soft:
        rtc0.adjust(dt);
    case off:
        return;
    }
    TERMINATE_FATAL(F("Invalid clock type!"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool Clock::isRunning(void) {
    return (type == soft) || (type == real);
}

bool Clock::isIndeterminate(void) {
    switch (type) {
    case real:
        if (rtc.lostPower()) {
            return true;
        }
        // sanity check
        if (rtc.now().unixtime() < pastpresent.unixtime()) {
            return true;
        }
        return false;
    case soft:
        // always true for soft rtc
        return true;
    case off:
        return false;
    }
    TERMINATE_FATAL(F("Invalid clock type!"));
}

void Clock::sync(void) {
    timeClient.begin();
    if (timeClient.update()) {
        DateTime datetime = DateTime(timeClient.getEpochTime());
        adjust(datetime);
        notification.info(F("Time (NTP): "), formatDateTimeISO8601(datetime));
    }
    else {
        notification.warn(F("Failure to sync time!"));
    }
    timeClient.end();
}

String Clock::formatISO8601() {
    return formatDateTimeISO8601(now());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

String formatDateTimeISO8601(DateTime dt) {
    char string[20];
    snprintf(string, sizeof(string), "%04d-%02d-%02dT%02d:%02d:%02dZ",
        dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second()
    );
    return String(string);
}
