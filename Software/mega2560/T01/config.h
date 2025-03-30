/*
坏引脚：25

*/

#include <Arduino.h>


#define interrupts() sei()
#define noInterrupts() cli()

// OLED显示及其引脚定义
#define OLED_SCREEN_WIDTH 128   // oled屏幕宽度
#define OLED_SCREEN_HEIGHT 64   // oled屏幕高度
// IIC
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET     -1
// SPI
// #define PIN_OLED_MOSI  51   // 输出       接D1
// #define PIN_OLED_CLK   52   // 时钟       接D0
// #define PIN_OLED_DC    48   // 控制/显示  接DC
// #define PIN_OLED_CS    53   // 片选       接DCS
// #define PIN_OLED_RESET 49   // 复位       接RES

#define PIN_MODE_TEST 24   // 测试模式引脚（1：测试 0：正式)
// 测试模式下，抓取动作不执行
// 正式模式下，OLED不显示


#define PIN_DISP_FUN1 26   // 显示工作状态：排空
#define PIN_DISP_FUN2 27   // 显示工作状态：装货
#define PIN_DISP_FUN3 28   // 显示工作状态：起点
#define PIN_DISP_FUN4 29   // 显示工作状态：中间1
#define PIN_DISP_FUN5 45   // 显示工作状态：中间2
#define PIN_DISP_FUN6 44   // 显示工作状态：中间3
#define PIN_DISP_FUN7 43   // 显示工作状态：遥控
#define PIN_DISP_FUN8 42   // 显示工作状态：测试

// 旋转编码开关
#define PIN_CODE_CLK 34   // 时钟
#define PIN_CODE_D 35     // 数字输入
#define PIN_CODE_SW 36    // 开关

// 巡迹模块引脚
#define PIN_XUNJI_1 A0
#define PIN_XUNJI_2 A1
#define PIN_XUNJI_3 A2
#define PIN_XUNJI_4 A3

#define PIN_XUNJI_5 A7
#define PIN_XUNJI_6 A6
#define PIN_XUNJI_7 A5
#define PIN_XUNJI_8 A4

#define PIN_XUNJI_9 A8
#define PIN_XUNJI_10 A14
#define PIN_XUNJI_11 A13
#define PIN_XUNJI_12 A12

// 光敏电阻引脚
#define PIN_GUANGMIN_1 A11  // 白
#define PIN_GUANGMIN_2 A15  // 黄
#define PIN_GUANGMIN_3 A8   // 辨色

// 微动开关
#define PIN_WEIDONG_1 32
#define PIN_WEIDONG_2 31
#define PIN_WEIDONG_3 30

// 舵机引脚
#define PIN_SERVO_1 13      // 抓斗左
#define PIN_SERVO_2 10       // 抓斗右
#define PIN_SERVO_3 9      // 分类
#define PIN_SERVO_4 12      // 装填左
#define PIN_SERVO_5 11      // 状态右

// 编码电机0
#define PIN_M0_SPD1 2   // 蓝 接电机信号线1 必须接外部中断
#define PIN_M0_SPD2 14  // 紫 接电机信号线2
#define PIN_M0_PWM 5    // 接电机驱动，控制速度
#define PIN_M0_F1 46    // 接电机驱动，方向1
#define PIN_M0_F2 47    // 接电机驱动，方向2

// 编码电机1
#define PIN_M1_SPD1 3   // 蓝 接电机信号线1 必须接外部中断
#define PIN_M1_SPD2 15  // 紫 接电机信号线2
#define PIN_M1_PWM 8    // 接电机驱动，控制速度
#define PIN_M1_F1 48   // 接电机驱动，方向1
#define PIN_M1_F2 49    // 接电机驱动，方向2

// 编码电机2
#define PIN_M2_SPD1 18  // 蓝 接电机信号线1 必须接外部中断
#define PIN_M2_SPD2 16  // 紫 接电机信号线2
#define PIN_M2_PWM 7    // 接电机驱动，控制速度
#define PIN_M2_F1 38    // 接电机驱动，方向1
#define PIN_M2_F2 39    // 接电机驱动，方向2

// 编码电机3
#define PIN_M3_SPD1 19  // 蓝 接电机信号线1 必须接外部中断
#define PIN_M3_SPD2 17  // 紫 接电机信号线2
#define PIN_M3_PWM 6    // 接电机驱动，控制速度
#define PIN_M3_F1 40    // 接电机驱动，方向1
#define PIN_M3_F2 41    // 接电机驱动，方向2

// 风扇
#define PIN_FAN_1 51    // 风扇1
#define PIN_FAN_2 50   // 风扇2

#define VEHICLE_SHEELBASE  27     // 轴距(cm)
#define VEHICLE_TREAD      26     // 轮距(cm)