/*
  CPU:  ESP32-S3
*/

#include "config.h"
#include "MSDriverMaster.h"
#include "i2cMaster.h" // I2C通信


I2C_Master _i2c;

// 电机驱动模块
MSDriverMaster _MSDriverMaster;

void initMSDriver();
void showLed(uint8_t val);

void setup() {
    Serial.begin(115200); // 初始化串口，波特率设置为9600
    // _i2c.init(I2C_SDA, I2C_SCL, I2C_FREQ);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_LED_A0, OUTPUT);
    pinMode(PIN_LED_A1, OUTPUT);
    pinMode(PIN_LED_A2, OUTPUT);

    // 等待其它设备上电完毕
    delay(1000);

    //initMSDriver();

    delay(1000);
}

// 主任务
void loop() {
    for (int i = 0; i < 8; i++) {
        showLed(i);
        delay(500);
    }
}

// 4电机闭环控制，舵机全开
void initMSDriver() {
    uint8_t motorMode = 0b11001001; // 测速，计数自动清零，PID控制

    uint8_t smode = 0b11; // 舵机模式
    _MSDriverMaster.init(0x32);
    _MSDriverMaster.setMotorMode(-1, motorMode);            // 设置所有电机工作模式
    _MSDriverMaster.setMotorPID(-1, 0.6, 0.000001, 0, 2.8); // 设置所有电机PID参数
    _MSDriverMaster.setServoMode(-1, smode);                // 设置所有舵机工作模式
    _MSDriverMaster.sendCmd(APPLY);
}

void showLed(uint8_t val) {
    digitalWrite(PIN_LED_A0, val& 0b0001);
    digitalWrite(PIN_LED_A1, val& 0b0010);
    digitalWrite(PIN_LED_A2, val& 0b0100);
}