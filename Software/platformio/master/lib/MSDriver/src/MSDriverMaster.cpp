#include "MSDriverMaster.h"
#include "i2cMaster.h"

extern I2C_Master _i2c;

/**
 * 初始化模块
 */
void MSDriverMaster::init(uint8_t addr)
{
    i2c_addr = addr;
}

// 发送命令，使设定生效
void MSDriverMaster::sendCmd(MSD_CMD cmd)
{
    uint8_t data = cmd;
    writeReg(CMD, &data, 1);
}

/*
设置指定电机的工作模式，num=-1时，设置所有电机

 电机工作模式
    s7:测速使能位
    s3:电机使能位
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
void MSDriverMaster::setMotorMode(int num, uint8_t &mode)
{
    if (num == 0 || num == -1) {
        writeReg(M0_MODE, &mode, 1);
    }
    if (num == 1 || num == -1) {
        writeReg(M1_MODE, &mode, 1);
    }
    if (num == 2 || num == -1) {
        writeReg(M2_MODE, &mode, 1);
    }
    if (num == 3 || num == -1) {
        writeReg(M3_MODE, &mode, 1);
    }
}

/**
 *
 * 设置指定舵机的PID参数，num=-1时，设置所有电机
 *
 * kr(转速系数)——PID控制时必须设置，用于设置目标转速(目标转速计数 = 转速系数 * 目标速度(-255 ~ 255))
 *     转速系数计算方法：
 *         转速系数 = 编码电机的分辨率 * 减速比 * 额定转速 * 4(4倍频测速)/600/255
 *         假设编码电机的分辨率(电机转一圈计数)是13，减速比是90(电机转90圈，输出轴转1圈)，额定转速是180rpm，
 *         采用4倍频(AB引脚的上升下降沿都参与)计数，
 *         则每转一圈计数13*90*4=4680次，
 *         每分钟计数为4680*180=842400次，
 *         即每100ms为842400/60/10=1404次。
 *         转速系数=1404/255=5.5
 *
 *         因为 目标转速计数 = 转速系数 * 目标速度
 *         所以转速系数越小，目标转速越慢
 *
 */
