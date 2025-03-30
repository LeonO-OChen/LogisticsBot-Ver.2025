#include "config.h"

// 机器人的基本动作

#define DEFAULT_GEAR 3


// 绕轴旋转
// axis:旋转轴 0,1,2,3,4,5,6,7,8 (中心，前传感器,后传感器，左传感器，右传感器，左前轮，右前轮，左后轮，右后轮)
// gear:档位 -5~5 -- >0 逆时针转; <0 顺时针转
void motion_XuanZhuan(int axis, int gear);

// 平移
// 方向 direction=1，2，3，4：前，后，左，右
void motion_move(int direction, int gear);

// 停止
void motion_stop();

// 寻迹:平移
// 方向 direction=1，2，3，4：前，后，左，右
// 直到(g)传感器检测到(times)次黑线
// wait=true: 第一次检测前适当延时
void motionXJ_move(int direction,int g, int times, bool wait);

// 寻迹:原地旋转直到g传感器检测到线
// direction = 1左转, -1:右转
void motionXJ_turn(int direction, int g);

// 抓取
void motion_ZhuaQu();

// 将舵机转到指定位置
// op:0: 1开2闭，乒乓球可以进入识别区（初始位置)  
//    1: 1闭2开，乒乓球在入识别区外等待，已经在识别区的则装入发射管
//    2: 1开2开，全通过
void motion_setServo(int color, int op);

// 判断n传感器是不是在线上
bool detect(int n);

//顶箱子
void DingXiangZi();

//辨色
int BianSe_huidu();
int BianSe_K210();



/*****************************************************/
//
//
/*****************************************************/
bool B_atCross();
float B_Xforward() ;
// 巡线前进n格
void motionX_forward(int n);