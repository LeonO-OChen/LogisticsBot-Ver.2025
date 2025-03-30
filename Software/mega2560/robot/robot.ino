/*
物流配送小车

主板：mega2560
电机驱动板：使用PWM输出

如何区分

*/

#include "SCoop.h" // 多线程
#include "config.h"
#include "hanzi.h"
#include "mecanum.h" // 4轮驱动
#include "motion.h"  // 机器人基本动作
#include "sensors.h" // 传感器
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h> // Timer5 舵机
#include <Wire.h>

static const unsigned char PIN_LED_SEL[] = {8,
                                            PIN_DISP_FUN1,
                                            PIN_DISP_FUN2,
                                            PIN_DISP_FUN3,
                                            PIN_DISP_FUN4,
                                            PIN_DISP_FUN5,
                                            PIN_DISP_FUN6,
                                            PIN_DISP_FUN7,
                                            PIN_DISP_FUN8};

int _XunJiChuanGanQi[13] = {0}; // 12路巡迹传感器的数值 (0~1023),下标0不用
int _GuangMin[4]; // 灰度辨色 (0~1023),  0,1,2,3: 不用，白，黄，辨色

// 用于区分乒乓球颜色
// 越亮值越小,黑色（空）最大)
// 一般认为白色比黄色亮(同种品牌的乒乓球)
const int _GuangMinThresholdW = 600; // 小于此数认为白色
const int _GuangMinThresholdY = 750; // 小于此数认为黄色

// 用于判断乒乓球是否在2号区
// 大于此值时，认为2号区有球(越亮值越大,黑色（空）最小)
int _GuangMinThreshold[3] = {0, 650,
                             750}; // 判断阈值  -- 不用，左边阈值，右边阈值

int _WeiDongKaiGuan[4]; // 3路微动开关 (0~1),下标0不用
int _Task = 0; // 任务（1:排空 2：装货 3~6:送货 7:测试1 8:测试2）

int _Select;    // 选择项 1~8
int _load[3];   // 已经装了几个球
int _toLoad[3]; // 还要装几个球

// 初始化oled屏幕
// Adafruit_SSD1306 oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, PIN_OLED_MOSI,
// PIN_OLED_CLK, PIN_OLED_DC, PIN_OLED_RESET, PIN_OLED_CS);
Adafruit_SSD1306 oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
int displayMode = 0; // 0:不显示,1:显示传感器，2：显示PID

// 麦克纳姆轮4轮控制
MecanumDriver _MecanumDriver;

Servo servoZhuaDou1; // 抓斗舵机
Servo servoZhuaDou2; // 抓斗舵机
Servo servoFenLei;   // 分类舵机（颜色）
Servo servoShiFang1; // 释放舵机（白）
Servo servoShiFang2; // 释放舵机（黄）

defineTask(Taskback); // 子线程——舵机控制，物品分类

