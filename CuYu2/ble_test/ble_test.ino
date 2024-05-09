/*
    CuYu2 BLE test by Takuto Yamana (Nyanyan)
*/
/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini

*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "70bf3a10-b523-4c8f-ba35-c19d97b6237a"
#define CHARACTERISTIC_UUID "d7e1a074-2d98-4abb-a3cf-898ec2a18e52"

int t = 0;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {

      ++t;
      t %= 26;
      char str[2];
      str[0] = ('a' + t);
      str[1] = '\0';
      pCharacteristic->setValue(str);
      Serial.println(str);

      std::string value = pCharacteristic->getValue();
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("start!");
  

  BLEDevice::init("CuYu2");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  delay(2000);
}