#include <Arduino.h>
#include "AiEsp32RotaryEncoder.h"
#include <BleGamepad.h>
#include <functional>

#define ENCODER_CONFIG_UNSET 0
#define ENCODER_CONFIG_ROTATION_AS_BUTTON 1
#define ENCODER_CONFIG_ROTATION_AS_AXIS 2

/**
 * Rotations as axis values
 */
class RotaryAxisEncoderController {
public:
  RotaryAxisEncoderController(
      AiEsp32RotaryEncoder& encoder,
      BleGamepad& gamepad,
      std::function<void(BleGamepadConfiguration&)> configCallback,
      std::function<void(BleGamepad&, int32_t)> updateCallback
      ) :
      encoder(encoder),
      gamepad(gamepad),
      configCallback(std::move(configCallback)),
      updateCallback(std::move(updateCallback))
      {}
  
  void setup() {
    encoder.setBoundaries(0, 8190, false);
    encoder.setAcceleration(250);
  }

  void config(BleGamepadConfiguration& config) {
    configCallback(config);
  }

  bool loop() {
    if (encoder.encoderChanged())
    {
#if DEBUG
      Serial.println("changed");
#endif

      int32_t encoderValue = encoder.readEncoder();
      updateCallback(gamepad, encoderValue);

#if DEBUG
      Serial.print("encoder: ");
      Serial.println(encoderValue);
#endif
      return true;
    }
    return false;
  }

private:
  AiEsp32RotaryEncoder& encoder;
  BleGamepad& gamepad;
  std::function<void(BleGamepadConfiguration&)> configCallback;
  std::function<void(BleGamepad&, int32_t)> updateCallback;
};

/**
 * Rotations as button presses
 */
class RotaryButtonEncoderController {
public:
  RotaryButtonEncoderController(
      AiEsp32RotaryEncoder& encoder,
      BleGamepad& gamepad,
      uint8_t ccwButtonNumber,
      uint8_t cwButtonNumber,
      uint32_t pressDurationMs,
      uint32_t threshold
      ) :
      encoder(encoder),
      ccwButtonNumber(ccwButtonNumber),
      cwButtonNumber(cwButtonNumber),
      pressDurationMs(pressDurationMs),
      threshold(threshold),
      gamepad(gamepad)
      {}

  void setup() {
    encoder.setBoundaries(threshold * -2, threshold * 2, false);
    encoder.disableAcceleration();
  }

  void config(BleGamepadConfiguration& config) {

  }

  bool loop() {
    if (encoder.encoderChanged()) {
      int32_t encoderValue = encoder.readEncoder();
#if DEBUG
      Serial.print("encoder: ");
      Serial.println(String(encoderValue));
      Serial.print("threshold: ");
      Serial.println(String(threshold));
      Serial.print("threshold *-1: ");
      Serial.println(String(threshold * -1));
      Serial.print("encoder value >= threshold : ");
      Serial.println(String(encoderValue >= threshold));
      Serial.print("encoder value <= threshold : ");
      Serial.println(String(encoderValue <= threshold * -1));
#endif

      if (encoderValue >= threshold) {
        handleButtonPress(cwButtonNumber);
        resetEncoder();
      } else if (encoderValue <= threshold * -1) {
        handleButtonPress(ccwButtonNumber);
        resetEncoder();
      }
    }
    return false;
  }

  void resetEncoder() {
    encoder.setEncoderValue(0);
  }

private:
  AiEsp32RotaryEncoder& encoder;
  uint8_t ccwButtonNumber;
  uint8_t cwButtonNumber;
  uint32_t pressDurationMs;
  int32_t threshold;
  BleGamepad& gamepad;

  void handleButtonPress(uint8_t buttonNumber) {
    gamepad.press(buttonNumber);
    gamepad.sendReport();
    // todo: schedule a delayed task and release without delaying the main loop
    //  although this delay is not blocking the WDT in the esp32 at least
    // https://www.esp32.com/viewtopic.php?p=10261#p10261
    vTaskDelay(pressDurationMs / portTICK_PERIOD_MS);
    gamepad.release(buttonNumber);
    gamepad.sendReport();
#if DEBUG
        Serial.print("button press: ");
        Serial.println(buttonNumber);
#endif
  }
};