void setup() {
    Serial.begin(115200);
    oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); // 启用OLED
    // oled.begin(SSD1306_SWITCHCAPVCC);  // 启用OLED
    show_hello();
    oled.setTextSize(1);

    //---------------------------------------------
    // 配置数字输入端口
    //---------------------------------------------
    pinMode(PIN_MODE_TEST, INPUT_PULLUP);

    // 微动开关
    pinMode(PIN_WEIDONG_1, INPUT_PULLUP);
    pinMode(PIN_WEIDONG_2, INPUT_PULLUP);
    pinMode(PIN_WEIDONG_3, INPUT_PULLUP);

    // 旋转编码器
    pinMode(PIN_CODE_CLK, INPUT);       // 时钟
    pinMode(PIN_CODE_D, INPUT);         // 数据
    pinMode(PIN_CODE_SW, INPUT_PULLUP); // 按钮

    //----------------------------------------------------
    // 配置数字输出端口
    //----------------------------------------------------
    // LED显示
    for (int i = 1; i <= PIN_LED_SEL[0]; i++) {
        pinMode(PIN_LED_SEL[i], OUTPUT);
    }

    // 超声测距触发
    // pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);

    // 风扇
    pinMode(PIN_FAN_1, OUTPUT);
    pinMode(PIN_FAN_2, OUTPUT);

    // -----------------------------------------------
    //  配置模拟输入端口
    // -----------------------------------------------
    // 巡线传感器
    pinMode(PIN_XUNJI_1, INPUT);
    pinMode(PIN_XUNJI_2, INPUT);
    pinMode(PIN_XUNJI_3, INPUT);
    pinMode(PIN_XUNJI_4, INPUT);
    pinMode(PIN_XUNJI_5, INPUT);
    pinMode(PIN_XUNJI_6, INPUT);
    pinMode(PIN_XUNJI_7, INPUT);
    pinMode(PIN_XUNJI_8, INPUT);
    pinMode(PIN_XUNJI_9, INPUT);
    pinMode(PIN_XUNJI_10, INPUT);
    pinMode(PIN_XUNJI_11, INPUT);
    pinMode(PIN_XUNJI_12, INPUT);

    // 变色/物体通过传感器
    pinMode(PIN_GUANGMIN_1, INPUT);
    pinMode(PIN_GUANGMIN_2, INPUT);
    pinMode(PIN_GUANGMIN_3, INPUT);

    // -----------------------------------------------
    //  初始化
    // -----------------------------------------------

    // 关闭风扇
    digitalWrite(PIN_FAN_1, LOW);
    digitalWrite(PIN_FAN_2, LOW);

    _Select = 1;
    _Task = 0; // 等待
    _load[0] = 0;
    _load[1] = 0;
    _load[2] = 0;
    _toLoad[0] = 0;
    _toLoad[1] = 0;
    _toLoad[2] = 0;

    // 舵机位置复原
    servoFenLei.attach(PIN_SERVO_3);   // 分类舵机
    servoShiFang1.attach(PIN_SERVO_4); // 装填舵机（左）
    servoShiFang2.attach(PIN_SERVO_5); // 装填舵机（右）
    servoFenLei.write(90);             // 分类舵机回到中心
    motion_setServo(1, 0);             // 装填舵机（左）关闭
    motion_setServo(2, 0);             // 装填舵机（右）关闭
    sleep(500);

    oled.clearDisplay();
    oled.display();

    // 开启多线程
    displayMode = 1;
    mySCoop.start();
}

// =========================================================
// 主循环——读取按键，显示状态等
// =========================================================
void loop() {

    static int ledFlash = 0;
    static unsigned long t0 = 0;

    int offset; // 旋转编码器读取结果

    // 读取选择编码器旋钮
    offset = readXuanZhuanBianMaQi();
    _Select += offset;
    if (_Select > PIN_LED_SEL[0]) {
        _Select = 1;
    } else if (_Select < 1) {
        _Select = PIN_LED_SEL[0];
    }

    // 读取选择编码器按钮
    // 选中的选项作为下一个任务
    if (buttonDown()) {
        _Task = _Select;
    }

    // 之后的程序每50ms才运行一次
    if (!timePassed(t0, 50)) {
        yield();
        return;
    }

    readXunJiChuanGanQi();
    readWeiDongKaiGuan();

    if (_Task == 0) {
        // 不执行任务时： 指示灯闪烁
        for (int i = 1; i <= PIN_LED_SEL[0]; i++) {
            if (i != _Select) {
                digitalWrite(PIN_LED_SEL[i], LOW);
            } else {
                // 闪烁
                ledFlash = 1 - ledFlash;
                digitalWrite(PIN_LED_SEL[i], ledFlash);
            }
        }
    } else {
        // 执行任务时： 指示灯常亮
        for (int i = 1; i <= PIN_LED_SEL[0]; i++) {
            if (i == _Task) {
                digitalWrite(PIN_LED_SEL[i], HIGH);
            } else {
                digitalWrite(PIN_LED_SEL[i], LOW);
            }
        }
    }

    if (_Task == 1) { // 排空
        displayMode = 0;
        empty();
        _Task = 0;
    } else if (_Task == 2) { // 装货
        displayMode = 0;
        renwu1();

        if (digitalRead(PIN_MODE_TEST)) {
            _Task = 0;
        } else {
            _Task++;
        }

    } else if (_Task == 3) { // 起点送货
        displayMode = 0;
        renwu2_1();
        if (digitalRead(PIN_MODE_TEST)) {
            _Task = 0;
        } else {
            _Task++;
        }
        _Select = _Task;
    } else if (_Task == 4) { // 中间1送货
        displayMode = 0;
        renwu2_2();
        if (digitalRead(PIN_MODE_TEST)) {
            _Task = 0;
        } else {
            _Task++;
        }
        _Select = _Task;
    } else if (_Task == 5) { // 中间2送货
        displayMode = 0;
        renwu3_1();
        if (digitalRead(PIN_MODE_TEST)) {
            _Task = 0;
        } else {
            _Task++;
        }
        _Select = _Task;
    } else if (_Task == 6) { // 中间3送货
        displayMode = 0;
        renwu3_2();
        _Task = 0;
        _Select = _Task;
    } else if (_Task == 7) { // 测试
        displayMode = 0;
        Test1();

        displayMode = 0;
        sleep(20);
        oled.clearDisplay(); // 清屏
        oled.display();
        _Task = 0;

    } else if (_Task == 8) { // 测试2
        displayMode = 0;
        Test8();
        displayMode = 0;
        sleep(20);
        oled.clearDisplay(); // 清屏
        oled.display();
        _Task = 0;
    }

    displayMode = 1;
    yield();
}

