#include "MyBle.h"
#include "freertos/FreeRTOS.h" // 多线程
#include "freertos/task.h"

MyBleClient *MyBleClient::m_pMyBleClient = nullptr;

MyBleClient::MyBleClient()
{
}

MyBleClient *MyBleClient::getInstance()
{
    if (m_pMyBleClient == nullptr) {
        m_pMyBleClient = new MyBleClient();
    }
    return m_pMyBleClient;
}

void MyBleClient::init()
{
    BLEDevice::init("");

    // 获取一个扫描器，设置回调函数
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
    pBLEScan->setInterval(1349);   // 扫描间隔(ms)
    pBLEScan->setWindow(449);      // 扫描窗口
    pBLEScan->setActiveScan(true); // 主动扫描

    _doConnect = false;
    _connected = false;
    _doScan = false;
}

// 从任务
void MyBleClient::runBackend(void *param)
{
    MyBleClient *pBleClient = (MyBleClient *)param;
    BLEScan *pBLEScan = BLEDevice::getScan();
    while (true) {

        if (pBleClient->_doConnect) {
            pBleClient->connectToServer();
            pBleClient->_doConnect = false;
        }

        // If we are connected to a peer BLE Server, update the characteristic each time we are reached
        // with the current time since boot.
        if ((!pBleClient->_connected) && pBleClient->_doScan) {
            Serial.println("BLE start scan");
            BLEScanResults found = pBLEScan->start(3, false);
            Serial.print("BLE Device found:");
            Serial.println(found.getCount());
            Serial.println("BLE scan done.");
            pBLEScan->clearResults();
            continue;
        }

        delay(100);
    }
}

void MyBleClient::autoConnect()
{
    _doScan = true;

    // 启动自动连接任务
    Serial.println("autoConnect begin");
    xTaskCreatePinnedToCore(MyBleClient::runBackend, "runBackend", 8 * 2048, this, 15, NULL, 0);
}

bool MyBleClient::connectToServer()
{
    BLEClient *pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback(this));

    // Connect to the remove BLE Server.
    pClient->connect(_pDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    pClient->setMTU(517);       // set client to request maximum MTU from server (default is 23 otherwise)

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(BLE_SERVER_SERVICE_UUID);
    if (pRemoteService == nullptr) {
        // 没有找到目标服务的SERVICE_UUID
        pClient->disconnect();
        return false;
    }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    _pRemoteCharacteristic = pRemoteService->getCharacteristic(BLE_SERVER_CHAR_UUID);
    if (_pRemoteCharacteristic == nullptr) {
        // 没有找到目标服务的特征UUID
        pClient->disconnect();
        return false;
    }

    // Serial.print("can Notify:");
    // if (_pRemoteCharacteristic->canNotify()) {
    //     Serial.print("true");
    //     _pRemoteCharacteristic->registerForNotify(notifyCallback);
    // } else {
    //     Serial.print("false");
    // }

    _connected = true;
    return true;
}

// void MyBleClient::notifyCallback(
//     BLERemoteCharacteristic *pBLERemoteCharacteristic,
//     uint8_t *pData,
//     size_t length,
//     bool isNotify)
// {
//     if (_OnReceiveCallback != nullptr) {
//         _OnReceiveCallback(pData, length);
//     }
// }

bool MyBleClient::isConnected()
{
    return _connected;
}

std::string MyBleClient::read()
{
    if (!_connected) {
        return "";
    }
    std::string s=_pRemoteCharacteristic->readValue();
    return s;
}