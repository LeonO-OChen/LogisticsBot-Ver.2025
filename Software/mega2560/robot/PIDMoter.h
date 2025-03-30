
#include "config.h"

/*
    控制电机转速(增量式PID)
*/

#ifndef PIDMoter_H
#define PIDMoter_H

class PIDMoter {

  private:
    float _pwm = 0; // 电机的PWM值——增量式PID必须是浮点型
    // PID:本次误差,本次误差-上次误差,上次误差-上上次误差
    float _bias[3] = {0, 0, 0};
    float _kp = 1;            // PID:KP
    float _ki = 0.02;         // PID:KI
    float _kd = 0;            // PID:KD
    float _count100msTar = 0; // 电机的目标转速——换算成每100ms计数
    float _count100ms = 0; // 电机的实际转速——换算成每100ms的计数

    unsigned long _sampleTime = 0; // 距离上次采样的微秒数

    /** 电机是否反向 1：正向 -1：反向**/
    int8_t _offset = 1;

    // _PIN_PWM=0: 滑行到停止 1~255：转速（太小会不转）
    int8_t _pinPWM; // 电机的PWM引脚
    // (_inA,_inB)=(0,0) or (1,1):制动； (0,1)：正转；(1,0)：反转
    int8_t _pinInA; // 电机的功能1引脚
    int8_t _pinInB; // 电机的功能2引脚

    int8_t _pinSpd1; // 电机的测速1引脚
    int8_t _pinSpd2; // 电机的测速2引脚

  public:
    int _count = 0; // 电机的转速计数

    void init(int8_t offset, int8_t pinPWM, int8_t pinInA, int8_t pinInB,
              int8_t pinSpd1, int8_t pinSpd2);
    void setMotor(int mspeed);
    void setPID(float kp, float ki, float kd);
    void ReadSpeed();
    void PIDControl(); // 如需PID控制，每20ms调用次函数
};

#endif