// =========================================================
// 子线程：舵机控制，物品分类
// =========================================================
void Taskback::setup() {}

void Taskback::loop() {

    // 确保每20ms才继续执行
    static unsigned long t0 = 0;
    if (timePassed(t0, 20)) {

        readGuangMin();

        if (_Task || _Select > 1) {
            // 待机时不执行以下动作

            // 按颜色分拣小球
            task_classify();

            // 将小球装入发射管
            task_load(1); //白
            task_load(2); //黄
        }
    }

    _MecanumDriver.PIDControl();

    if (digitalRead(PIN_MODE_TEST)) {
        // oledDisplay();
    }
}

// =========================================================
// 功能函数
// =========================================================
// 将小球按颜色分到不同的盒子
// 黄球对蓝色的反光更小-->传感器的值更小
void task_classify() {
    // 将分拣及装入动作分成若干步骤，每20ms执行一次本程序，执行相应的步骤
    static int step = 0; // 分拣步骤
    static int color = 0;

    if (step == 0) {
        // 等待直到检测到有球
        if (_GuangMin[3] < _GuangMinThresholdY) {
            step = 1;
        }
        return;
    }

    step++;
    if (step == 25) {
        // 0.5秒(20*25)后再次检测
        if (_GuangMin[3] < _GuangMinThresholdY) {
            step = 200;
            // 确实有球
            if (_GuangMin[3] < _GuangMinThresholdW) {
                // 白
                color = 1;
                servoFenLei.write(30);
            } else {
                // 黄
                color = 2;
                servoFenLei.write(141);
            }
        } else {
            // 错误信号，从头开始
            step = 0;
        }
    } else if (step == 240) {
        // 0.8s(200~240)后，舵机复位
        step = 300;
        servoFenLei.write(90);
    } else if (step == 325) {
        // 0.5s(300~325)后，舵机复位完毕
        step = 0;
    }
}

