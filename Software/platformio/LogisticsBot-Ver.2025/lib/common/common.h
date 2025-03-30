#include <Arduino.h>

#ifndef COMMON_H
#define COMMON_H

// 判断从指定时间的开始，有没有到达diff毫秒
bool timePassed(unsigned long &t, int32_t diff);

#endif