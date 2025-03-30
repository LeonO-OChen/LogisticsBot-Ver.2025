#include "PIDMoter.h"
#include "config.h"
/*
  麦克纳姆轮4轮控制
*/
extern PIDMoter _PIDMoter[4];

#ifndef MecanumDriver_H
#define MecanumDriver_H

class MecanumDriver {
  public:
    PIDMoter _PIDMoter[4];
    MecanumDriver();

    // 驱动4个电机的绝对速度 -- 左前，右前，左后，右后:速度 -100~100
    void setMotor(int speedFL, int speedFR, int speedBL, int speedBR);

    // 驱动4个电机的绝对速度 -- Vx:X轴（右平移）速度  Vy:Y轴（前进）速度
    // w：逆时针旋转速度 为了PID能有效控制，应该 |Vx|+|Vy|+|30w|<100
    void setSpeed(int Vx, int Vy, float w); // 以坐标方式设置小车的4轮速度

    void PIDControl();// 如需PID控制，每30ms调用此函数

    static void ReadM0Speed();
    static void ReadM1Speed();
    static void ReadM2Speed();
    static void ReadM3Speed();

};

extern MecanumDriver _MecanumDriver;

#endif