// 将单一颜色装入发射管
// 每20ms执行一次本程序，执行相应的步骤
void task_load(int color) {
    // 将分拣及装入动作分成若干步骤，每20ms执行一次本程序，执行相应的步骤
    static int step[3] = {0, 0, 0}; // 装弹步骤
    // 没有装弹命令
    if (_toLoad[color] == 0) {
        return;
    }

    ++step[color];
    if (step[color] < 1500) {
        if (_GuangMin[color] > _GuangMinThreshold[color]) {
            // 30秒钟内检测到乒乓球在2号区域
            // 下一步骤：再次检测
            step[color] = 3000;
        }
    } else if (step[color] == 1500) {
        // 没有检测到2号区域有球，装弹完毕
        _toLoad[color] = 0; // 放弃装弹
        step[color] = 0;
    } else if (step[color] == 3025) {
        // 等待0.5秒再次检测
        // 3000~3025 = 0.5s
        if (_GuangMin[color] > _GuangMinThreshold[color]) {
            // 第二次检测到乒乓球——确认
            // 转动阀门，等待一会儿
            step[color] = 4000;
            motion_setServo(color, 1);
        } else {
            // 第二次没有检测到乒乓球——回退
            step[color] = 100;
        }
    } else if (step[color] == 4040) {
        // 等待0.8秒检查是否已通过
        // 4000~4040 = 0.8s
        if (_GuangMin[color] < _GuangMinThreshold[color]) {
            // 已通过，关闭阀门，进入下一步骤
            step[color] = 8000;
            motion_setServo(color, 0);
        } else {
            // 未通过，尝试关闭后重新开启
            step[color] = 9000;
            motion_setServo(color, 0);
        }
    } else if (step[color] == 8025) {
        // 等待0.5秒后阀门完全关闭，装入完成
        // 8000~8025 = 0.5s
        _load[color]++;
        _toLoad[color]--;
        step[color] = 0;
    } else if (step[color] == 9030) {
        // 等待0.6秒后阀门完全关闭，然后重新检测
        // 9000~9030 = 0.6s
        step[color] = 0;
    }
}

// 将小球装入发射管
// color: 1:白 2:黄
void load(int color, int num) {
    // 将分拣及装入动作分成若干步骤，每20ms执行一次本程序，执行相应的步骤
    _load[color] = 0;
    _toLoad[color] = num;
}

// 查看发射管内的小球数
// 返回:-1: 正在装弹
//       0: 装弹结束，但是没有
//      1~：装弹结束，返回装弹个数
int loaded(int color) {
    if (_toLoad[color] == 0) {
        // 不在装弹或装弹结束
        return _load[color];
    }
    return -1;
}

