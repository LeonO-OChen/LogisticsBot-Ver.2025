#include <Arduino.h>

/*
规定主机端向本模块传输的数据必须是以下格式：
指定地址写：
    REG地址，数据……
*/

#ifndef MSDriver_Common_H
#define MSDriver_Common_H

enum MSD_REG_ADDR {
    ID = 0x00,  // 00H(R/-)
    CMD = 0x01, // 01H(R/W): 0x01:reset; else:apply
    //---------------------------------------------------------------
    // 模式寄存器
    // 默认状态：电机不测速，AB引脚和舵机引脚都作为输入口
    //---------------------------------------------------------------
    M0_MODE = 0x04,    /* 04H(R/W):电机工作模式(M0) */
    M1_MODE = 0x05,    /* 05H(R/W):电机工作模式(M1) */
    M2_MODE = 0x06,    /* 06H(R/W):电机工作模式(M2) */
    M3_MODE = 0x07,    /* 07H(R/W):电机工作模式(M3) */
    S_MODE0123 = 0x08, /* 08H(R/W):舵机工作模式 b7b6:S3; b5b4b:S2; 3b2:S1; b1b0:S0 */
    S_MODE4567 = 0x09, /* 09H(R/W):舵机工作模式 b7b6:S7; b5b4b:S6; 3b2:S5; b1b0:S4 */

    M0_KP = 0x0C, /*  0CH(R/W):KP (float) */
    M0_KI = 0x10, /*  10H(R/W):KI (float) */
    M0_KD = 0x14, /*  14H(R/W):KD (float) */
    M0_KR = 0x18, /*  18H(R/W):M0电机转速系数(float) */

    M1_KP = 0x1C, /*  1CH(R/W):KP (float) */
    M1_KI = 0x20, /*  20H(R/W):KI (float) */
    M1_KD = 0x24, /*  24H(R/W):KD (float) */
    M1_KR = 0x28, /*  28H(R/W):M1电机转速系数(float) */

    M2_KP = 0x2C, /*  2CH(R/W):KP (float) */
    M2_KI = 0x30, /*  30H(R/W):KI (float) */
    M2_KD = 0x34, /*  34H(R/W):KD (float) */
    M2_KR = 0x38, /*  38H(R/W):M2电机转速系数(float) */

    M3_KP = 0x3C, /*  3CH(R/W):KP (float) */
    M3_KI = 0x40, /*  40H(R/W):KI (float) */
    M3_KD = 0x44, /*  44H(R/W):KD (float) */
    M3_KR = 0x48, /*  48H(R/W):M3电机转速系数(float) */

    //---------------------------------------------------------------
    // 控制寄存器
    //---------------------------------------------------------------
    M0_SPEED = 0x4C, /* 4CH(R/W):M0电机目标转速  (int16 : -255 ~ 255) */
    M1_SPEED = 0x4E, /* 4EH(R/W):M1电机目标转速  (int16 : -255 ~ 255) */
    M2_SPEED = 0x50, /* 50H(R/W):M2电机目标转速  (int16 : -255 ~ 255) */
    M3_SPEED = 0x52, /* 52H(R/W):M3电机目标转速  (int16 : -255 ~ 255) */

    S0_ANGLE = 0x54, /* 54H(R/W):S0舵机目标角度 (uint8 :0~180) */
    S1_ANGLE = 0x55, /* 55H(R/W):S1舵机目标角度 (uint8 :0~180) */
    S2_ANGLE = 0x56, /* 56H(R/W):S2舵机目标角度 (uint8 :0~180) */
    S3_ANGLE = 0x57, /* 57H(R/W):S3舵机目标角度 (uint8 :0~180) */
    S4_ANGLE = 0x58, /* 58H(R/W):S4舵机目标角度 (uint8 :0~180) */
    S5_ANGLE = 0x59, /* 59H(R/W):S5舵机目标角度 (uint8 :0~180) */
    S6_ANGLE = 0x5A, /* 5AH(R/W):S6舵机目标角度 (uint8 :0~180) */
    S7_ANGLE = 0x5B, /* 5BH(R/W):S7舵机目标角度 (uint8 :0~180) */

