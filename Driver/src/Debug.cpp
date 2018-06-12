#include <Arduino.h>

#include "Debug.h"

extern "C" {
    #include "user_interface.h"
}

uint32 Debug::lastResetReason() {
    struct rst_info *info = system_get_rst_info();
    return info->reason;
}

String Debug::lastException() {
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
