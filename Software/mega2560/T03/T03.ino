/*
物流配送小车

主板：mega2560
电机驱动板：使用PWM输出

如何区分

*/

#include "PIDMoter.h"
#include "config.h"

PIDMoter _PIDMoter[4];

void setup() {
  Serial.begin(115200);

  _PIDMoter[0].init(-1, PIN_M0_PWM, PIN_M0_F1, PIN_M0_F2, PIN_M0_SPD1,
                    PIN_M0_SPD2);
  _PIDMoter[1].init(1, PIN_M1_PWM, PIN_M1_F1, PIN_M1_F2, PIN_M1_SPD1,
                    PIN_M1_SPD2);
  _PIDMoter[2].init(-1, PIN_M2_PWM, PIN_M2_F1, PIN_M2_F2, PIN_M2_SPD1,
                    PIN_M2_SPD2);
  _PIDMoter[3].init(1, PIN_M3_PWM, PIN_M3_F1, PIN_M3_F2, PIN_M3_SPD1,
                    PIN_M3_SPD2);

  // 使用外部中断计数
  attachInterrupt(digitalPinToInterrupt(PIN_M0_SPD1), ReadM0Speed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_M1_SPD1), ReadM1Speed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_M2_SPD1), ReadM2Speed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_M3_SPD1), ReadM3Speed, CHANGE);
}

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

String msg = "";
float kp = 0;
float ki = 0.2;
float kd = 0;

void loop() {

  if (stringComplete) {
    Serial.println(inputString);
    switch (inputString.charAt(0)) {
      case 'p':
        kp = inputString.substring(1).toFloat();
        break;
      case 'i':
        ki = inputString.substring(1).toFloat();
        break;
      case 'd':
        kd = inputString.substring(1).toFloat();
        break;
    }
    msg = "kp:" + String(kp) + " ki:" + String(ki) + " kd:" + String(kd);
    Serial.println(msg);
    // clear the string:
    inputString = "";
    stringComplete = false;

    analogWrite(PIN_M3_PWM, 0);
    delay(2000);

    _PIDMoter[3].setPID(kp, ki, kd);
    _PIDMoter[3].setMotor(70);

  }

  _PIDMoter[3].PIDControl();
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop
    // can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
    yield();
  }
}

static void ReadM0Speed() {
  _PIDMoter[0].ReadSpeed();
}

static void ReadM1Speed() {
  _PIDMoter[1].ReadSpeed();
}

static void ReadM2Speed() {
  _PIDMoter[2].ReadSpeed();
}

static void ReadM3Speed() {
  _PIDMoter[3].ReadSpeed();
}