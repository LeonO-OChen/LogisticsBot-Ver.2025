#include "GameSir.h"

bool testKey(std::string data, int key)
{
    if (data.length() != 12) {
        return false;
    }

    if (data[0] == 0xA1 && data[1] == 0xC4) {
        switch (key) {
        case KEY_L1:
            return (data[8] & 0x40);
        case KEY_R1:
            return (data[8] & 0x80);
        case KEY_L2:
            return (data[9] & 0x01);
        case KEY_R2:
            return (data[9] & 0x02);
        case KEY_A:
            return (data[8] & 0x01);
        }
    }

    return false;
}

bool testValue(std::string data, int analogKey, uint8_t &value)
{
    if (data.length() != 12) {
        return false;
    }

    if (data[0] == 0xA1 && data[1] == 0xC4) {
        switch (analogKey) {
        case A_LEFT_STICK_H:
            value = data[2];
            break;
        case A_LEFT_STICK_V:
            value = data[3];
            break;
        case A_RIGHT_STICK_H:
            value = data[4];
            break;
        case A_RIGHT_STICK_V:
            value = data[5];
            break;
        case A_L2:
            value = data[6];
            break;
        case A_R2:
            value = data[7];
            break;
        default:
            return false;
        }
        return true;
    }

    return false;
}