// 在OLED上显示所有传感器的数值
// 由于显示占用大量时间，可能导致死机，因此改成分步显示
void oledDisplay() {
    static char str[30] = {0};
    static int step = 0;

    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    if (displayMode == 1) {
        // 显示传感器读数
        step++;
        switch (step) {
        case 1:
            // 显示前方4个循迹传感器的数值
            oled.fillRect(12, 0, 110, 8, SSD1306_BLACK);
            oled.setCursor(12, 0);
            sprintf(str, "%5d %5d %5d %5d", _XunJiChuanGanQi[1],
                    _XunJiChuanGanQi[2], _XunJiChuanGanQi[3],
                    _XunJiChuanGanQi[4]);
            oled.print(str);
            oled.display();
            break;
        case 2:
            // 显示后方4个循迹传感器的数值
            oled.fillRect(12, 46, 110, 8, SSD1306_BLACK);
            oled.setCursor(12, 46);
            sprintf(str, "%5d %5d %5d %5d", _XunJiChuanGanQi[5],
                    _XunJiChuanGanQi[6], _XunJiChuanGanQi[7],
                    _XunJiChuanGanQi[8]);
            oled.print(str);
            oled.display();
            break;
        case 3:
            // 显示左边2个循迹传感器的数值
            oled.fillRect(0, 12, 30, 32, SSD1306_BLACK);
            oled.setCursor(0, 12);
            sprintf(str, "%5d", _XunJiChuanGanQi[9]);
            oled.print(str);
            oled.setCursor(0, 24);
            sprintf(str, "%5d", _XunJiChuanGanQi[10]);
            oled.print(str);
            oled.display();
            break;
        case 4:
            // 显示右边2个循迹传感器的数值
            oled.fillRect(96, 12, 30, 32, SSD1306_BLACK);
            oled.setCursor(96, 12);
            sprintf(str, "%5d", _XunJiChuanGanQi[11]);
            oled.print(str);
            oled.setCursor(96, 24);
            sprintf(str, "%5d", _XunJiChuanGanQi[12]);
            oled.print(str);
            oled.display();
            break;
        case 5:
            // 显示3个灰度传感器（光敏电阻）的数值
            oled.fillRect(10, 56, 110, 8, SSD1306_BLACK);
            oled.setCursor(10, 56);
            sprintf(str, "%4d   %4d   %4d", _GuangMin[3], _GuangMin[1],
                    _GuangMin[2]);
            oled.print(str);
            oled.drawBitmap(0, 55, HAN_ZI_10[0], 10, 10, 1);  // 前
            oled.drawBitmap(42, 55, HAN_ZI_10[1], 10, 10, 1); // 左
            oled.drawBitmap(86, 55, HAN_ZI_10[2], 10, 10, 1); // 右
            oled.display();
            break;
        case 6:
            // 显示3个微动开关的数值
            if (_WeiDongKaiGuan[1]) {
                oled.drawRect(41, 28, 5, 5, SSD1306_WHITE);
            } else {
                oled.fillRect(41, 28, 5, 5, SSD1306_WHITE);
            }
            oled.display();
            break;
        case 7:
            if (_WeiDongKaiGuan[2]) {
                oled.drawRect(58, 36, 5, 5, SSD1306_WHITE);
            } else {
                oled.fillRect(58, 36, 5, 5, SSD1306_WHITE);
            }
            oled.display();
            break;
        case 8:
            if (_WeiDongKaiGuan[3]) {
                oled.drawRect(75, 36, 5, 5, SSD1306_WHITE);
            } else {
                oled.fillRect(75, 36, 5, 5, SSD1306_WHITE);
            }
            oled.display();
            break;
        default:
            step = 0;
        }
        // 显示

        // } else if (displayMode == 2) {
        //     // 显示PID数据

        //     step++;
        //     switch (step) {
        //     case 1:
        //         sprintf(str, "%3d/%3d %3d", (int)_PIDMoter[0]._count100ms,
        //                 (int)_PIDMoter[0]._count100msTar, _PIDMoter[0]._pwm);
        //         oled.fillRect(0, 10, 128, 8, SSD1306_BLACK);
        //         oled.setCursor(1, 10);
        //         oled.print(str);
        //         oled.display();
        //         break;
        //     case 2:
        //         sprintf(str, "%3d/%3d %3d", (int)_PIDMoter[1]._count100ms,
        //                 (int)_PIDMoter[1]._count100msTar, _PIDMoter[1]._pwm);
        //         oled.fillRect(30, 20, 90, 8, SSD1306_BLACK);
        //         oled.setCursor(30, 20);
        //         oled.print(str);
        //         oled.display();
        //         break;
        //     case 3:
        //         sprintf(str, "%3d/%3d %3d", (int)_PIDMoter[2]._count100ms,
        //                 (int)_PIDMoter[2]._count100msTar, _PIDMoter[2]._pwm);
        //         oled.fillRect(0, 46, 128, 8, SSD1306_BLACK);
        //         oled.setCursor(1, 46);
        //         oled.print(str);
        //         oled.display();
        //         break;
        //     case 4:
        //         sprintf(str, "%3d/%3d %3d", (int)_PIDMoter[3]._count100ms,
        //                 (int)_PIDMoter[3]._count100msTar, _PIDMoter[3]._pwm);
        //         oled.fillRect(30, 56, 90, 8, SSD1306_BLACK);
        //         oled.setCursor(30, 56);
        //         oled.print(str);
        //         oled.display();
        //     default:
        //         step = 0;
        //     }
        // } else if (displayMode == 3) {
        //     //
        //     sprintf(str, "%3d/%3d %3d", (int)_PIDMoter[0]._count100ms,
        //             (int)_PIDMoter[0]._count100msTar, _PIDMoter[0]._pwm);
        //     Serial.println(str);
    }
}

