/*
  CPU:  ESP32-S3
*/

#include "MSDriverMaster.h"
#include "common.h"
#include "config.h"
#include "freertos/FreeRTOS.h" // 多线程
#include "i2cMaster.h"         // I2C通信

I2C_Master _i2c;

// 电机驱动模块
MSDriverMaster _MSDriverMaster;
int _Task = 0;            // 任务（1:排空 2：装货 3~6:送货 7:测试1 8:测试2）
volatile int _Select = 1; // 选择项 1~8  (使用volatile，因为会在中断中修改)
// 为0时数码管会闪烁——调整旋转编码器时会设置该值，使不闪烁
volatile int _ledCnt = 0;

void initMSDriver();
void showLed(int val);
void handleEncoder();
int buttonDown();
void taskDisplay(void *param);

void setup() {
    Serial.begin(115200); // 初始化串口
    _i2c.init(I2C_SDA, I2C_SCL, I2C_FREQ);

    // 数码管
    pinMode(PIN_LED_D, OUTPUT);
    pinMode(PIN_LED_CLK, OUTPUT);
    pinMode(PIN_LED_LOAD, OUTPUT);

    // 旋转编码器
    pinMode(PIN_CODE_CLK, INPUT);       // 时钟
    pinMode(PIN_CODE_D, INPUT);         // 数据
    pinMode(PIN_CODE_SW, INPUT_PULLUP); // 按钮
    // 设置外部中断
    attachInterrupt(PIN_CODE_CLK, handleEncoder, CHANGE);

    // 等待其它设备上电完毕
    delay(1000);

    initMSDriver();

    delay(1000);
    _MSDriverMaster.motor(0, 100);
    _MSDriverMaster.motor(1, 100);
    _MSDriverMaster.motor(2, 100);
    _MSDriverMaster.motor(3, 100);

    xTaskCreatePinnedToCore(taskDisplay, "taskDisplay", 2048, NULL, 15, NULL,
                            0);
}

// 主任务,交互
// 读取控制信号
// 执行动作
// 启动任务
void loop() {

    static unsigned long t0 = 0;

    // 读取选择编码器按钮
    // 选中的选项作为下一个任务
    if (buttonDown()) {
        _Task = _Select;
    }

    // 之后的程序每10ms才运行一次
    // 会影响到数码管的亮度
    if (!timePassed(t0, 10)) {
        return;
    }

    // readXunJiChuanGanQi();
    // readWeiDongKaiGuan();

    if (_Task) {
        delay(2000);
        _Task = 0;
    }
}
// if (_Task == 1) { // 排空
//     displayMode = 0;
//     empty();
//     _Task = 0;
// } else if (_Task == 2) { // 装货
//     displayMode = 0;
//     renwu1();

//     if (digitalRead(PIN_MODE_TEST)) {
//         _Task = 0;
//     } else {
//         _Task++;
//     }

// } else if (_Task == 3) { // 起点送货
//     displayMode = 0;
//     renwu2_1();
//     if (digitalRead(PIN_MODE_TEST)) {
//         _Task = 0;
//     } else {
//         _Task++;
//     }
//     _Select = _Task;
// } else if (_Task == 4) { // 中间1送货
//     displayMode = 0;
//     renwu2_2();
//     if (digitalRead(PIN_MODE_TEST)) {
//         _Task = 0;
//     } else {
//         _Task++;
//     }
//     _Select = _Task;
// } else if (_Task == 5) { // 中间2送货
//     displayMode = 0;
//     renwu3_1();
//     if (digitalRead(PIN_MODE_TEST)) {
//         _Task = 0;
//     } else {
//         _Task++;
//     }
//     _Select = _Task;
// } else if (_Task == 6) { // 中间3送货
//     displayMode = 0;
//     renwu3_2();
//     _Task = 0;
//     _Select = _Task;
// } else if (_Task == 7) { // 测试
//     displayMode = 0;
//     Test1();

//     displayMode = 0;
//     sleep(20);
//     oled.clearDisplay(); // 清屏
//     oled.display();
//     _Task = 0;

// } else if (_Task == 8) { // 测试2
//     displayMode = 0;
//     Test8();
//     displayMode = 0;
//     sleep(20);
//     oled.clearDisplay(); // 清屏
//     oled.display();
//     _Task = 0;
// }

