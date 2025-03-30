#include "SCoop.h" // 多线程
#include "config.h"
#include "mecanum.h"
#include "motion.h"
#include "sensors.h" // 传感器
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h> // Timer5 舵机

extern Servo servoShiFang1; // 释放舵机（白）
extern Servo servoShiFang2; // 释放舵机（黄）
extern Servo servoZhuaDou1; // 抓斗舵机
extern Servo servoZhuaDou2; // 抓斗舵机
extern int _XunJiChuanGanQi[13]; // 12路巡迹传感器的数值 (0~1023),下标0不用
extern Adafruit_SSD1306 oled;

// 方向(1,2,3,4),档位(1,2,3,4,5)对应的速度(x,y)
static const int TBL_speed[5][6][2] = {
    // 不用
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    // 前
    {{0, 0}, {0, 25}, {0, 40}, {0, 65}, {0, 75}, {0, 90}},
    // 后
    {{0, 0}, {0, -25}, {0, -40}, {0, -65}, {0, -75}, {0, -90}},
    // 左
    {{0, 0}, {-25, 0}, {-40, 0}, {-65, 0}, {-75, 0}, {-90, 0}},
    // 右
    {{0, 0}, {25, 0}, {40, 0}, {65, 0}, {75, 0}, {90, 0}}};

// 档位对应的速度(%)
static const int TBL_speedPerCent[6] = {0, 25, 40, 65, 75, 90};

// 传感器阈值  每个灰度传感器的敏感度可能不同，为每个探头设置了阈值
// 大于threshold[n]，认为在黑线上
static const int TBL_threshold[13] = {
    0,   // 不用
    525, // 1
    385, // 2
    555, // 3
    400, // 4
    375, // 5
    390, // 6
    380, // 7
    345, // 8
    390, // 9
    575, // 10
    450, // 11
    460  // 12
};

// 传感器排列
// static const unsigned char PROGMEM ChuanGanQiPaiLie[5][13] = {
static const unsigned char TBL_ChuanGanQiPaiLie[5][13] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}, // 前进时
    {0, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}, // 后退时
    {0, 6, 5, 4, 10, 11, 12, 1, 2, 3, 9, 8, 7}, // 左移时
    {0, 7, 8, 9, 3, 2, 1, 12, 11, 10, 4, 5, 6}, // 右移时
};

// 平移方向和小车的夹角(-360~360), 右为正， 左为负
int _degree = 0;
// 微调角度(-360~360), 右为正， 左为负
int _degreeDiff = 5;

// 判断n传感器是不是在线上
bool detect(int n) { return (_XunJiChuanGanQi[n] > TBL_threshold[n]); }

// 平移
// 方向 direction=1，2，3，4：前，后，左，右
void motion_move(int direction, int gear) {
    _MecanumDriver.setSpeed(TBL_speed[direction][gear][0],
                            TBL_speed[direction][gear][1], 0);
}

// 绕轴旋转
// axis:旋转轴 0,1,2,3,4,5,6,7,8
// (中心，传感器前,传感器后，传感器左，传感器前右，左前轮，右前轮，左后轮，右后轮)
// gear:档位 -5~5 -- >0 逆时针转; <0 顺时针转
void motion_XuanZhuan(int axis, int gear) {

    int aSpeed = TBL_speedPerCent[abs(gear)];
    int a = 45; //
    int b = 42; //
    if (axis == 0) {
        // 绕中心旋转
        if (gear > 0) {
            // 逆时针转 左转
            _MecanumDriver.setMotor(0 - aSpeed, aSpeed, 0 - aSpeed, aSpeed);
        } else {
            // 顺时针转 右转
            _MecanumDriver.setMotor(aSpeed, 0 - aSpeed, aSpeed, 0 - aSpeed);
        }
    } else if (axis == 1) {
        // 绕前传感器旋转
        if (gear > 0) {
            // 逆时针转
            _MecanumDriver.setMotor(a - b, b - a, 0 - a - b, a + b);
        } else {
            // 顺时针转
            // -a+b,a-b,a+b,-a-b
            _MecanumDriver.setMotor(b - a, a - b, a + b, 0 - a - b);
        }
    } else if (axis == 2) {
        // 绕后传感器旋转
        if (gear > 0) {
            // 逆时针转
            _MecanumDriver.setMotor(0 - a - b, a + b, a - b, b - a);
        } else {
            // 顺时针转
            // a+b,-a-b,-a+b,a-b
            _MecanumDriver.setMotor(a + b, 0 - a - b, b - a, a - b);
        }
    } else if (axis == 3) {
        // 绕左传感器旋转
        if (gear > 0) {
            // 逆时针转
            _MecanumDriver.setMotor(-46, 90, -46, 90);
        } else {
            // 顺时针转
            _MecanumDriver.setMotor(46, -90, 46, -90);
        }
    } else if (axis == 4) {
        // 绕右传感器旋转
        if (gear > 0) {
            // 逆时针转
            _MecanumDriver.setMotor(-90, 46, -90, 46);
        } else {
            // 顺时针转
            _MecanumDriver.setMotor(90, -46, 90, -46);
        }
    } else if (axis == 5) {
        // 绕左前轮
        if (gear > 0) {
            // 逆时针转
            _MecanumDriver.setMotor(0, 62, -54, 82);
        } else {
            // 顺时针转
            _MecanumDriver.setMotor(0, -62, 54, -82);
        }
    } else if (axis == 6) {
        // 绕右前轮
        if (gear > 0) {
            // 逆时针转
            _MecanumDriver.setMotor(-62, 0, -82, 54);
        } else {
            // 顺时针转
            _MecanumDriver.setMotor(62, 0, 82, -54);
        }
    } else if (axis == 7) {
        // 绕左后轮
        if (gear > 0) {
            // 逆时针转
            _MecanumDriver.setMotor(-54, 82, 0, 62);
        } else {
            // 顺时针转
            _MecanumDriver.setMotor(54, -82, 0, -62);
        }
    } else if (axis == 8) {
        // 绕右后轮
        if (gear > 0) {
            // 逆时针转
            _MecanumDriver.setMotor(-82, 54, -62, 0);
        } else {
            // 顺时针转
            _MecanumDriver.setMotor(82, -54, 62, 0);
        }
    }
}