    //---------------------------------------------------------------
    // 反馈寄存器
    //---------------------------------------------------------------
    M0_SPEED_RA = 0x5C, /* 5CH(R/W):M0电机实际转速 (int32:当前计数) */
    M0_SPEED_RB = 0x5E, /* 5EH(R/W):M0的AB作为IO口时，B脚的输入 */
    M1_SPEED_RA = 0x60, /* 60H(R/W):M1电机实际转速 (int32:当前计数) */
    M1_SPEED_RB = 0x62, /* 62H(R/W):M1的AB作为IO口时，B脚的输入 */
    M2_SPEED_RA = 0x64, /* 64H(R/W):M2电机实际转速 (int32:当前计数) */
    M2_SPEED_RB = 0x66, /* 66H(R/W):M2的AB作为IO口时，B脚的输入 */
    M3_SPEED_RA = 0x68, /* 68H(R/W):M3电机实际转速 (int32:当前计数) */
    M3_SPEED_RB = 0x6A, /* 6AH(R/W):M3的AB作为IO口时，B脚的输入 */
    S0_ANGLE_R = 0x6C,  /* 6CH(R/-):S0舵机实际角度 (uint16 :) */
    S1_ANGLE_R = 0x6E,  /* 6EH(R/-):S0舵机实际角度 (uint16 :) */
    S2_ANGLE_R = 0x70,  /* 70H(R/-):S0舵机实际角度 (uint16 :) */
    S3_ANGLE_R = 0x72,  /* 72H(R/-):S0舵机实际角度 (uint16 :) */
    S4_ANGLE_R = 0x74,  /* 74H(R/-):S0舵机实际角度 (uint16 :) */
    S5_ANGLE_R = 0x76,  /* 76H(R/-):S0舵机实际角度 (uint16 :) */
    S6_ANGLE_R = 0x78,  /* 78H(R/-):S0舵机实际角度 (uint16 :) */
    S7_ANGLE_R = 0x7A,  /* 7AH(R/-):S0舵机实际角度 (uint16 :) */

    END = 0x7B /* 最终地址 */
};

typedef struct {
    /* 电机工作模式
    s7:测速使能位
    s3:电机禁用位
    0xxx xxxx: 默认 无需测速，相应的AB引脚作为IO口
        0xxx x000: 相应的A脚作为数字输入 INPUT
        0xxx x001: 相应的A脚作为模拟输入 INPUT
        0xxx x010: 相应的A脚作为输入 INPUT_PLLDOWN
        0xxx x011: 相应的A脚作为输入 INPUT_PULLUP
        0xxx x100: 相应的A脚作为输出 OUTPUT LOW
        0xxx x101: 相应的A脚作为输出 OUTPUT HIGH
        0xxx x110: 相应的A脚作为输出 OUTPUT_OPEN_DRAIN LOW
        0xxx x111: 相应的A脚作为输出 OUTPUT_OPEN_DRAIN HIGH

        0000 xxxx: 相应的B脚作为数字输入 INPUT
        0001 xxxx: 相应的B脚作为模拟输入 INPUT
        0010 xxxx: 相应的B脚作为输入 INPUT_PLLDOWN
        0011 xxxx: 相应的B脚作为输入 INPUT_PULLUP
        0100 xxxx: 相应的B脚作为输出 OUTPUT LOW
        0101 xxxx: 相应的B脚作为输出 OUTPUT HIGH
        0110 xxxx: 相应的B脚作为输出 OUTPUT_OPEN_DRAIN LOW
        0111 xxxx: 相应的B脚作为输出 OUTPUT_OPEN_DRAIN HIGH

    1xxx xxxx: 需要测速，AB作为测速中断
        10xx xxxx: 计数不自动清零
        11xx xxxx: 转换成100ms的计数
        1xxx xxx0: 无PID控制
        11xx xxx1: 需PID控制
    */
    uint8_t mMode[4]; /* 电机工作模式(M0~M3) */

    /* 舵机工作模式
    00:默认 INPUT
    01:输出 高低电平
    10:输出 PWM
    11:舵机模式
    */
    uint8_t smode0123; /* 03H(R/W): 舵机工作模式(S0 ~ S3) */
    uint8_t smode4567; /* 04H(R/W): 舵机工作模式(S4 ~ S7) */
    uint16_t NA;       /* 占位——由于编译器会结构对齐，使数据长度保持成4的整数倍（32位系统），这里被跳过了2个字节 */

    float m0Kp; /* 05H(R/W): KP */
    float m0Ki; /* 09H(R/W): KI */
    float m0Kd; /* 0DH(R/W): KD */
    float m0KR; /* 11H(R/W): M0电机转速系数 */

    float m1Kp; /* 05H(R/W): KP */
    float m1Ki; /* 09H(R/W): KI */
    float m1Kd; /* 0DH(R/W): KD */
    float m1KR; /* 11H(R/W): M1电机转速系数 */

    float m2Kp; /* 05H(R/W): KP */
    float m2Ki; /* 09H(R/W): KI */
    float m2Kd; /* 0DH(R/W): KD */
    float m2KR; /* 11H(R/W): M2电机转速系数 */

    float m3Kp; /* 05H(R/W): KP */
    float m3Ki; /* 09H(R/W): KI */
    float m3Kd; /* 0DH(R/W): KD */
    float m3KR; /* 11H(R/W): M3电机转速系数 */
} MSDriverReg_MOD_t;

