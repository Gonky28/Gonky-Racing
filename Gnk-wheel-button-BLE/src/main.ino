#include <Arduino.h>
#include <BleGamepad.h>

// Levas
#define X_BUTTON 4         // A
#define CIRCLE_BUTTON 5    // B
#define TRIANGLE_BUTTON 15  // Y
#define SQUARE_BUTTON 16   // X

// SENSORES HALL
#define ACCELERATOR 6  // Controla el acelerador (Trigger R2)
#define BRAKE 7  // Controla el freno (Trigger L2)
#define NUM_BUTTONS 4

int buttonsPins[NUM_BUTTONS] = {X_BUTTON, CIRCLE_BUTTON, TRIANGLE_BUTTON, SQUARE_BUTTON};
int PCGamepadButtons[NUM_BUTTONS] = {1, 2, 4, 3};

uint16_t acceleratorLecture = 0;
uint16_t brakeLecture = 0;
uint16_t acceleratorValue = 0;
uint16_t brakeValue = 0;

BleGamepad bleGamepad("Gnk Shifter", "Gonky Racing");
BleGamepadConfiguration bleGamepadConfig;

void setup() {
    delay(20);
    Serial.begin(115200);
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(buttonsPins[i], INPUT_PULLUP);
    }
    pinMode(ACCELERATOR, INPUT);
    pinMode(BRAKE, INPUT);
    bleGamepadConfig.setAutoReport(false);
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    bleGamepadConfig.setVid(0xe502);
    bleGamepadConfig.setPid(0xabcd);
    bleGamepadConfig.setHatSwitchCount(4);
    bleGamepad.begin(&bleGamepadConfig);
}

void loop() {
    if (bleGamepad.isConnected()) {
        // Lectura de botones
        for (int i = 0; i < NUM_BUTTONS; i++) {
            if (!digitalRead(buttonsPins[i])) {
                bleGamepad.press(PCGamepadButtons[i]);
            } else {
                bleGamepad.release(PCGamepadButtons[i]);
            }
        }

        // Lectura de sensores Hall
        acceleratorLecture = analogRead(ACCELERATOR);
        delay(30); // Pequeño retraso para estabilidad
        brakeLecture = analogRead(BRAKE);
        delay(30); // Pequeño retraso para estabilidad

        Serial.print("Acelerador: ");
        Serial.println(acceleratorLecture); // Mostrar el valor del acelerador en el monitor serie
        Serial.print("Freno: ");
        Serial.println(brakeLecture); // Mostrar el valor del freno en el monitor serie

        // Mapeo de valores de los sensores Hall
        acceleratorValue = map(acceleratorLecture, 0, 4095, 0, 32767); // Acelerador (Trigger R2)
        brakeValue = map(brakeLecture, 0, 4095, 0, 32767); // Freno (Trigger L2)

        joysticksHandlerForPC(acceleratorValue, brakeValue);
        bleGamepad.sendReport();
        delay(10); // Pequeño retraso para evitar saturar el BLE
    }
}

void joysticksHandlerForPC(uint16_t accelerator, uint16_t brake) {
    bleGamepad.setRZ(accelerator); // Acelerador (Trigger R2)
    bleGamepad.setZ(brake);        // Freno (Trigger L2)
}