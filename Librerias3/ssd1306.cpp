#include "ssd1306.h"
#include <cstring>

// Constructor
SSD1306::SSD1306(I2C &i2c) : _i2c(i2c) {}

// Enviar comando a la pantalla OLED
void SSD1306::sendCommand(uint8_t cmd) {
    char data[2];
    data[0] = 0x00;  // Control byte para comandos
    data[1] = cmd;
    _i2c.write(SSD1306_ADDR, data, 2);
}

// Enviar datos a la pantalla OLED
void SSD1306::sendData(const uint8_t* data, int length) {
    char buffer[length + 1];
    buffer[0] = 0x40;  // Control byte para datos
    memcpy(&buffer[1], data, length);
    _i2c.write(SSD1306_ADDR, buffer, length + 1);
}

// Inicialización del OLED con los comandos necesarios
void SSD1306::init() {
    sendCommand(0xAE);  // Apagar la pantalla
    sendCommand(0xAF);  // Encender la pantalla
    sendCommand(0xD5);  // Set Display Clock Divide Ratio / Oscillator Frequency
    sendCommand(0x80);  // Ratio de oscilación
    sendCommand(0xA8);  // Set Multiplex Ratio
    sendCommand(0x3F);  // 1/64 duty
    sendCommand(0xD3);  // Set Display Offset
    sendCommand(0x00);  // No offset
    sendCommand(0x40);  // Set Start Line at 0
    sendCommand(0x8D);  // Enable Charge Pump
    sendCommand(0x14);  // Activar Charge Pump
    sendCommand(0x20);  // Set Memory Addressing Mode
    sendCommand(0x00);  // Horizontal Addressing Mode
    sendCommand(0xA1);  // Set Segment Re-map (columna 127 es segmento 0)
    sendCommand(0xC8);  // Set COM Output Scan Direction (Normal)
    sendCommand(0xDA);  // Set COM Pins hardware configuration
    sendCommand(0x12);
    sendCommand(0x81);  // Set Contrast Control
    sendCommand(0xCF);  // Contraste
    sendCommand(0xD9);  // Set Pre-charge Period
    sendCommand(0xF1);  
    sendCommand(0xDB);  // Set VCOMH Deselect Level
    sendCommand(0x40);  
    sendCommand(0xA4);  // Resume to RAM content display
    sendCommand(0xA6);  // Normal display (A7 para invertir colores)
    sendCommand(0xAF);  // Encender la pantalla
}

// Limpiar la pantalla OLED
void SSD1306::clearDisplay() {
    uint8_t clearData[128];  // Ancho de la pantalla es 128
    memset(clearData, 0, sizeof(clearData));  // Llenar con ceros

    for (int page = 0; page < 8; page++) {  // Pantalla de 8 páginas
        sendCommand(0xB0 + page);  // Seleccionar página
        sendCommand(0x00);         // Columna baja
        sendCommand(0x10);         // Columna alta
        sendData(clearData, sizeof(clearData));  // Enviar fila de píxeles apagados
    }
}

// Mostrar texto en la pantalla
void SSD1306::displayText(const char* text, int line) {
    if (line < 0 || line >= 8) return;  // Verificar que la línea sea válida

    sendCommand(0xB0 + line);  // Establecer la página (línea)
    sendCommand(0x00);         // Columna baja a 0
    sendCommand(0x10);         // Columna alta a 0

    int i = 0;
    while (text[i] != '\0' && i < 21) {  // 21 caracteres máximo por línea
        sendChar(text[i]);
        i++;
    }
}

void SSD1306::sendChar(char c) {
    uint8_t charData[6];  // 5 columnas de datos + 1 columna de espacio
    getCharData(c, charData);
    sendData(charData, 6);
}

