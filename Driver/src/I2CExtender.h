#ifndef __I2C_EXTENDER_H__
#define __I2C_EXTENDER_H__

///////////////////////////////////////////////////////////////////////////////////////////////////
// Weather Station:
// Class to operate the I2C extender module of the Weather Station. See schematics.
///////////////////////////////////////////////////////////////////////////////////////////////////

class I2CExtender {
public:
    I2CExtender(int enablepin);

    bool begin();

    // Activates the I2C extender module by driving the enable pin.
    bool activate(void);

    // Deactivates the I2C extender module by deactivating the enable pin.
    void deactivate(void);

private:
    int enablepin;
};

#endif
