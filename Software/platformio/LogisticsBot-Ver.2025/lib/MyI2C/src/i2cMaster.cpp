#include <Arduino.h>
#include "i2cMaster.h"
#include "Wire.h"

void I2C_Master::init(int sda ,int scl, uint32_t freq){
    Wire.begin(sda, scl,freq); //初始化I2C
}

bool I2C_Master::WriteByte(uint8_t addr, uint8_t val) {
    Wire.beginTransmission(addr);
    Wire.write(val);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    return true;
}

//向地址中写入数据（reg：地址  val：数据内容  len：数据长度）
bool I2C_Master::WriteDataArray(uint8_t  addr, uint8_t reg, uint8_t *val,
                             uint8_t  len) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    for (uint8_t  i = 0; i < len; i++) {
        Wire.write(val[i]);
    }
    if (Wire.endTransmission() != 0) {
        return false;
    }

    return true;
}
//读取地址中的数据（reg：地址  val：数据内容）
bool I2C_Master::ReadDataByte(uint8_t addr, uint8_t reg, uint8_t &val) {
    if (!WriteByte(addr, reg)) {
        return false;
    }

    uint8_t size = 1;
    Wire.requestFrom(addr, size);
    while (Wire.available()) {
        val = Wire.read();
    }

    return true;
}

//读取地址中指定长度的数据（reg：地址  val：数据内容 len：数据长度）
int I2C_Master::ReadDataArray(uint8_t  addr, uint8_t reg, uint8_t *val,
                           uint8_t  len) {
    /* Indicate which register we want to read from */
    if (!WriteByte(addr, reg)) {
        return -1;
    }

    /* Read block data */
    uint8_t  i = 0;
    Wire.requestFrom(addr, len);
    while (Wire.available()) {
        if (i >= len) {
            return -1;
        }
        val[i] = Wire.read();
        i++;
    }

    return i;
}