// 从任务
// 数码管显示
void taskDisplay(void *param) {
    // 数码管每0.5s左右闪烁一次——闪烁频率
    int ledFlash = 0; // 闪烁
    int ledFlashCnt = 0;

    // Loop forever
    for (;;) {
        if (_Task == 0 && _ledCnt == 0) {
            // 不执行任务时： 指示灯闪烁
            if (ledFlashCnt-- == 0) {
                ledFlash = 1 - ledFlash;
                ledFlashCnt = 200;
            }
            if (ledFlash) {
                showLed(_Select);
            } else {
                showLed(-1);
            }
        } else {
            // 执行任务时： 指示灯常亮
            showLed(_Select);
        }

        if (_ledCnt) {
            _ledCnt--;
        }

        delay(2);
    }
}

// 4电机闭环控制，舵机全开
void initMSDriver() {
    // uint8_t motorMode = 0b11000001; // 测速，计数自动清零，PID控制
    uint8_t motorMode = 0b11000000; // 测速，计数自动清零，无PID控制

    uint8_t smode = 0b11; // 舵机模式
    _MSDriverMaster.init(0x32);
    // 设置所有电机工作模式
    _MSDriverMaster.setMotorMode(-1, motorMode);
    // 设置所有电机PID参数
    _MSDriverMaster.setMotorPID(-1, 0.6, 0.000001, 0, 2.8);
    // 设置所有舵机工作模式
    _MSDriverMaster.setServoMode(-1, smode);
    _MSDriverMaster.sendCmd(APPLY);
}

unsigned char smgduan[] = {
    // 共阳
    B11000000, // 0
    B11111001, // 1
    B10100100, // 2
    B10110000, // 3
    B10011001, // 4
    B10010010, // 5
    B10000010, // 6
    B11111000, // 7
    B10000000, // 8
    B10010000, // 9
    B01000000, // 0.
    B01111001, // 1.
    B00100100, // 2.
    B00110000, // 3.
    B00011001, // 4.
    B00010010, // 5.
    B00000010, // 6.
    B01111000, // 7.
    B00000000, // 8.
    B00010000, // 9.
    B11111111, // 空
}; // 显示0~9的值共阴

void showLed(int val) {

    static int ledFreq = 10;   // 频闪——每被调用10次点亮一次——控制亮度
    static int ledFreqCnt = 0; //

    if (ledFreqCnt == ledFreq) {
        // 熄灭
        digitalWrite(PIN_LED_LOAD, LOW); // 低电位表示启动
        shiftOut(PIN_LED_D, PIN_LED_CLK, MSBFIRST, smgduan[20]);
        shiftOut(PIN_LED_D, PIN_LED_CLK, MSBFIRST, smgduan[20]);
        digitalWrite(PIN_LED_LOAD, HIGH); // 高电位表示停止
        ledFreqCnt--;
    } else if (ledFreqCnt-- == 0) {
        // 默认不显示
        unsigned char gewei = 20;
        unsigned char shiwei = 20;
        
        // 只显示大于0的数
        if (val >= 0) {
            gewei = (val % 100) % 10;
            shiwei = (val % 100) / 10;

            // 十位上的0不显示
            if (shiwei == 0) {
                shiwei = 20;
            }
        }

        digitalWrite(PIN_LED_LOAD, LOW); // 低电位表示启动
        shiftOut(PIN_LED_D, PIN_LED_CLK, MSBFIRST, smgduan[gewei]);
        shiftOut(PIN_LED_D, PIN_LED_CLK, MSBFIRST, smgduan[shiwei]);
        digitalWrite(PIN_LED_LOAD, HIGH); // 高电位表示停止

        ledFreqCnt = ledFreq;
    }
}

void handleEncoder() {
    // 防抖动变量
    static unsigned long lastDebounceTime = 0;
    static unsigned long debounceDelay = 1000; // 微秒
    static int position = 0;
    int diff = 0;

    // 如果正在执行任务，则不读取
    if (_Task) {
        return;
    }

    // 防抖动处理
    if ((micros() - lastDebounceTime) < debounceDelay) {
        return;
    }
    lastDebounceTime = micros();

    // 读取两个引脚的状态
    int a = digitalRead(PIN_CODE_CLK);
    int b = digitalRead(PIN_CODE_D);

    // 根据A、B相的状态变化判断方向
    // 这里假设编码器是四分之一周期检测（每个变化都检测）
    if (a == b) {
        position--; // 顺时针
    } else {
        position++; // 逆时针
    }
    diff = position / 2;
    if (diff) {
        _Select += diff;
        position = 0;

        _Select = _Select < 8 ? _Select : 0;
        _Select = _Select >= 0 ? _Select : 7;
    }

    _ledCnt = 300; // 数码管暂时不闪烁
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