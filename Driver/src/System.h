#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <Arduino.h>
#include <stdint.h>

#if defined(ESP8266)
#define RESET_REASON rst_reason
#elif defined(ESP32)
#include <rom/rtc.h>
#endif

#define TERMINATE_FATAL(message) notification.fatal(message); \
    std::terminate();
#define TERMINATE_FATAL_BLINK(message, count) notification.fatal(message, count); \
    std::terminate();
#define TERMINATE_FATAL_BLINK_RESTART(message, count) notification.fatal(message, count, false); \
    System::panic();

class System {

public:
    /*
      REASON_DEFAULT_RST // normal startup by power on
      REASON_WDT_RST // hardware watch dog reset
      REASON_EXCEPTION_RST // exception reset
      REASON_SOFT_WDT_RST // software watch	dog	reset
      REASON_SOFT_RESTART // software restart
      REASON_DEEP_SLEEP_AWAKE // wake up from deep sleep
      REASON_EXT_SYS_RST // external system reset
    */
    #if defined(ESP8266) || defined(ESP32)
    static RESET_REASON lastResetReason();
    static bool lastResetReasonIsDeepSleepAwake();
    static String lastException();
    #endif

    static void wifiOn();
    static void wifiOff();

    static void panic();
};

#endif