void motion_stop() { _MecanumDriver.setMotor(0, 0, 0, 0); }

// 抓取
void motion_ZhuaQu() {
    static const int tz=-5;  // 调整角度偏差。
    static const int Kuang[][2] = {
        // 0~175, 延时
        // 0收起
        {80, 200},  {102, 200}, {124, 200}, {146, 200},  {180, 800}, // 放下
        {160, 300}, {180, 800}, {160, 300}, {180, 800},  {160, 300}, // 抓取
        {180, 800}, {160, 300}, {180, 800}, {160, 300},  {180, 800}, // 抓取
        {180, 800}, {160, 300}, {180, 800}, {160, 300},  {180, 800}, // 抓取
        {146, 200}, {124, 400}, {110, 400}, {100, 1500}, {50, 1000}, // 收回
    };

    int DongZuoShuLiang = sizeof(Kuang) / sizeof(Kuang[0]);

    // 启动舵机
    servoZhuaDou1.attach(PIN_SERVO_1);
    servoZhuaDou2.attach(PIN_SERVO_2);

    for (int i = 0; i < DongZuoShuLiang; i++) {
        int dushu1 = Kuang[i][0] + tz;
        int dushu2 = 175 - dushu1;
        servoZhuaDou1.write(dushu1);
        servoZhuaDou2.write(dushu2);
        sleep(Kuang[i][1]);
    }

    // 关闭舵机
    servoZhuaDou1.detach();
    servoZhuaDou2.detach();
}

// 将舵机转到指定位置
// op:0: 1开2闭（初始位置)
//    1: 1闭2开
//    2: 1开2开
void motion_setServo(int color, int op) {
    if (color == 1) {
        switch (op) {
        case 0:
            servoShiFang1.write(90);
            break;
        case 1:
            servoShiFang1.write(0);
            break;
        case 2:
            servoShiFang1.write(170);
            break;
        }
    } else if (color == 2) {
        switch (op) {
        case 0:
            servoShiFang2.write(90);
            break;
        case 1:
            servoShiFang2.write(180);
            break;
        case 2:
            servoShiFang2.write(5);
            break;
        }
    }
}

/*
   寻迹
   direction=1，2，3，4：前，后，左，右
*/
void patrol(int direction) {

    // 0: 无矫正
    // 1: 微左转
    // 2: 微右转
    static int patrolMode = 0;

    unsigned char *g = TBL_ChuanGanQiPaiLie[direction];
    // 边走边对齐，保持2，11在线上，1，3，10，12在线外

    if (detect(g[2])) {
        // 对齐中
        if (patrolMode != 0) {
            _MecanumDriver.setSpeed(TBL_speed[direction][DEFAULT_GEAR][0],
                                    TBL_speed[direction][DEFAULT_GEAR][1], 0);
            patrolMode = 0;
        }
    } else if (detect(g[1])) {

        // 微左转
        if (patrolMode != 1) {
            _MecanumDriver.setSpeed(TBL_speed[direction][1][0],
                                    TBL_speed[direction][1][1], 2);
            patrolMode = 1;
        }

    } else if (detect(g[3])) {
        // 微右转
        if (patrolMode != 2) {
            _MecanumDriver.setSpeed(TBL_speed[direction][1][0],
                                    TBL_speed[direction][1][1], -2);
            patrolMode = 2;
        }
    } else {
        // 对齐中
        if (patrolMode != 0) {
            _MecanumDriver.setSpeed(TBL_speed[direction][DEFAULT_GEAR][0],
                                    TBL_speed[direction][DEFAULT_GEAR][1], 0);
            patrolMode = 0;
        }
    }
}

