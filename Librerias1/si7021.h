#ifndef SI7021_H
#define SI7021_H

#include "mbed.h"

class Si7021 {
public:
    Si7021(PinName sda, PinName scl);
    
    float readTemperature();
    float readHumidity();

private:
    I2C i2c;
    const int SI7021_ADDR = 0x40 << 1;
    
    uint16_t readMeasurement(uint8_t command);
    void writeCommand(uint8_t command);
    uint16_t readData();
    void reset();
};

#endif