/*
物流配送小车

主板：mega2560
电机驱动板：使用PWM输出

如何区分

*/

#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// 初始化oled屏幕
// Adafruit_SSD1306 oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, PIN_OLED_MOSI,
// PIN_OLED_CLK, PIN_OLED_DC, PIN_OLED_RESET, PIN_OLED_CS);
Adafruit_SSD1306 oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
    Serial.begin(115200);
    oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); // 启用OLED
    // oled.begin(SSD1306_SWITCHCAPVCC);  // 启用OLED
    oled.setTextSize(1);

    //---------------------------------------------
    // 配置数字输入端口
    //---------------------------------------------
    pinMode(PIN_M0_SPD1, INPUT);
    pinMode(PIN_M0_SPD2, INPUT);

    pinMode(PIN_M1_SPD1, INPUT);
    pinMode(PIN_M1_SPD2, INPUT);

    pinMode(PIN_M2_SPD1, INPUT);
    pinMode(PIN_M2_SPD2, INPUT);

    pinMode(PIN_M3_SPD1, INPUT);
    pinMode(PIN_M3_SPD2, INPUT);

    //速度输出
    pinMode(PIN_M0_PWM, OUTPUT);
    pinMode(PIN_M0_F1, OUTPUT);
    pinMode(PIN_M0_F2, OUTPUT);

    pinMode(PIN_M1_PWM, OUTPUT);
    pinMode(PIN_M1_F1, OUTPUT);
    pinMode(PIN_M1_F2, OUTPUT);

    pinMode(PIN_M2_PWM, OUTPUT);
    pinMode(PIN_M2_F1, OUTPUT);
    pinMode(PIN_M2_F2, OUTPUT);

    pinMode(PIN_M3_PWM, OUTPUT);
    pinMode(PIN_M3_F1, OUTPUT);
    pinMode(PIN_M3_F2, OUTPUT);

    // 前进
    digitalWrite(PIN_M0_F1, HIGH);
    digitalWrite(PIN_M0_F2, LOW);
    digitalWrite(PIN_M1_F1, HIGH);
    digitalWrite(PIN_M1_F2, LOW);
    digitalWrite(PIN_M2_F1, HIGH);
    digitalWrite(PIN_M2_F2, LOW);
    digitalWrite(PIN_M3_F1, HIGH);
    digitalWrite(PIN_M3_F2, LOW);

    analogWrite(PIN_M0_PWM, 50);
    analogWrite(PIN_M1_PWM, 50);
    analogWrite(PIN_M2_PWM, 50);
    analogWrite(PIN_M3_PWM, 50);
}

// =========================================================
// 主循环——读取按键，显示状态等
// =========================================================
void loop() {}