void MSDriverMaster::setMotorPID(int num, float kp, float ki, float kd, float kr)
{
    MSDriverReg_MOD_t mode;
    memset((void *)&mode, 0, sizeof(mode));
    if (num == 0 || num == -1) {
        mode.m0Kp = kp;
        mode.m0Ki = ki;
        mode.m0Kd = kd;
        mode.m0KR = kr;
    }
    if (num == 1 || num == -1) {
        mode.m1Kp = kp;
        mode.m1Ki = ki;
        mode.m1Kd = kd;
        mode.m1KR = kr;
    }
    if (num == 2 || num == -1) {
        mode.m2Kp = kp;
        mode.m2Ki = ki;
        mode.m2Kd = kd;
        mode.m2KR = kr;
    }
    if (num == 3 || num == -1) {
        mode.m3Kp = kp;
        mode.m3Ki = ki;
        mode.m3Kd = kd;
        mode.m3KR = kr;
    }

    // 发送——防止对方缓存溢出，每发一次后暂停一会儿
    switch (num) {
    case 0:
        writeReg(M0_KP, (uint8_t *)&mode.m0Kp, sizeof(float));
        delay(10);
        writeReg(M0_KI, (uint8_t *)&mode.m0Ki, sizeof(float));
        delay(10);
        writeReg(M0_KD, (uint8_t *)&mode.m0Kd, sizeof(float));
        delay(10);
        writeReg(M0_KR, (uint8_t *)&mode.m0KR, sizeof(float));
        break;
    case 1:
        writeReg(M1_KP, (uint8_t *)&mode.m1Kp, sizeof(float));
        delay(10);
        writeReg(M1_KI, (uint8_t *)&mode.m1Ki, sizeof(float));
        delay(10);
        writeReg(M1_KD, (uint8_t *)&mode.m1Kd, sizeof(float));
        delay(10);
        writeReg(M1_KR, (uint8_t *)&mode.m1KR, sizeof(float));
        break;
    case 2:
        writeReg(M2_KP, (uint8_t *)&mode.m2Kp, sizeof(float));
        delay(10);
        writeReg(M2_KI, (uint8_t *)&mode.m2Ki, sizeof(float));
        delay(10);
        writeReg(M2_KD, (uint8_t *)&mode.m2Kd, sizeof(float));
        delay(10);
        writeReg(M2_KR, (uint8_t *)&mode.m2KR, sizeof(float));
        break;
    case 3:
        writeReg(M3_KP, (uint8_t *)&mode.m3Kp, sizeof(float));
        delay(10);
        writeReg(M3_KI, (uint8_t *)&mode.m3Ki, sizeof(float));
        delay(10);
        writeReg(M3_KD, (uint8_t *)&mode.m3Kd, sizeof(float));
        delay(10);
        writeReg(M3_KR, (uint8_t *)&mode.m3KR, sizeof(float));
        break;
    default:
        writeReg(M0_KP, (uint8_t *)&mode.m0Kp, sizeof(float));
        delay(10);
        writeReg(M0_KI, (uint8_t *)&mode.m0Ki, sizeof(float));
        delay(10);
        writeReg(M0_KD, (uint8_t *)&mode.m0Kd, sizeof(float));
        delay(10);
        writeReg(M0_KR, (uint8_t *)&mode.m0KR, sizeof(float));
        delay(10);
        writeReg(M1_KP, (uint8_t *)&mode.m1Kp, sizeof(float));
        delay(10);
        writeReg(M1_KI, (uint8_t *)&mode.m1Ki, sizeof(float));
        delay(10);
        writeReg(M1_KD, (uint8_t *)&mode.m1Kd, sizeof(float));
        delay(10);
        writeReg(M1_KR, (uint8_t *)&mode.m1KR, sizeof(float));
        delay(10);
        writeReg(M2_KP, (uint8_t *)&mode.m2Kp, sizeof(float));
        delay(10);
        writeReg(M2_KI, (uint8_t *)&mode.m2Ki, sizeof(float));
        delay(10);
        writeReg(M2_KD, (uint8_t *)&mode.m2Kd, sizeof(float));
        delay(10);
        writeReg(M2_KR, (uint8_t *)&mode.m2KR, sizeof(float));
        delay(10);
        writeReg(M3_KP, (uint8_t *)&mode.m3Kp, sizeof(float));
        delay(10);
        writeReg(M3_KI, (uint8_t *)&mode.m3Ki, sizeof(float));
        delay(10);
        writeReg(M3_KD, (uint8_t *)&mode.m3Kd, sizeof(float));
        delay(10);
        writeReg(M3_KR, (uint8_t *)&mode.m3KR, sizeof(float));
    }
}

/**
 *
 * 设置指定舵机的工作模式，num=-1时，设置所有电机
 *
 *  00:默认 INPUT
 *  01:输出 高低电平
 *  10:输出 PWM
 *  11:舵机模式
 */
void MSDriverMaster::setServoMode(int num, const uint8_t &mode)
{
    uint8_t mode2;
    if (num == 0 || num == -1) {
        mode2 = mode;
        servoMode0123 = (servoMode0123 & 0b11111100) | (mode2 & 0b00000011);
    }
    if (num == 1 || num == -1) {
        mode2 = mode << 2;
        servoMode0123 = (servoMode0123 & 0b11110011) | (mode2 & 0b00001100);
    }
    if (num == 2 || num == -1) {
        mode2 = mode << 4;
        servoMode0123 = (servoMode0123 & 0b11001111) | (mode2 & 0b00110000);
    }
    if (num == 3 || num == -1) {
        mode2 = mode << 6;
        servoMode0123 = (servoMode0123 & 0b00111111) | (mode2 & 0b11000000);
    }

    if (num == 4 || num == -1) {
        mode2 = mode;
        servoMode4567 = (servoMode4567 & 0b11111100) | (mode2 & 0b00000011);
    }
    if (num == 5 || num == -1) {
        mode2 = mode << 2;
        servoMode4567 = (servoMode4567 & 0b11110011) | (mode2 & 0b00001100);
    }
    if (num == 6 || num == -1) {
        mode2 = mode << 4;
        servoMode4567 = (servoMode4567 & 0b11001111) | (mode2 & 0b00110000);
    }
    if (num == 7 || num == -1) {
        mode2 = mode << 6;
        servoMode4567 = (servoMode4567 & 0b00111111) | (mode2 & 0b11000000);
    }

    if (num == -1) {
        writeReg(S_MODE0123, (uint8_t *)&servoMode0123, 1);
        writeReg(S_MODE4567, (uint8_t *)&servoMode4567, 1);
    } else if (num <= 3) {
        writeReg(S_MODE0123, (uint8_t *)&servoMode0123, 1);
    } else if (num <= 7) {
        writeReg(S_MODE4567, (uint8_t *)&servoMode4567, 1);
    }
}