// 排空
// 关闭风扇
// 复位舵机
void empty() {
    int l1, l2;

    // OLED显示
    oled.clearDisplay();
    oled.drawBitmap(20, 30, HAN_ZI_16[0], 16, 16, 1); // 排
    oled.drawBitmap(40, 30, HAN_ZI_16[1], 16, 16, 1); // 空
    oled.drawBitmap(60, 30, HAN_ZI_16[2], 16, 16, 1); // …
    oled.display();

    // 关闭风扇
    digitalWrite(PIN_FAN_1, LOW);
    digitalWrite(PIN_FAN_2, LOW);

    while (true) {
        // 将管内乒乓球冲走
        // 打开风扇，3秒后关闭
        digitalWrite(PIN_FAN_1, HIGH);
        digitalWrite(PIN_FAN_2, HIGH);
        sleep(3000);
        digitalWrite(PIN_FAN_1, LOW);
        digitalWrite(PIN_FAN_2, LOW);

        // 发出装填指令——从线程会开始装填
        load(1, 5); // 装入5个白球
        load(2, 5); // 装入5个黄球

        // 等待装填完成
        while (1) {
            l1 = loaded(1);
            l2 = loaded(2);
            // 等待直到装入完毕——装填数<0表示装填未完成，>=0表示最终装填数
            if (l1 < 0 || l2 < 0) {
                yield();
            } else {
                break;
            }
        }

        // 如果已经没有球，退出任务
        if (l1 == 0 && l2 == 0) {
            break;
        }
    }
}

void show_hello() {
    oled.clearDisplay(); // 清屏
    oled.setTextSize(2);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(40, 16);
    oled.println("Hello!");
    oled.display(); // 显示
}

void renwu1() {
    // 开始
    if (!digitalRead(PIN_MODE_TEST)) {
        // 正式模式时先暂停30秒
        sleep(30000);
    }

    // 抓取
    motion_ZhuaQu();
}

void renwu2_1() {
    motionXJ_move(4, 1, 2, false); // 3号右移2格
    load(1, 5);                    // 装入5个白球
    load(2, 5);                    // 装入5个黄球
    motionXJ_move(1, 4, 2, false); // 7号前进2格
    motionXJ_turn(1, 2);           // 左转直到2号传感器到黑线
    motionXJ_move(1, 4, 3, false); // 7号前进3格
    motionXJ_turn(1, 2);           // 左转直到2号传感器到黑线

    int color = BianSe_K210();

    //顶箱子
    DingXiangZi();

    // 等待装填完成
    while (1) {
        // 等待直到装入完毕——装填数<0表示装填未完成，>=0表示最终装填数
        if (loaded(1) < 0 || loaded(2) < 0) {
            yield();
        } else {
            break;
        }
    }

    // 辨色
    if (color == 1) {
        // 白
        // 将管内乒乓球冲走
        // 打开风扇，3秒后关闭
        digitalWrite(PIN_FAN_1, HIGH);
        sleep(3000);
        digitalWrite(PIN_FAN_1, LOW);
    } else {
        // 黄
        // 将管内乒乓球冲走
        // 打开风扇，3秒后关闭
        digitalWrite(PIN_FAN_2, HIGH);
        sleep(3000);
        digitalWrite(PIN_FAN_2, LOW);
    }
}

void renwu2_2() {
    motionXJ_move(2, 3, 2, false); // 6号后退2格
    motionXJ_turn(1, 1);           // 左转， 直到1号传感器到线
    motionXJ_move(1, 4, 2, true);  // 前进2格

    int color = BianSe_K210();

    DingXiangZi();

    // 辨色
    if (color == 1) {
        // 白
        // 将管内乒乓球冲走
        // 打开风扇，3秒后关闭
        digitalWrite(PIN_FAN_1, HIGH);
        sleep(3000);
        digitalWrite(PIN_FAN_1, LOW);
    } else {
        // 黄
        // 将管内乒乓球冲走
        // 打开风扇，3秒后关闭
        digitalWrite(PIN_FAN_2, HIGH);
        sleep(3000);
        digitalWrite(PIN_FAN_2, LOW);
    }
}

