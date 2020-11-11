#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <Arduino.h>

class Switch {
    class Implementation;

public:
    enum class PullUp : bool { Disable, Enable };

    Switch(uint8_t pin, PullUp enablePullUp, uint32_t debounceTime);
    ~Switch();

    bool begin(void);
    bool read(void);

    bool isClosed(void);
    bool isOpen(void);

private:
    Switch(const Switch&);
    Switch& operator=(const Switch&);

    Implementation *impl;
};

#endif