// num(0~3), speed(-100~100)
void MSDriverMaster::motor100(int8_t num, int16_t speed)
{
    int data = speed * 255 / 100;
    motor(num, data);
}

// num(0~3), pwm(-255~255)
void MSDriverMaster::motor(int8_t num, int16_t pwm)
{
    uint8_t reg = M0_SPEED + (num << 1); // 每个值占2个字节
    writeReg(reg, (uint8_t *)&pwm, 2);
}

void MSDriverMaster::motorBreak(int8_t num)
{
    uint8_t reg = M0_SPEED + (num << 1);
    uint16_t data = 0x0E0E;
    writeReg(reg, (uint8_t *)&data, 2);
}

void MSDriverMaster::motorRelease(int8_t num)
{
    uint8_t reg = M0_SPEED + (num << 1);
    uint16_t data = 0x0E00;
    writeReg(reg, (uint8_t *)&data, 2);
}

// num(0~7), degree(0~180)
void MSDriverMaster::servo(int8_t num, uint8_t angle)
{
    uint8_t reg = S0_ANGLE + num;
    int16_t data = angle;
    writeReg(reg, (uint8_t *)&data, 1);
}

/**
 * 获取电机的当前转速（仅在测速模式下使用）
 * 在非测速模式下，0、1字节代表引脚A的读数，2、3字节代表引脚B的读数
 */
bool MSDriverMaster::getValueM(int num, int32_t *val)
{
    if (num == -1) {
        return readReg(M0_SPEED_RA, (uint8_t *)val, sizeof(int32_t) * 4);
    } else {
        uint8_t reg = M0_SPEED_RA + (num << 2); // 数据占4字节
        return readReg(reg, (uint8_t *)val, sizeof(int32_t));
    }
}

/**
 * 获取电机引脚A的读数（仅在非测速模式下使用）
 * 在测速模式下，代表电机速度值的0、1字节
 */
bool MSDriverMaster::getValueMA(int num, uint16_t *val)
{
    uint8_t reg = M0_SPEED_RA + (num << 2); // 数据占4字节
    return readReg(reg, (uint8_t *)val, sizeof(uint16_t));
}

/**
 * 获取电机引脚B的读数（仅在非测速模式下使用）
 * 在测速模式下，代表电机速度值的2、3字节
 */
bool MSDriverMaster::getValueMB(int num, uint16_t *val)
{
    uint8_t reg = M0_SPEED_RB + (num << 2); // 数据占4字节
    return readReg(reg, (uint8_t *)val, sizeof(uint16_t));
}

/**
 * 获取舵机引脚的读数（仅在【00:默认INPUT】模式下有效）
 *
 */
bool MSDriverMaster::getValueS(int num, uint16_t *val)
{
    if (num == -1) {
        return readReg(S0_ANGLE_R, (uint8_t *)val, sizeof(uint16_t) * 8);
    } else {
        uint8_t reg = S0_ANGLE_R + (num << 1); // 数据占2字节
        return readReg(reg, (uint8_t *)val, sizeof(uint16_t));
    }
}

bool MSDriverMaster::readReg(uint8_t reg, uint8_t *val, uint8_t len)
{
    int read = _i2c.ReadDataArray(i2c_addr, reg, val, len);
    return (read == len);
}

void MSDriverMaster::writeReg(uint8_t reg, uint8_t *val, uint8_t len)
{
    _i2c.WriteDataArray(i2c_addr, reg, val, len);
}