void renwu3_1() {
    load(1, 5); // 装入5个白球
    load(2, 5); // 装入5个黄球

    motionXJ_move(2, 4, 1, false); // 9号后退1格
    motionXJ_move(4, 1, 1, true);  // 3号右移1格
    motionXJ_move(1, 3, 1, false); // 4号前进1格
    motionXJ_move(3, 1, 1, false); // 1号左移1格
    motionXJ_move(1, 4, 1, true);  // 7号前进1格
    motionXJ_move(4, 1, 1, false); // 3号右移1格

    DingXiangZi();

    // 等待直到装入完毕——装填数<0表示装填未完成，>=0表示最终装填数
    while (1) {
        if (loaded(1) < 0 || loaded(2) < 0) {
            yield();
        } else {
            break;
        }
    }

    // 将管内乒乓球冲走
    // 打开风扇，3秒后关闭
    digitalWrite(PIN_FAN_1, HIGH);
    digitalWrite(PIN_FAN_2, HIGH);
    sleep(3000);
    digitalWrite(PIN_FAN_1, LOW);
    digitalWrite(PIN_FAN_2, LOW);
}

void renwu3_2() {
    motionXJ_move(2, 4, 1, false); // 7号后退1格
    motionXJ_move(4, 1, 1, true);  // 3号右移1格
    motionXJ_move(1, 4, 1, false); // 7号前进1格

    motionXJ_move(1, 1, 1, false); // 1号前进1格
                                   // motion_move(1,2);
                                   // sleep(300);
                                   // motion_stop();
}

// 测试基本动作
void Test1() {
    displayMode = 2;
    sleep(30);
    oled.clearDisplay(); // 清屏
    oled.display();

    //DingXiangZi();

    unsigned long t0 = micros();

    // 前，后，左，右
    motion_move(1, 3);
    sleep(2000);
    motion_move(2, 3);
    sleep(2000);
    motion_move(3, 3);
    sleep(2000);
    motion_move(4, 3);
    sleep(2000);

    // 原地右转，原地左转
    motion_XuanZhuan(0, -5);
    sleep(3000);
    motion_XuanZhuan(0, 5);
    sleep(3000);

    // 绕前传感器顺时针转，逆时针转
    motion_XuanZhuan(1, -3);
    sleep(3000);
    motion_XuanZhuan(1, 3);
    sleep(3000);

    // 绕后传感器顺时针转，逆时针转
    motion_XuanZhuan(2, -3);
    sleep(3000);
    motion_XuanZhuan(2, 3);
    sleep(3000);

    // 绕左传感器顺时针转，逆时针转
    motion_XuanZhuan(3, -3);
    sleep(3000);
    motion_XuanZhuan(3, 3);
    sleep(3000);

    // 绕右传感器顺时针转，逆时针转
    motion_XuanZhuan(4, -3);
    sleep(3000);
    motion_XuanZhuan(4, 3);
    sleep(3000);

    // 绕左前轮顺时针转，逆时针转
    motion_XuanZhuan(5, -3);
    sleep(3000);
    motion_XuanZhuan(5, 3);
    sleep(3000);

    // 绕右前轮顺时针转，逆时针转
    motion_XuanZhuan(6, -3);
    sleep(3000);
    motion_XuanZhuan(6, 3);
    sleep(3000);

    // 绕左后轮顺时针转，逆时针转
    motion_XuanZhuan(7, -3);
    sleep(3000);
    motion_XuanZhuan(7, 3);
    sleep(3000);

    // 绕右后轮顺时针转，逆时针转
    motion_XuanZhuan(8, -3);
    sleep(3000);
    motion_XuanZhuan(8, 3);
    sleep(3000);

    // // motion_move(1, 0);        // 前进->停
    // // motion_moveRepeat(3, 1);  // 左移1格

    // motion_turn(2,3); //右转
    // _MecanumDriver.setMotor(80, -80, 80, -80);
    motion_stop();
}

