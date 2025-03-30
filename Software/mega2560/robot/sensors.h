#include "config.h"


// 读取12个循迹传感器的数值
/*
   1 2 3 4
9    前    11
  左    右 
10   后    12
   5 6 7 8
*/
void readXunJiChuanGanQi();

// 读取3个光敏电阻的数值
// 分别用于颜色分类识别，出料器有无乒乓球识别
/*
      3,4
    1     2
*/
void readGuangMin();

// 读取3个微动开关的数值 0：放开，1：按下
/*
    1
       2   3
*/
void readWeiDongKaiGuan();
