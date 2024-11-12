#include "mbed.h"
#include "ssd1306.h"
#include "tm1638.h"
#include "si7021.h"
#include <vector>
#include <algorithm>

// Definición de pines
#define LM35_PIN A2
#define RESISTIVE_PIN A3
#define SI7021_SDA_PIN D3
#define SI7021_SCL_PIN D6
#define TM1638_DIO_PIN D7
#define TM1638_CLK_PIN D8
#define TM1638_STB_PIN D9

// Constantes del sistema
const int NUM_MUESTRAS = 10;
const int LM35_MUESTRAS = 4;
const int SI7021_MUESTRAS = 4;
const int RESISTIVE_MUESTRAS = 2;
const int TEMP_REFERENCIA = 2000;  // 20.00°C
const int CALIBRACION = 100;       // Factor de calibración (1.00)
const int UMBRAL_RUIDO = 10;       // Umbral para ruido (0.10V * 100)

// Constantes para el termistor NTC
const float R_REFERENCIA = 10000.0;  // 10k ohm
const float BETA = 3950.0;           // Valor típico para NTC
const float T_KELVIN = 273.15;       // Offset para convertir a Kelvin

// Objetos de sensores
AnalogIn lm35(LM35_PIN);
AnalogIn resistiveSensor(RESISTIVE_PIN);
I2C i2c(I2C_SDA, I2C_SCL);
Si7021 si7021(SI7021_SDA_PIN, SI7021_SCL_PIN);
SSD1306 oled(i2c);
TM1638 display(TM1638_DIO_PIN, TM1638_CLK_PIN, TM1638_STB_PIN);

struct Temperatura {
    int entero;
    int decimal;
};

std::vector<Temperatura> medidas(NUM_MUESTRAS);
Temperatura promedio, mediana;
int errorAbsoluto, errorRelativo;
bool medicionCompleta = false;

// Funciones de lectura de sensores
Temperatura leerTemperaturaLM35() {
    int lectura = lm35.read_u16();
    int voltaje = (lectura * 3300) / 65535;
    int temp_raw = (voltaje * 100) / 10;
    temp_raw = (temp_raw * CALIBRACION) / 100;
    
    Temperatura t;
    t.entero = abs(temp_raw / 100);
    t.decimal = abs(temp_raw % 100);
    return t;
}

Temperatura leerTemperaturaResistiva() {
    // Leemos el voltaje del ADC y convertimos a voltios
    float Vout = resistiveSensor.read() * 3.3f;
    
    // Calculamos la resistencia del NTC usando divisor de voltaje
    // Si el NTC está conectado a tierra (GND)
    float Rt = R_REFERENCIA * (3.3f - Vout) / Vout;
    
    // Constantes para el cálculo
    const float T0 = 298.15f;    // 25°C en Kelvin
    const float R0 = 10000.0f;   // Resistencia a 25°C (10k)
    
    // Cálculo de temperatura usando ecuación beta
    float lnRt = log(Rt / R0);
    float tempK = 1.0f / ((1.0f / T0) + (1.0f / BETA) * lnRt);
    float tempC = tempK - 273.15f;
    
    // Verificación de rango razonable
    if (tempC < -10.0f || tempC > 85.0f) {
        tempC = 25.0f;  // Valor por defecto si está fuera de rango
    }
    
    // Convertimos a centésimas para el formato de display
    // Multiplicamos por 100 para preservar 2 decimales
    int temp_centesimas = (int)(tempC * 100.0f);
    
    Temperatura t;
    // La parte entera son los grados
    t.entero = temp_centesimas / 100;
    // Los decimales son el resto
    t.decimal = abs(temp_centesimas % 100);
    
    // Debug - imprimir valores para verificación
    printf("Vout: %.3f V, Rt: %.0f ohm, Temp: %.2f°C\n", 
           Vout, Rt, tempC);
    
    return t;
}

Temperatura leerTemperaturaSI7021() {
    int temp_raw = si7021.readTemperature() * 100;
    
    Temperatura t;
    t.entero = abs(temp_raw / 100);
    t.decimal = abs(temp_raw % 100);
    return t;
}

void mostrarEnOLED(const char* linea1, Temperatura valor, const char* unidad, int fila) {
    char linea[32];
    snprintf(linea, sizeof(linea), "%s%d.%02d %s", linea1, valor.entero, valor.decimal, unidad);
    oled.displayText(linea, fila);
}

void mostrarValorTM1638(Temperatura valor) {
    display.clearDisplay();
    if (valor.entero < 0) {
        display.displayDigit(0, 0xF, false);
        display.displayDigit(1, abs(valor.entero) % 10, true);
    } else {
        display.displayDigit(0, valor.entero / 10, false);
        display.displayDigit(1, valor.entero % 10, true);
    }
    display.displayDigit(2, (valor.decimal / 10) % 10, false);
    display.displayDigit(3, valor.decimal % 10, false);
}

Temperatura calcularPromedio(const std::vector<Temperatura>& arr) {
    long long suma = 0;
    for (const auto& t : arr) {
        suma += (t.entero * 100 + t.decimal);
    }
    
    int promedio_total = suma / arr.size();
    Temperatura prom;
    prom.entero = abs(promedio_total / 100);
    prom.decimal = abs(promedio_total % 100);
    return prom;
}

