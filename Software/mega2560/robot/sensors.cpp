#include "mecanum.h"
#include "config.h"
#include <Servo.h>  // Timer5 舵机
#include "SCoop.h"  // 多线程


extern int _XunJiChuanGanQi[13];                        // 12路巡迹传感器的数值 (0~1023),下标0不用
extern int _GuangMin[4];                                // 灰度辨色 (0~1023),  0,1,2,3: 不用，白，黄，辨色
extern int _WeiDongKaiGuan[4];                          // 3路微动开关 (0~1),下标0不用


// 读取12个循迹传感器的数值
/*
   1 2 3 4
9    前    11
  左    右 
10   后    12
   5 6 7 8
*/
void readXunJiChuanGanQi() {
  _XunJiChuanGanQi[1] = analogRead(PIN_XUNJI_1);
  _XunJiChuanGanQi[2] = analogRead(PIN_XUNJI_2);
  _XunJiChuanGanQi[3] = analogRead(PIN_XUNJI_3);
  _XunJiChuanGanQi[4] = analogRead(PIN_XUNJI_4);
  _XunJiChuanGanQi[5] = analogRead(PIN_XUNJI_5);
  _XunJiChuanGanQi[6] = analogRead(PIN_XUNJI_6);
  _XunJiChuanGanQi[7] = analogRead(PIN_XUNJI_7);
  _XunJiChuanGanQi[8] = analogRead(PIN_XUNJI_8);
//   _XunJiChuanGanQi[9] = analogRead(PIN_XUNJI_9);
//   _XunJiChuanGanQi[10] = analogRead(PIN_XUNJI_10);
//   _XunJiChuanGanQi[11] = analogRead(PIN_XUNJI_11);
//   _XunJiChuanGanQi[12] = analogRead(PIN_XUNJI_12);
}

// 读取3个光敏电阻的数值
// 分别用于颜色分类识别，出料器有无乒乓球识别
/*
      3
    1     2
*/
void readGuangMin() {
  _GuangMin[1] = analogRead(PIN_GUANGMIN_1);
  _GuangMin[2] = analogRead(PIN_GUANGMIN_2);
  _GuangMin[3] = analogRead(PIN_GUANGMIN_3);
}

// 读取3个微动开关的数值 0：放开，1：按下
/*
    1
       2   3
*/
void readWeiDongKaiGuan() {
  _WeiDongKaiGuan[1] = 1- digitalRead(PIN_WEIDONG_1);
  _WeiDongKaiGuan[2] = 1- digitalRead(PIN_WEIDONG_2);
  _WeiDongKaiGuan[3] = 1- digitalRead(PIN_WEIDONG_3);
}
