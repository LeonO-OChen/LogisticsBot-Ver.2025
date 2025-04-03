#include "BLEDevice.h"
#include <Arduino.h>

#ifndef MY_BLE_H
#define MY_BLE_H

// 根据以下信息自动连接蓝牙手柄
// 蓝牙手柄的MAC地址
const static String BLE_SERVER_MAC = "86:55:a2:c0:8d:0d";
// 蓝牙手柄的服务UUID
const static BLEUUID BLE_SERVER_SERVICE_UUID("00008650-0000-1000-8000-00805f9b34fb");
// 蓝牙手柄的特征UUID
const static BLEUUID BLE_SERVER_CHAR_UUID("00008651-0000-1000-8000-00805f9b34fb");

class MyClientCallback;
class MyAdvertisedDeviceCallbacks;

class MyBleClient
{
    static MyBleClient *m_pMyBleClient;
    BLEAdvertisedDevice *_pDevice = nullptr;
    BLERemoteCharacteristic *_pRemoteCharacteristic;

    boolean _doConnect = false;
    boolean _connected = false;
    boolean _doScan = false;

    // 回调函数：当收到通知
    static void notifyCallback(
        BLERemoteCharacteristic *pBLERemoteCharacteristic,
        uint8_t *pData,
        size_t length,
        bool isNotify);

    bool connectToServer();

    friend MyClientCallback;
    friend MyAdvertisedDeviceCallbacks;
    static void runBackend(void *param);

public:
    static MyBleClient *getInstance();
    MyBleClient();
    void init();
    void autoConnect();
    bool isConnected();
    std::string read();
};

class MyClientCallback : public BLEClientCallbacks
{
    MyBleClient *_pBleClient;

public:
    MyClientCallback(MyBleClient *pBleClient)
    {
        _pBleClient = pBleClient;
    }

    void onConnect(BLEClient *pclient)
    {
    }

    void onDisconnect(BLEClient *pclient)
    {
        _pBleClient->_connected = false;
    }
};

// 收到广播后，会调用此类
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    MyBleClient *_pBleClient;

public:
    MyAdvertisedDeviceCallbacks(MyBleClient *pBleClient)
    {
        _pBleClient = pBleClient;
    }

    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        Serial.print("found device:");

        // 找到一个蓝牙设备，看看他是否是要连接的蓝牙手柄
        BLEAdvertisedDevice *device = new BLEAdvertisedDevice(advertisedDevice);

        Serial.println(device->getAddress().toString().c_str());

        if (BLE_SERVER_MAC.equals(device->getAddress().toString().c_str())) {
            Serial.println("Target BLE device found!");
            // 找到了！
            // 停止扫描
            BLEDevice::getScan()->stop();
            _pBleClient->_pDevice = new BLEAdvertisedDevice(advertisedDevice);
            _pBleClient->_doConnect = true; // 准备连接
            _pBleClient->_doScan = true;    // 可以扫描——如果连接失败的话
        }
    }
};

#endif