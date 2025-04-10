/*
  CPU:  ESP32-S3
*/

#include "GameSir.h"
#include "MSDriverMaster.h"
#include "MyBle.h"
#include "common.h"
#include "config.h"
#include "freertos/FreeRTOS.h" // 多线程
#include "i2cMaster.h"         // I2C通信
#include <Adafruit_NeoPixel.h> // RGB LED灯
#include <Adafruit_SSD1306.h>  // OLED显示屏

I2C_Master _i2c;
// OLED显示屏
Adafruit_SSD1306 oled32(128, 32, &Wire, -1);
Adafruit_SSD1306 oled64(128, 64, &Wire, -1);
// RGB LED
Adafruit_NeoPixel rgbLED(1, 48, NEO_GRB + NEO_KHZ800);
// 蓝牙
MyBleClient *pMyBleClient;
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
    showLed(-1); // 数码管不显示

    // RGB LED灯
    rgbLED.begin();
    rgbLED.setBrightness(20);

    // 旋转编码器
    pinMode(PIN_CODE_CLK, INPUT);       // 时钟
    pinMode(PIN_CODE_D, INPUT);         // 数据
    pinMode(PIN_CODE_SW, INPUT_PULLUP); // 按钮
    // 设置外部中断
    attachInterrupt(PIN_CODE_CLK, handleEncoder, CHANGE);

    // 等待其它设备上电完毕
    delay(200);

    // 初始化OLED
    oled32.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    oled64.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    oled64.setRotation(2); // 上下翻转显示

    oled32.clearDisplay(); // 清屏
    oled32.setTextSize(2);
    oled32.setTextColor(SSD1306_WHITE);
    oled32.setCursor(40, 16);
    oled32.println("init...");
    oled32.display(); // 显示
    oled32.setTextSize(1);

    oled64.clearDisplay(); // 清屏
    oled64.setTextSize(2);
    oled64.setTextColor(SSD1306_WHITE);
    oled64.setCursor(40, 16);
    oled64.println("init...");
    oled64.display(); // 显示
    oled64.setTextSize(1);

    // 初始化电机驱动模块
    initMSDriver();

    _MSDriverMaster.motor(0, 180);
    _MSDriverMaster.motor(1, -180);
    _MSDriverMaster.motor(2, 180);
    _MSDriverMaster.motor(3, -180);

    // 蓝牙
    pMyBleClient = MyBleClient::getInstance();
    pMyBleClient->init();
    pMyBleClient->autoConnect(); // 自动连接

    // 子线程：显示状态
    xTaskCreatePinnedToCore(taskDisplay, "taskDisplay", 2048, NULL, 15, NULL, 0);

    // 结束初始化
    oled32.clearDisplay(); // 清屏
    oled32.display();      // 显示
    oled64.clearDisplay(); // 清屏
    oled64.display();      // 显示
}

// 主任务,交互
// 读取控制信号
// 执行动作
// 启动任务
void loop() {
    static char str[20];
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

    int32_t speed0 = 0;
    int32_t speed1 = 0;
    int32_t speed2 = 0;
    int32_t speed3 = 0;
    _MSDriverMaster.getValueM(0, &speed0);
    _MSDriverMaster.getValueM(1, &speed1);
    _MSDriverMaster.getValueM(2, &speed2);
    _MSDriverMaster.getValueM(3, &speed3);

    oled32.clearDisplay(); // 清屏
    sprintf(str, "%7d %7d\n", speed0, speed1);
    oled32.setCursor(10, 4);
    oled32.println(str);
    sprintf(str, "%7d %7d\n", speed2, speed3);
    oled32.setCursor(10, 20);
    oled32.println(str);
    oled32.display(); // 显示

    // readXunJiChuanGanQi();
    // readWeiDongKaiGuan();

    if (_Task) {
        delay(2000);
        _Task = 0;
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
}

// 从任务
// 数码管显示
void taskDisplay(void *param) {
    // 数码管每0.5s左右闪烁一次——闪烁频率
    int ledFlash = 0; // 闪烁
    int ledFlashCnt = 0;

    unsigned long t0 = millis();
    int s = 0;

    // Loop forever
    for (;;) {
        // 数码管
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

        // RGB灯
        // 每10ms执行
        // if (timePassed(t0, 500)) {
        if (s) {
            if (pMyBleClient->isConnected()) {
                // 闪蓝灯
                rgbLED.setPixelColor(0, rgbLED.Color(50, 50, 200));
            } else {
                // 闪红灯
                rgbLED.setPixelColor(0, rgbLED.Color(200, 50, 50));
            }
            s = 0;
        } else {
            rgbLED.setPixelColor(0, rgbLED.Color(0, 0, 0));
            s = 1;
        }
        rgbLED.show();
        //}

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
    // 176RPM电机：分辨率11，转速系数2.25
    _MSDriverMaster.setMotorPID(-1, 2, 0.00017, 16, 2.25);
    _MSDriverMaster.setMotorPID(2, 2, 0.00016, 16, 2.25);
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