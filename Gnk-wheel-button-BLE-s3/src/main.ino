#include <Arduino.h>
#include <BleGamepad.h>

// Levas
#define X_BUTTON 4           // Botón A
#define CIRCLE_BUTTON 5      // Botón B
#define TRIANGLE_BUTTON 15   // Botón Y
#define SQUARE_BUTTON 16     // Botón X

// Sensores Hall - Embrague-freno de mano
#define ACCELERATOR 6        // Controla el acelerador (Trigger R2)
#define BRAKE 7              // Controla el freno (Trigger L2)

// Número de botones individuales
#define NUM_BUTTONS 4
int buttonsPins[NUM_BUTTONS] = {X_BUTTON, CIRCLE_BUTTON, TRIANGLE_BUTTON, SQUARE_BUTTON};
int PCGamepadButtons[NUM_BUTTONS] = {1, 2, 4, 3}; // Correspondencia de botones individuales

// Definición de la matriz de botones (5x2) Botones Individuales Volante
#define NUM_ROWS 5
#define NUM_COLS 2
int rowsPins[NUM_ROWS] = {17, 18, 19, 21, 22}; // Pines de las filas
int colsPins[NUM_COLS] = {23, 25};             // Pines de las columnas

// Correspondencia con botones del gamepad
int buttonMapping[NUM_ROWS][NUM_COLS] = {
    {5, 6},
    {7, 8},
    {9, 10},
    {11, 12},
    {13, 14}
};

// Rotarys volante
#define NUM_ROTARYS 4
#define ROTARY_PIN_A1 26
#define ROTARY_PIN_B1 27
#define ROTARY_BUTTON_1 28

#define ROTARY_PIN_A2 32
#define ROTARY_PIN_B2 33
#define ROTARY_BUTTON_2 34

#define ROTARY_PIN_A3 35
#define ROTARY_PIN_B3 36
#define ROTARY_BUTTON_3 37

#define ROTARY_PIN_A4 38
#define ROTARY_PIN_B4 39
#define ROTARY_BUTTON_4 40

int rotaryButtonPins[NUM_ROTARYS] = {ROTARY_BUTTON_1, ROTARY_BUTTON_2, ROTARY_BUTTON_3, ROTARY_BUTTON_4};

// Funky Switch 7 en 1
#define NUM_FUNKY_SWITCH 2

// Funky Switch 1
#define FUNKY1_UP 41
#define FUNKY1_DOWN 42
#define FUNKY1_LEFT 43
#define FUNKY1_RIGHT 44
#define FUNKY1_CENTER 45
#define FUNKY1_EXTRA1 46
#define FUNKY1_EXTRA2 47

// Funky Switch 2
#define FUNKY2_UP 48
#define FUNKY2_DOWN 49
#define FUNKY2_LEFT 50
#define FUNKY2_RIGHT 51
#define FUNKY2_CENTER 52
#define FUNKY2_EXTRA1 53
#define FUNKY2_EXTRA2 54

int funkySwitchPins[NUM_FUNKY_SWITCH][7] = {
    {FUNKY1_UP, FUNKY1_DOWN, FUNKY1_LEFT, FUNKY1_RIGHT, FUNKY1_CENTER, FUNKY1_EXTRA1, FUNKY1_EXTRA2},
    {FUNKY2_UP, FUNKY2_DOWN, FUNKY2_LEFT, FUNKY2_RIGHT, FUNKY2_CENTER, FUNKY2_EXTRA1, FUNKY2_EXTRA2}
};

// Variables para sensores Hall
uint16_t acceleratorLecture = 0;
uint16_t brakeLecture = 0;
uint16_t acceleratorValue = 0;
uint16_t brakeValue = 0;

// Configuración del BLE Gamepad
BleGamepad bleGamepad("Gnk Shifter", "Gonky Racing");
BleGamepadConfiguration bleGamepadConfig;

// Función para leer valores filtrados de pines analógicos
uint16_t readFilteredAnalog(int pin, int samples = 10) {
    uint32_t sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delay(2); // Pequeño retraso entre lecturas para estabilidad
    }
    return sum / samples;
}

void joysticksHandlerForPC(uint16_t accelerator, uint16_t brake) {
    bleGamepad.setRZ(accelerator); // Acelerador (Trigger R2)
    bleGamepad.setZ(brake);        // Freno (Trigger L2)
}

