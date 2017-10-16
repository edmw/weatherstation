
// Macros for printing to the serial console.

#define SERIAL_OUT 1

#if SERIAL_OUT
#define SERIAL_BEGIN() { delay(500); Serial.begin(9600); while (!Serial); }
#define SERIAL_PRINT(s) { Serial.print(s); }
#define SERIAL_PRINTF(s, f) { Serial.print(s, f); }
#define SERIAL_PRINTB(b) { b ? Serial.print(F("TRUE")) : Serial.print(F("FALSE")); }
#define SERIAL_PRINTLN(s) { Serial.println(s); }
#define SERIAL_PRINT_BANNER(m) { Serial.println(); Serial.println(m); }
#define SERIAL_PRINT_NUMBER(m, v) { Serial.print(m); Serial.print(v); Serial.println(); }
#define SERIAL_PRINT_MILLIS(m, v) { Serial.print(m); Serial.print(v); Serial.print(F(" ms")); Serial.println(); }
#define SERIAL_PRINT_BYTE(b) { Serial.print(F("0x")); if (b < 16) Serial.print(F("0")); Serial.print(b, HEX); }
#else
#define SERIAL_BEGIN()
#define SERIAL_PRINT(s)
#define SERIAL_PRINTF(s, f)
#define SERIAL_PRINTB
#define SERIAL_PRINTLN(s)
#define SERIAL_PRINT_BANNER(m)
#define SERIAL_PRINT_NUMBER(m, v)
#define SERIAL_PRINT_MILLIS(m, v)
#define SERIAL_PRINT_BYTE(b)
#endif
