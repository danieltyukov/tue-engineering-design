#include <Arduino.h>
#include <NimBLEDevice.h>

NimBLEServer *pServer;
NimBLECharacteristic *pCharacteristic;
bool deviceConnected = false;

const int ledPin = 2;
const int rightBuzzerPin = 3;
const int leftBuzzerPin = 4;
int ledState = LOW;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DIRECTION_UUID "dfc521a5-ce89-43bd-82a0-28a37f3a2b5a"

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        digitalWrite(ledPin, HIGH);  // Turn on the LED when device connected
    };

    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        ledState = LOW;
        digitalWrite(ledPin, ledState);  // Turn off the LED on disconnect
    }
};

class MyCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value == "r:0") {
            digitalWrite(leftBuzzerPin, HIGH);
            digitalWrite(rightBuzzerPin, HIGH);
            delay(10000); // wait for 10 seconds
            digitalWrite(leftBuzzerPin, LOW);
            digitalWrite(rightBuzzerPin, LOW);
        }
    }
};

void setup() {
    pinMode(ledPin, OUTPUT);
    pinMode(leftBuzzerPin, OUTPUT);
    pinMode(rightBuzzerPin, OUTPUT);

    NimBLEDevice::init("ESP32_BT");
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());  // Set the callbacks for server

    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                        DIRECTION_UUID,
                        NIMBLE_PROPERTY::READ   |
                        NIMBLE_PROPERTY::WRITE  |
                        NIMBLE_PROPERTY::NOTIFY
                    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();
    pServer->getAdvertising()->start();
}

void loop() {
    // Blink the LED when not connected
    if (!deviceConnected) {
        digitalWrite(ledPin, !digitalRead(ledPin));
        delay(500);
    } else {
        delay(10);
    }
}