void burbuja(std::vector<Temperatura>& arr) {
    for (size_t i = 0; i < arr.size() - 1; i++) {
        for (size_t j = 0; j < arr.size() - i - 1; j++) {
            int valor1 = arr[j].entero * 100 + arr[j].decimal;
            int valor2 = arr[j + 1].entero * 100 + arr[j + 1].decimal;
            if (valor1 > valor2) {
                std::swap(arr[j], arr[j + 1]);
            }
        }
    }
}

Temperatura calcularMediana(std::vector<Temperatura> arr) {
    burbuja(arr);
    return arr[arr.size() / 2];
}

void calcularErrores(Temperatura medida, int referencia) {
    // Convertir ambos valores a centésimas de grado
    int valor_medido = medida.entero * 100 + medida.decimal;
    
    // Error absoluto en centésimas de grado
    errorAbsoluto = abs(valor_medido - referencia);
    
    // Error relativo en porcentaje con dos decimales
    // Multiplicamos por 100 para obtener porcentaje
    errorRelativo = (errorAbsoluto * 100) / referencia;
}

Temperatura convertirPorcentajeADisplay(int valor) {
    Temperatura t;
    t.entero = abs(valor / 100);
    t.decimal = abs(valor % 100);
    return t;
}

void manejarBotones() {
    uint8_t botones = display.readButtons();

    if (botones & 0x01) {
        mostrarValorTM1638(promedio);
        ThisThread::sleep_for(500ms);
    }
    if (botones & 0x02) {
        mostrarValorTM1638(mediana);
        ThisThread::sleep_for(500ms);
    }
    if (botones & 0x04) {
        Temperatura errorAbs;
        errorAbs.entero = errorAbsoluto / 100;
        errorAbs.decimal = errorAbsoluto % 100;
        mostrarValorTM1638(errorAbs);
        ThisThread::sleep_for(500ms);
    }
    if (botones & 0x08) {
        mostrarValorTM1638(convertirPorcentajeADisplay(errorRelativo));
        ThisThread::sleep_for(500ms);
    }
    if (botones & 0x10) {
        medicionCompleta = false;
        ThisThread::sleep_for(500ms);
    }
}

int main() {
    oled.init();
    oled.clearDisplay();
    
    oled.displayText("Sistema de", 0);
    ThisThread::sleep_for(200ms);
    oled.displayText("Medicion de", 2);
    ThisThread::sleep_for(200ms);
    oled.displayText("Temperatura", 4);
    ThisThread::sleep_for(3000ms);
    
    display.init();
    display.setBrightness(7);

    while (true) {
        if (!medicionCompleta) {
            int idx = 0;
            
            oled.clearDisplay();
            oled.displayText("Midiendo LM35...", 0);
            for (int i = 0; i < LM35_MUESTRAS; i++, idx++) {
                medidas[idx] = leerTemperaturaLM35();
                mostrarValorTM1638(medidas[idx]);
                mostrarEnOLED("LM35: ", medidas[idx], "C", 2);
                ThisThread::sleep_for(2s);
            }
            
            oled.clearDisplay();
            oled.displayText("Midiendo SI7021...", 0);
            for (int i = 0; i < SI7021_MUESTRAS; i++, idx++) {
                medidas[idx] = leerTemperaturaSI7021();
                mostrarValorTM1638(medidas[idx]);
                mostrarEnOLED("SI7021: ", medidas[idx], "C", 2);
                ThisThread::sleep_for(2s);
            }
            
            oled.clearDisplay();
            oled.displayText("Midiendo Resistivo...", 0);
            for (int i = 0; i < RESISTIVE_MUESTRAS; i++, idx++) {
                medidas[idx] = leerTemperaturaResistiva();
                mostrarValorTM1638(medidas[idx]);
                mostrarEnOLED("Resistivo: ", medidas[idx], "C", 2);
                ThisThread::sleep_for(2s);
            }

            promedio = calcularPromedio(medidas);
            mediana = calcularMediana(medidas);
            calcularErrores(promedio, TEMP_REFERENCIA);
            
            medicionCompleta = true;

            oled.clearDisplay();
            oled.displayText("Medicion", 0);
            oled.displayText("Completada!", 2);
            ThisThread::sleep_for(3s);
        }

        oled.clearDisplay();
        mostrarEnOLED("Prom: ", promedio, "C", 0);
        mostrarEnOLED("Med: ", mediana, "C", 2);
        
        // Mostrar error absoluto en °C
        Temperatura errorAbsTemp;
        errorAbsTemp.entero = errorAbsoluto / 100;
        errorAbsTemp.decimal = errorAbsoluto % 100;
        mostrarEnOLED("Err Abs: ", errorAbsTemp, "C", 4);
        
        // Mostrar error relativo en porcentaje
        Temperatura errorRelTemp;
        errorRelTemp.entero = errorRelativo / 100;
        errorRelTemp.decimal = errorRelativo % 100;
        mostrarEnOLED("Err Rel: ", errorRelTemp, "%", 6);

        for (int i = 0; i < 50; i++) {
            manejarBotones();
            ThisThread::sleep_for(100ms);
        }
    }
}