// Mapa de bits para caracteres simples (se pueden añadir más caracteres)
void SSD1306::getCharData(char c, uint8_t* charData) {
    // Fuente de 5x7 para caracteres desde ' ' hasta 'Z'
    static const uint8_t font[][5] = {
        {0x00, 0x00, 0x00, 0x00, 0x00},  // Espacio
        {0x00, 0x06, 0x5F, 0x06, 0x00},  // !
        {0x07, 0x00, 0x07, 0x00, 0x00},  // "
        {0x14, 0x7F, 0x14, 0x7F, 0x14},  // #
        {0x24, 0x2A, 0x7F, 0x2A, 0x12},  // $
        {0x23, 0x13, 0x08, 0x64, 0x62},  // %
        {0x36, 0x49, 0x55, 0x22, 0x50},  // &
        {0x00, 0x05, 0x03, 0x00, 0x00},  // '
        {0x00, 0x1C, 0x22, 0x41, 0x00},  // (
        {0x00, 0x41, 0x22, 0x1C, 0x00},  // )
        {0x14, 0x08, 0x3E, 0x08, 0x14},  // *
        {0x08, 0x08, 0x3E, 0x08, 0x08},  // +
        {0x00, 0x50, 0x30, 0x00, 0x00},  // ,
        {0x08, 0x08, 0x08, 0x08, 0x08},  // -
        {0x00, 0x60, 0x60, 0x00, 0x00},  // .
        {0x20, 0x10, 0x08, 0x04, 0x02},  // /
        {0x3E, 0x51, 0x49, 0x45, 0x3E},  // 0
        {0x00, 0x42, 0x7F, 0x40, 0x00},  // 1
        {0x42, 0x61, 0x51, 0x49, 0x46},  // 2
        {0x21, 0x41, 0x45, 0x4B, 0x31},  // 3
        {0x18, 0x14, 0x12, 0x7F, 0x10},  // 4
        {0x27, 0x45, 0x45, 0x45, 0x39},  // 5
        {0x3C, 0x4A, 0x49, 0x49, 0x30},  // 6
        {0x01, 0x71, 0x09, 0x05, 0x03},  // 7
        {0x36, 0x49, 0x49, 0x49, 0x36},  // 8
        {0x06, 0x49, 0x49, 0x29, 0x1E},  // 9
        {0x00, 0x36, 0x36, 0x00, 0x00},  // :
        {0x00, 0x56, 0x36, 0x00, 0x00},  // ;
        {0x08, 0x14, 0x22, 0x41, 0x00},  // <
        {0x14, 0x14, 0x14, 0x14, 0x14},  // =
        {0x00, 0x41, 0x22, 0x14, 0x08},  // >
        {0x02, 0x01, 0x51, 0x09, 0x06},  // ?
        {0x32, 0x49, 0x79, 0x41, 0x3E},  // @
        {0x7E, 0x11, 0x11, 0x11, 0x7E},  // A
        {0x7F, 0x49, 0x49, 0x49, 0x36},  // B
        {0x3E, 0x41, 0x41, 0x41, 0x22},  // C
        {0x7F, 0x41, 0x41, 0x22, 0x1C},  // D
        {0x7F, 0x49, 0x49, 0x49, 0x41},  // E
        {0x7F, 0x09, 0x09, 0x09, 0x01},  // F
        {0x3E, 0x41, 0x49, 0x49, 0x7A},  // G
        {0x7F, 0x08, 0x08, 0x08, 0x7F},  // H
        {0x00, 0x41, 0x7F, 0x41, 0x00},  // I
        {0x20, 0x40, 0x41, 0x3F, 0x01},  // J
        {0x7F, 0x08, 0x14, 0x22, 0x41},  // K
        {0x7F, 0x40, 0x40, 0x40, 0x40},  // L
        {0x7F, 0x02, 0x0C, 0x02, 0x7F},  // M
        {0x7F, 0x04, 0x08, 0x10, 0x7F},  // N
        {0x3E, 0x41, 0x41, 0x41, 0x3E},  // O
        {0x7F, 0x09, 0x09, 0x09, 0x06},  // P
        {0x3E, 0x41, 0x51, 0x21, 0x5E},  // Q
        {0x7F, 0x09, 0x19, 0x29, 0x46},  // R
        {0x46, 0x49, 0x49, 0x49, 0x31},  // S
        {0x01, 0x01, 0x7F, 0x01, 0x01},  // T
        {0x3F, 0x40, 0x40, 0x40, 0x3F},  // U
        {0x1F, 0x20, 0x40, 0x20, 0x1F},  // V
        {0x3F, 0x40, 0x38, 0x40, 0x3F},  // W
        {0x63, 0x14, 0x08, 0x14, 0x63},  // X
        {0x07, 0x08, 0x70, 0x08, 0x07},  // Y
        {0x61, 0x51, 0x49, 0x45, 0x43},  // Z
    };
    
    if (c >= ' ' && c <= 'Z') {
        memcpy(charData, font[c - ' '], 5);
        charData[5] = 0x00;  // Añadir una columna de espacio
    } else if (c >= 'a' && c <= 'z') {
        // Convertir minúsculas a mayúsculas
        memcpy(charData, font[c - 'a' + ('A' - ' ')], 5);
        charData[5] = 0x00;
    } else {
        // Para caracteres desconocidos, usar un espacio
        memset(charData, 0, 6);
    }
}