void setup() {
    delay(20);
    Serial.begin(115200);

    // Configuración de pines para botones individuales
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(buttonsPins[i], INPUT_PULLUP);
    }

    // Configuración de pines para la matriz de botones
    for (int i = 0; i < NUM_ROWS; i++) {
        pinMode(rowsPins[i], OUTPUT);
        digitalWrite(rowsPins[i], HIGH); // Mantener filas desactivadas
    }
    for (int i = 0; i < NUM_COLS; i++) {
        pinMode(colsPins[i], INPUT_PULLUP); // Columnas como entradas con pull-up
    }

    // Configuración de pines para sensores Hall
    pinMode(ACCELERATOR, INPUT);
    pinMode(BRAKE, INPUT);

    // Configuración de pines para encoders rotatorios y sus pulsadores
    pinMode(ROTARY_PIN_A1, INPUT_PULLUP);
    pinMode(ROTARY_PIN_B1, INPUT_PULLUP);
    pinMode(ROTARY_BUTTON_1, INPUT_PULLUP);

    pinMode(ROTARY_PIN_A2, INPUT_PULLUP);
    pinMode(ROTARY_PIN_B2, INPUT_PULLUP);
    pinMode(ROTARY_BUTTON_2, INPUT_PULLUP);

    pinMode(ROTARY_PIN_A3, INPUT_PULLUP);
    pinMode(ROTARY_PIN_B3, INPUT_PULLUP);
    pinMode(ROTARY_BUTTON_3, INPUT_PULLUP);

    pinMode(ROTARY_PIN_A4, INPUT_PULLUP);
    pinMode(ROTARY_PIN_B4, INPUT_PULLUP);
    pinMode(ROTARY_BUTTON_4, INPUT_PULLUP);

    // Configuración de pines para Funky Switch 7 en 1
    for (int i = 0; i < NUM_FUNKY_SWITCH; i++) {
        for (int j = 0; j < 7; j++) {
            pinMode(funkySwitchPins[i][j], INPUT_PULLUP);
        }
    }

    // Configuración del BLE Gamepad
    bleGamepadConfig.setAutoReport(false);
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    bleGamepadConfig.setVid(0xe502);
    bleGamepadConfig.setPid(0xabcd);
    bleGamepadConfig.setHatSwitchCount(0); // No usamos hat-switches
    bleGamepad.begin(&bleGamepadConfig);
}

void loop() {
    if (bleGamepad.isConnected()) {
        // Lectura de botones individuales
        for (int i = 0; i < NUM_BUTTONS; i++) {
            if (!digitalRead(buttonsPins[i])) {
                bleGamepad.press(PCGamepadButtons[i]);
            } else {
                bleGamepad.release(PCGamepadButtons[i]);
            }
        }

        // Lectura de la matriz de botones
        for (int row = 0; row < NUM_ROWS; row++) {
            digitalWrite(rowsPins[row], LOW); // Activar fila actual
            delayMicroseconds(50); // Pequeño retraso para estabilidad

            for (int col = 0; col < NUM_COLS; col++) {
                int buttonID = buttonMapping[row][col];
                if (!digitalRead(colsPins[col])) {
                    bleGamepad.press(buttonID); // Botón presionado
                } else {
                    bleGamepad.release(buttonID); // Botón liberado
                }
            }

            digitalWrite(rowsPins[row], HIGH); // Desactivar fila actual
        }

        // Lectura de sensores Hall con filtrado
        acceleratorLecture = readFilteredAnalog(ACCELERATOR);
        brakeLecture = readFilteredAnalog(BRAKE);

        // Mostrar valores en el monitor serie
        Serial.print("Acelerador: ");
        Serial.println(acceleratorLecture);
        Serial.print("Freno: ");
        Serial.println(brakeLecture);

        // Mapeo de valores para BLE Gamepad
        acceleratorValue = map(acceleratorLecture, 0, 4095, 0, 32767); // Acelerador (Trigger R2)
        brakeValue = map(brakeLecture, 0, 4095, 0, 32767);             // Freno (Trigger L2)

        // Actualización de los ejes del gamepad
        joysticksHandlerForPC(acceleratorValue, brakeValue);

        // Enviar reporte BLE
        bleGamepad.sendReport();

        // Lectura de pulsadores de encoders rotatorios
        for (int i = 0; i < NUM_ROTARYS; i++) {
            if (!digitalRead(rotaryButtonPins[i])) {
                bleGamepad.press(i + 15); // Asigna un ID único para cada pulsador
            } else {
                bleGamepad.release(i + 15); // Libera el ID único cuando no está presionado
            }
        }

           // Lectura de Funky Switch 7 en 1
        for (int i = 0; i < NUM_FUNKY_SWITCH; i++) {
            for (int j = 0; j < 7; j++) {
                if (!digitalRead(funkySwitchPins[i][j])) {
                    bleGamepad.press(i * 7 + j + 30); // Asigna un ID único para cada función del Funky Switch
                } else {
                    bleGamepad.release(i * 7 + j + 30); // Libera el ID único cuando no está presionado
                }
            }
        }

        // Retraso breve para evitar saturación BLE
        delay(10);
    }
}
