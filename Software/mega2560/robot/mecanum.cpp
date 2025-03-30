#include "PIDMoter.h"
#include "config.h"
#include "mecanum.h"

MecanumDriver::MecanumDriver() {
    // 250rpm
    //  _wheel[1].setOffset(1);
    //  _wheel[2].setOffset(-1);
    //  _wheel[3].setOffset(1);
    //  _wheel[4].setOffset(-1);

    // 85rpm
    _PIDMoter[0].init(-1, PIN_M0_PWM, PIN_M0_F1, PIN_M0_F2, PIN_M0_SPD1,
                      PIN_M0_SPD2);
    _PIDMoter[1].init(1, PIN_M1_PWM, PIN_M1_F1, PIN_M1_F2, PIN_M1_SPD1,
                      PIN_M1_SPD2);
    _PIDMoter[2].init(-1, PIN_M2_PWM, PIN_M2_F1, PIN_M2_F2, PIN_M2_SPD1,
                      PIN_M2_SPD2);
    _PIDMoter[3].init(1, PIN_M3_PWM, PIN_M3_F1, PIN_M3_F2, PIN_M3_SPD1,
                      PIN_M3_SPD2);

    // 使用外部中断计数
    attachInterrupt(digitalPinToInterrupt(PIN_M0_SPD1),
                    MecanumDriver::ReadM0Speed, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_M1_SPD1),
                    MecanumDriver::ReadM1Speed, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_M2_SPD1),
                    MecanumDriver::ReadM2Speed, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_M3_SPD1),
                    MecanumDriver::ReadM3Speed, CHANGE);
}

void MecanumDriver::setMotor(
    int speedFL, int speedFR, int speedBL,
    int speedBR) { // 驱动4个电机 -- 左前，右前，左后，右后:速度 -100~100
    _PIDMoter[0].setMotor(speedFL);
    _PIDMoter[1].setMotor(speedFR);
    _PIDMoter[2].setMotor(speedBL);
    _PIDMoter[3].setMotor(speedBR);
}

// 以坐标方式设置小车的4轮速度
// Vx: X轴方向速度, 右为正，左为负
// Vy: y轴方向速度, 前为正，后为负
// w: Z轴方向角速度, 逆时针为正，顺时针为负
// 为了PID能有效控制，应该 |Vx|+|Vy|+|30w|<100
void MecanumDriver::setSpeed(int Vx, int Vy, float w) {
    int Vw = ((VEHICLE_SHEELBASE + VEHICLE_TREAD) >> 1) * w; // w*(a+b)
    _PIDMoter[0].setMotor(Vy + Vx - Vw);
    _PIDMoter[1].setMotor(Vy - Vx + Vw);
    _PIDMoter[2].setMotor(Vy - Vx - Vw);
    _PIDMoter[3].setMotor(Vy + Vx + Vw);
}

void MecanumDriver::PIDControl() {
    // 依次对每个轮胎做PID控制
    _PIDMoter[0].PIDControl();
    _PIDMoter[1].PIDControl();
    _PIDMoter[2].PIDControl();
    _PIDMoter[3].PIDControl();
}

static void MecanumDriver::ReadM0Speed() {
    _MecanumDriver._PIDMoter[0].ReadSpeed();
}

static void MecanumDriver::ReadM1Speed() {
    _MecanumDriver._PIDMoter[1].ReadSpeed();
}

static void MecanumDriver::ReadM2Speed() {
    _MecanumDriver._PIDMoter[2].ReadSpeed();
}

static void MecanumDriver::ReadM3Speed() {
    _MecanumDriver._PIDMoter[3].ReadSpeed();
}