typedef struct {
    /* 21H(R/W): M0电机转速 (-255 ~ 255) */
    /* 23H(R/W): M1电机转速 (-255 ~ 255) */
    /* 25H(R/W): M2电机转速 (-255 ~ 255) */
    /* 27H(R/W): M3电机转速 (-255 ~ 255) */
    int16_t speedM[4];

    /* 29H(R/W): S0舵机角度 (0 ~ 180) 或 (0 ~ 255) */
    /* 2AH(R/W): S1舵机角度 (0 ~ 180) 或 (0 ~ 255) */
    /* 2BH(R/W): S2舵机角度 (0 ~ 180) 或 (0 ~ 255) */
    /* 2CH(R/W): S3舵机角度 (0 ~ 180) 或 (0 ~ 255) */
    /* 2DH(R/W): S4舵机角度 (0 ~ 180) 或 (0 ~ 255) */
    /* 2EH(R/W): S5舵机角度 (0 ~ 180) 或 (0 ~ 255) */
    /* 2FH(R/W): S6舵机角度 (0 ~ 180) 或 (0 ~ 255) */
    /* 30H(R/W): S7舵机角度 (0 ~ 180) 或 (0 ~ 255) */
    uint8_t angleS[8];
} MSDriverReg_CTL_t;

typedef struct {
    /* 31H(R/W): M0电机实际转速 */
    /* 35H(R/W): M1电机实际转速 */
    /* 39H(R/W): M2电机实际转速 */
    /* 3DH(R/W): M3电机实际转速 */
    int32_t currspeedM[4];

    /* 41H(R/-): S0舵机当前角度 */
    /* 43H(R/-): S2舵机当前角度 */
    /* 45H(R/-): S4舵机当前角度 */
    /* 47H(R/-): S6舵机当前角度 */
    /* 49H(R/-): S0舵机当前角度 */
    /* 4BH(R/-): S2舵机当前角度 */
    /* 4DH(R/-): S4舵机当前角度 */
    /* 4FH(R/-): S6舵机当前角度 */
    uint16_t currValueS[8];
} MSDriverReg_FB_t;

// 指令
enum MSD_CMD {
    INIT = 0x0E, // 00H(R/-)
    APPLY = 0x01,  // 应用：变更电机/舵机模式
};

#endif