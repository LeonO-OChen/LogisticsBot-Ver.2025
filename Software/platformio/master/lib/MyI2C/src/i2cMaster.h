#include <Arduino.h>

#ifndef I2C_H
#define I2C_H

class I2C_Master {
private:


public:
  void init(int sda ,int scl, uint32_t freq);

  //写入一个字节的数据（addr：i2c设备地址 val：数据内容）
  bool WriteByte(uint8_t addr, uint8_t val);

  //写入指定长度的数据（addr：i2c设备地址 reg：寄存器地址  val：数据内容指针  len：数据长度）
  bool WriteDataArray(uint8_t addr, uint8_t reg, uint8_t *val, uint8_t len);

  //读取一个字节的数据（addr：i2c设备地址 reg：寄存器地址  val：数据内容）
  bool ReadDataByte(uint8_t addr, uint8_t reg, uint8_t &val);

  //读取指定长度的数据（addr：i2c设备地址 reg：寄存器地址  val：接收数据内容指针 len：数据长度）
  int ReadDataArray(uint8_t addr, uint8_t reg, uint8_t *val, uint8_t len);
};

#endif