/*
   平移毫秒
*/
void motionXJ_moveTime(int direction, int gear, int ms) {

    _MecanumDriver.setSpeed(TBL_speed[direction][gear][0],
                            TBL_speed[direction][gear][1], 0);

    unsigned long t0;

    t0 = millis();
    // 保持平移，直到检测到岔口
    while (millis() - t0 < ms) {
        readXunJiChuanGanQi();
        patrol(direction);
        yield();
    }
}

// 寻迹:平移
// 方向 direction=1，2，3，4：前，后，左，右
// 直到g指定方向上的传感器检测到(times)次黑线 1，2，3，4：前，后，左，右
// wait=true: 第一次检测前适当延时
void motionXJ_move(int direction, int g, int times, bool wait) {
    unsigned long t0 = 0;

    _MecanumDriver.setSpeed(TBL_speed[direction][DEFAULT_GEAR][0],
                            TBL_speed[direction][DEFAULT_GEAR][1], 0);

    if (wait) {
        sleep(100);
    }

    // 重复
    while (times > 0) {
        t0 = millis();
        // 保持平移，直到检测到岔口
        while (1) {
            readXunJiChuanGanQi();
            patrol(direction);

            readXunJiChuanGanQi();

            if (g == 1) {
                if ((detect(1) || detect(2) || detect(3)) &&
                    millis() - t0 > 500) {
                    break;
                }
            } else if (g == 2) {
                if ((detect(10) || detect(11) || detect(12)) &&
                    millis() - t0 > 500) {
                    break;
                }
            } else if (g == 3) {
                if ((detect(4) || detect(5) || detect(6)) &&
                    millis() - t0 > 500) {
                    break;
                }
            } else if (g == 4) {
                if ((detect(7) || detect(8) || detect(9)) &&
                    millis() - t0 > 500) {
                    break;
                }
            }

            // 如果到交叉处(500ms以内不检测——保证走出线后再检测)

            yield();
        }

        times--;
    }
    // 停止
    _MecanumDriver.setMotor(0, 0, 0, 0);
}

// 原地旋转直到g传感器检测到线
// direction = 1左转, -1:右转
void motionXJ_turn(int direction, int g) {
    motion_XuanZhuan(0, direction * 3);
    sleep(200);
    readXunJiChuanGanQi();
    while (!detect(g)) {
        yield();
        sleep(5);
        readXunJiChuanGanQi();
    }
    // 停止
    motion_stop();
}

//顶箱子
void DingXiangZi() {
    motion_move(1, 1);
    while (digitalRead(PIN_WEIDONG_2) == HIGH &&
           digitalRead(PIN_WEIDONG_3) == HIGH) {
        sleep(20);
    }
    motion_stop();
}

// 辨色
// 越亮值越小
int BianSe_huidu() {
    // 1:白 2:黄
    if (analogRead(PIN_XUNJI_2) < 350) {
        return 1;
    } else {
        return 2;
    }
}

int BianSe_K210() {
    sleep(200);
    int cntW = 0; // 白色计数
    int cntY = 0; // 黄色计数

    // 连续3次收到同样信号则确定，最多尝试100次
    for (int i = 0; i < 100; i++)
        while (cntW < 3 && cntY < 3) {
            while (!Serial.available())
                sleep(1);

            int inByte = Serial.read();
            if (inByte == 1) {
                // 白色
                cntW++;
                cntY = 0;
            } else if (inByte == 2) {
                // 黄色
                cntW = 0;
                cntY++;
            } else {
                cntW = 0;
                cntY = 0;
            }
            sleep(20);
        }

    if (cntW) {
        return 1;
    } else if (cntY) {
        return 2;
    }
    return 0;
}

// 巡线前行
// 返回偏转系数
float B_Xforward() {
    static float kp = 0.0001;
    static float ki = 0;
    static float kd = 0;
    static int error = 0;
    static int lastError = 0;
    static int allError = 0;

    // 灰度值越大，颜色越深（黑）
    error = analogRead(PIN_XUNJI_1) - analogRead(PIN_XUNJI_3);
    allError += error;

    float pid = kp * error + ki * allError + kd * (error - lastError);
    lastError = error;

    return pid;
}

bool B_atCross() {
    if (analogRead(PIN_XUNJI_1) > 3000 || analogRead(PIN_XUNJI_3) > 3000) {
        return true;
    }
    return false;
}


void motionX_forward(int n){
    int Vy = 50;
    float w = 0;
    do {
        w += B_Xforward();
        _MecanumDriver.setSpeed(0, Vy, w);
        if (B_atCross()) {
            n--;
            while (B_atCross())
                ;
        }

    } while (n > 0);
    motion_stop();
}