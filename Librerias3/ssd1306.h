#ifndef SSD1306_H
#define SSD1306_H

#include "mbed.h"

class SSD1306 {
public:
    SSD1306(I2C &i2c);
    void init();
    void clearDisplay();
    void displayText(const char* text, int line);

private:
    I2C &_i2c;
    static const int SSD1306_ADDR = 0x3C << 1;  // Dirección I2C del OLED
    void sendCommand(uint8_t cmd);
    void sendData(const uint8_t* data, int length);
    void getCharData(char c, uint8_t* charData);
    void sendChar(char c);
};

#endif