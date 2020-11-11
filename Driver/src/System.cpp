#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <esp_task_wdt.h>
#endif

#include "System.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(ESP8266)

extern "C" {
    #include "user_interface.h"
}

RESET_REASON System::lastResetReason() {
    struct rst_info *info = system_get_rst_info();
    return static_cast<RESET_REASON>(info->reason);
}

bool lastResetReasonIsDeepSleepAwake() {
    return System::lastResetReason() == REASON_DEEP_SLEEP_AWAKE;
}

String System::lastException() {
    struct rst_info *info = system_get_rst_info();
    if (info->reason == REASON_WDT_RST ||
        info->reason == REASON_EXCEPTION_RST ||
        info->reason == REASON_SOFT_WDT_RST)
    {
      return "reason=" + String(info->reason) +
          ", exccause=0x" + String(info->exccause, HEX) +
          ", excvaddr=0x" + String(info->excvaddr, HEX) +
          ", epc1=0x" + String(info->epc1, HEX) +
          ", epc2=0x" + String(info->epc2,HEX) +
          ", epc3=0x" + String(info->epc3, HEX) +
          ", depc=0x" + String(info->depc, HEX);
    }
    return "";
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(ESP32)

RESET_REASON System::lastResetReason() {
    // Note: this is just checking for CPU core 1
    return rtc_get_reset_reason(0);
}

bool lastResetReasonIsDeepSleepAwake() {
    return System::lastResetReason() == DEEPSLEEP_RESET;
}

String System::lastException() {
    return String(); // no-op on ESP32
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(ESP8266) || defined(ESP32)

void System::wifiOn() {
    //WiFi.forceSleepWake();
    delay(1);
    WiFi.mode(WIFI_STA);
    delay(100);
}

void System::wifiOff() {
    WiFi.disconnect();
    uint16_t i = 0;
    while ((WiFi.status() == WL_CONNECTED) && (i++ < 3*10)) {
        delay(100);
    }
    WiFi.mode(WIFI_OFF);
    delay(1);
    //WiFi.forceSleepBegin();
    delay(100);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#else
///////////////////////////////////////////////////////////////////////////////////////////////////

void System::wifiOn() {
}

void System::wifiOff() {
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

[[ noreturn ]] void System::panic() {
    #if defined(ESP8266)
    // disable software watchdog
    ESP.wdtDisable();

    #elif defined(ESP32)
    // set watchdog to panic after ten seconds
    esp_task_wdt_init(10, true);
    // add current thread to watchdog
    esp_task_wdt_add(NULL);

    #endif

    // hardware watchdog resets
    while (1){};
}