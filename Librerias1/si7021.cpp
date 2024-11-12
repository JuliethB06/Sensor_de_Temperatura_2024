#include "si7021.h"

Si7021::Si7021(PinName sda, PinName scl) : i2c(sda, scl) {
    i2c.frequency(100000);  // Set to 100kHz
    reset();
    ThisThread::sleep_for(50ms);  // Wait for reset to complete
}

float Si7021::readTemperature() {
    uint16_t rawTemp = readMeasurement(0xE3);  // Command for temperature measurement
    float temp = ((175.72 * rawTemp) / 65536.0) - 46.85;
    return temp;
}

float Si7021::readHumidity() {
    uint16_t rawHumidity = readMeasurement(0xE5);  // Command for humidity measurement
    float humidity = ((125.0 * rawHumidity) / 65536.0) - 6.0;
    return humidity;
}

uint16_t Si7021::readMeasurement(uint8_t command) {
    writeCommand(command);
    ThisThread::sleep_for(20ms);  // Wait for measurement
    return readData();
}

void Si7021::writeCommand(uint8_t command) {
    char cmd[1] = {command};
    i2c.write(SI7021_ADDR, cmd, 1);
}

uint16_t Si7021::readData() {
    char data[2];
    i2c.read(SI7021_ADDR, data, 2);
    return (data[0] << 8) | data[1];
}

void Si7021::reset() {
    char resetCmd[1] = {0xFE};
    i2c.write(SI7021_ADDR, resetCmd, 1);
}