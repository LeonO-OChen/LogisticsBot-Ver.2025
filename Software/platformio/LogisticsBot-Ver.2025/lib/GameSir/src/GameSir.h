#include <Arduino.h>

/*
GAMESIR_T1S蓝牙手柄

         0  1  2  3  4  5  6  7  8  9 10 11
        A1 C4 80 80 80 80 00 00 00 00 00 00
L1 	    A1 C4 80 80 80 80 00 00 40 00 00 00
L2 	    A1 C4 80 80 80 80 XX 00 00 01 00 00
R1	    A1 C4 80 80 80 80 00 00 80 00 00 00
R2	    A1 C4 80 80 80 80 00 XX 00 02 00 00

UP	    A1 C4 80 80 80 80 00 00 00 00 01 00
DN	    A1 C4 80 80 80 80 00 00 00 00 05 00
LF	    A1 C4 80 80 80 80 00 00 00 00 07 00
RT      A1 C4 80 80 80 80 00 00 00 00 03 00

UPLF    A1 C4 80 80 80 80 00 00 00 00 08 00
UPRT    A1 C4 80 80 80 80 00 00 00 00 02 00
DNLF	  A1 C4 80 80 80 80 00 00 00 00 06 00
DNRT	  A1 C4 80 80 80 80 00 00 00 00 04 00

A	      A1 C4 80 80 80 80 00 00 01 00 00 00
B	      A1 C4 80 80 80 80 00 00 03 00 00 00
X	      A1 C4 80 80 80 80 00 08 00 00 00 00
Y	      A1 C4 80 80 80 80 00 10 00 00 00 00
LEFT STICK
  LFRT	A1 C4 XX 80 80 80 00 00 00 00 00 00     00~FF
  UPDN	A1 C4 80 XX 80 80 00 00 00 00 00 00     00~FF
RIGHT STICK
  LFRT	A1 C4 80 80 XX 80 00 00 00 00 00 00     00~FF
  UPDN  A1 C4 80 80 80 XX 00 00 00 00 00 00     00~FF
sel	    A1 C4 80 80 80 80 00 00 00 04 00 00
start	  A1 C4 80 80 80 80 00 00 00 08 00 00
*/

#ifndef GAME_SIR_H
#define GAME_SIR_H

#define KEY_L1 1
#define KEY_L2 2
#define KEY_R1 3
#define KEY_R2 4
#define KEY_A 5
#define KEY_B 6
#define KEY_X 7
#define KEY_Y 8
#define KEY_SEL 9
#define KEY_START 10
#define CAP_UP 11
#define CAP_DN 12
#define CAP_LF 13
#define CAP_RT 14
#define CAP_UPLF 15
#define CAP_UPRT 16
#define CAP_DNLF 17
#define CAP_DNRT 18

#define A_LEFT_STICK_H 19  // 水平
#define A_LEFT_STICK_V 20  // 竖直
#define A_RIGHT_STICK_H 21 // 水平
#define A_RIGHT_STICK_V 22 // 竖直
#define A_L2 23
#define A_R2 24

bool testKey(std::string data, int key);
bool testValue(std::string data, int analogKey, uint8_t &value);

#endif