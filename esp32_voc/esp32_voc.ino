/*
ESP32 - VOC BLE Bridge
 Receives VOC readings from the Arduino over UART (Serial2) and sends
 them over Bluetooth Low Energy (BLE) so the Web Bluetooth page (index.html) can read them.
 * The device name service UUID and characteristic UUID below must match the values used in index.html:
 *   name           : "ESP32-VOC-Sensor"
 *   service UUID   : 12345678-1234-5678-1234-56789abcdef0
 *   characteristic : abcd1234-5678-1234-5678-abcdef123456
 *
 * Wiring to Arduino (level shift the Arduino's 5V TX line):
 *   ESP32 GPIO16 (RX) <-- [level shifter] <-- Arduino D7 (TX)
 *   ESP32 GPIO17 (TX) --> Arduino D6 (RX)
 *   Common GND between both boards.
 *
 * Board: any ESP32 dev module, using the Espressif ESP32 Arduino core.
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_NAME "ESP32-VOC-Sensor"
#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;

// Restart advertising when a client disconnects so it can reconnect.
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer *pServer) override {
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

String vocReading = "0";

void setup() {
  Serial.begin(115200);                       // USB debug
  Serial2.begin(9600, SERIAL_8N1, 16, 17);    // link to Arduino: RX=16, TX=17

  BLEDevice::init(DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  // Descriptor required for notifications (0x2902)
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("0");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE advertising as " DEVICE_NAME);
}

void loop() {
  // Read one full line ("<value>\n") coming from the Arduino.
  if (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      vocReading = line;
      pCharacteristic->setValue(vocReading.c_str()); // served on READ
      if (deviceConnected) {
        pCharacteristic->notify(); // push to subscribers
      }
      Serial.print("VOC -> BLE: ");
      Serial.println(vocReading);
    }
  }
  delay(20);
}
