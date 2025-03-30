#include "PIDMoter.h"

void PIDMoter::init(int8_t offset, int8_t pinPWM, int8_t pinInA, int8_t pinInB,
                    int8_t pinSpd1, int8_t pinSpd2) {
    this->_offset = offset;
    this->_pinPWM = pinPWM;
    this->_pinInA = pinInA;
    this->_pinInB = pinInB;
    this->_pinSpd1 = pinSpd1;
    this->_pinSpd2 = pinSpd2;

    //测速
    pinMode(pinSpd1, INPUT);
    pinMode(pinSpd2, INPUT);

    //速度输出
    pinMode(pinPWM, OUTPUT);
    pinMode(pinInA, OUTPUT);
    pinMode(pinInB, OUTPUT);
}

void PIDMoter::setPID(float kp, float ki, float kd) {
    this->_kp = kp;
    this->_ki = ki;
    this->_kd = kd;
}

// 驱动单个电机 -- mNum: 1~4电机号 mspeed:速度 -100~100
void PIDMoter::setMotor(int mspeed) {
    int speed = _offset * mspeed;

    // 使用分辨率11的电机：
    // 250rpm：减速比30，转一圈计数11*30*2=660，每分钟计数660*250=165000次，每100毫秒165000/600=275次
    // 140rpm：减速比56，转一圈计数11*56*2=1232，每分钟计数1232*140=172480次，每100毫秒172480/600=287.5次
    // 85rpm:减速比90，转一圈计数11*90*2=1980，每分钟计数1980*85=16830次，每100毫秒16830/600=280.5次
    // 即：无论使用哪种电机，基本都是280次/100ms

    _count100msTar = 2.8 * speed;
}

void PIDMoter::PIDControl() {

    unsigned long t1 = micros();
    unsigned long dt = t1 - _sampleTime;
    if (dt > 100000) {
        // 超过100ms，作废
        _count100ms = -1;
        _count = 0;
        _sampleTime = micros();
        return;
    } else if (dt < 5000) {
        // 小于15ms，跳过PID处理
        return;
    } else {
        _count100ms = _count; // 速度大小（含方向）
        _count = 0;
        _sampleTime = micros();

        // 换算成100ms的计数
        _count100ms = _count100ms * 100000 / dt;
    }

    // PID 计算
    float bias = _count100msTar - _count100ms;
    _bias[2] = _bias[1];        // 上次误差-上上次误差
    _bias[1] = bias - _bias[0]; // 本次误差-上次误差
    _bias[0] = bias;            // 本次误差

    // 增量式PID
    float pid =
        _kp * _bias[1] + _ki * _bias[0]; // + _kd * (_bias[1] - _bias[2]);
    _pwm += pid;

    Serial.print(255);
    Serial.print(" ");
    Serial.print(0);
    Serial.print(" ");
    Serial.print(_count100msTar);
    Serial.print(" ");
    Serial.print(_count100ms);
    Serial.print(" ");
    Serial.println(_pwm);

    _pwm = _pwm > 255 ? 255 : _pwm;
    _pwm = _pwm < -255 ? -255 : _pwm;

    // 输出
    if (_pwm > 0) {
        // 前进
        digitalWrite(_pinInA, HIGH);
        digitalWrite(_pinInB, LOW);
    } else if (_pwm < 0) {
        // 后退
        digitalWrite(_pinInA, LOW);
        digitalWrite(_pinInB, HIGH);
    } else {
        // 刹车
        digitalWrite(_pinInA, HIGH);
        digitalWrite(_pinInB, HIGH);
        analogWrite(_pinPWM, 255);
        return;
    }

    // pwm 过小时电机不会转，长时间会烧毁，因此不给电
    int ppwm = abs(_pwm);
    if (ppwm < 45) {
        ppwm = 0;
    }

    analogWrite(_pinPWM, ppwm);
}

void PIDMoter::ReadSpeed() {

    if (digitalRead(_pinSpd1) == HIGH) {
        if (digitalRead(_pinSpd2) == LOW) {
            _count++;
        } else {
            _count--;
        }
    } else {
        if (digitalRead(_pinSpd2) == LOW) {
            _count--;
        } else {
            _count++;
        }
    }
}
