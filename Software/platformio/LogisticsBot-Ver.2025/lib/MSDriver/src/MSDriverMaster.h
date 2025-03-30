#include "MSDriverCommon.h"
#include <Arduino.h>
/*
电机舵机驱动模块
通过I2C发送指令，可以控制4路电机，8路舵机

*/

#ifndef MSDriver_Master_H
#define MSDriver_Master_H

class MSDriverMaster
{
public:
    uint8_t i2c_addr = 0x32; // I2C模块地址
    uint8_t servoMode0123 = 0;
    uint8_t servoMode4567 = 0;

    void init(uint8_t addr = 0x32);
    void sendCmd(MSD_CMD cmd);

    void setMotorMode(int num, uint8_t &mode);
    void setMotorPID(int num, float kp, float ki, float kd, float kr);
    void motor100(int8_t num, int16_t speed);
    void motor(int8_t num, int16_t pwm);
    void motorBreak(int8_t num); // 刹车
    void motorRelease(int8_t num);  // 滑行

    void setServoMode(int num, const uint8_t &mode);
    void servo(int8_t num, uint8_t angle); // num(0~3), angle(0~180)

    bool getValueM(int num, int32_t *val);
    bool getValueMA(int num, uint16_t *val);
    bool getValueMB(int num, uint16_t *val);
    bool getValueS(int num, uint16_t *val);

    void writeReg(uint8_t reg, uint8_t *val, uint8_t len);
    bool readReg(uint8_t reg, uint8_t *val, uint8_t len);
};

#endif