// 测试屏幕
void Test2() {
    char str1[24] = {0};
    int c;
    oled.clearDisplay(); // 清屏
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.print("IR");
    oled.display();

    while (1) {
        if (Serial1.available()) {
            c = Serial1.read();
            oled.clearDisplay(); // 清屏
            oled.setTextSize(1);
            oled.setTextColor(SSD1306_WHITE);
            oled.setCursor(0, 0);
            sprintf(str1, "%d", c);
            oled.print(str1);
            oled.print("cd");
            oled.display();
        }
        sleep(1);
    }
}

void Test3() {
    digitalWrite(37, !digitalRead(37));

    unsigned long t0 = micros();
    delayMicroseconds(20);

    unsigned long t1 = micros();
    char str[30] = {0};

    oled.clearDisplay(); // 清屏
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    sprintf(str, "%ld  %ld ", t0, t1);
    oled.print(str);
    oled.setCursor(0, 20);
    sprintf(str, "%ld", t1 - t0);
    oled.print(str);
    oled.display();
}

//  测试中断测速
void Test4() {
    char str1[24] = {0};
    char str2[24] = {0};
    oled.clearDisplay(); // 清屏
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);

    while (true) {
        _MecanumDriver._PIDMoter[0].setMotor(0);
        sleep(1000);

        _MecanumDriver._PIDMoter[0].setPID(2, 1, 0);
        _MecanumDriver._PIDMoter[0].setMotor(70);

        char i = 0;
        while (true) {
            sprintf(str1, "%5d    %5d", _MecanumDriver._PIDMoter[0]._count,
                    _MecanumDriver._PIDMoter[1]._count);
            sprintf(str2, "%5d    %5d %3d", _MecanumDriver._PIDMoter[2]._count,
                    _MecanumDriver._PIDMoter[3]._count, i++);

            oled.clearDisplay(); // 清屏
            oled.setCursor(12, 10);
            oled.print(str1);
            oled.setCursor(12, 30);
            oled.print(str2);
            // 显示
            oled.display();
            sleep(100);

            if (digitalRead(PIN_MODE_TEST)) {
                break;
            }
        }
    }
}

void Test5() {
    // 测试辨色
    while (true) {
        int n = BianSe_K210();
        oled.clearDisplay(); // 清屏
        oled.setCursor(12, 10);
        if (n == 1) {

            oled.print("white");

        } else if (n == 2) {
            oled.print("yellow");

        } else {
            oled.print("unknown");
        }
        oled.display();
    }
}

void Test6() {
    // 测试分类舵机角度
    while (true) {
        servoFenLei.write(30);
        sleep(800);
        servoFenLei.write(141);
        sleep(800);
    }
}

//  测试PID
void Test7() {
    char str1[24] = {0};
    oled.clearDisplay(); // 清屏
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);

    _MecanumDriver._PIDMoter[1].setPID(2, 1, 0);
    _MecanumDriver._PIDMoter[1].setMotor(70);
    while (true) {

        sleep(100);
    }
}

//  测试PID巡线
void Test8() { motionX_forward(1); }

int readXuanZhuanBianMaQi() {
    // 旋转编码器输出判断用——记录前一次信号状态
    static int previousOutputClk = LOW;

    int codeA = digitalRead(PIN_CODE_CLK);
    int codeB = digitalRead(PIN_CODE_D);

    if (previousOutputClk == HIGH && codeA == LOW) {
        previousOutputClk = codeA;
        delay(50);
        if (codeA != codeB) {
            return -1;
        } else {
            return 1;
        }
    }

    previousOutputClk = codeA;
    return 0;
}

int buttonDown() {
    // 旋转编码器输出判断用——记录前一次信号状态
    static int previousOutputSW = LOW;
    int codeSW = digitalRead(PIN_CODE_SW);

    if (previousOutputSW == HIGH && codeSW == LOW) {
        delay(50);
        previousOutputSW = codeSW;
        return 1;
    }

    previousOutputSW = codeSW;
    return 0;
}

bool timePassed(unsigned long &t, int diff) {
    unsigned long t1 = millis();
    if (t1 - t >= diff) {
        t = t1;
        return true;
    }
    return false;
}
