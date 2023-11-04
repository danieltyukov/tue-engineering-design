#include <Arduino.h>
#include <NimBLEDevice.h>

NimBLEServer *pServer;
NimBLECharacteristic *pCharacteristic;
bool deviceConnected = false;

const int ledPin = 2;
const int rightBuzzerPin = 3;
const int leftBuzzerPin = 4;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DIRECTION_UUID "dfc521a5-ce89-43bd-82a0-28a37f3a2b5a"

unsigned long lastActionTime = 0;

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        digitalWrite(ledPin, HIGH);
    };

    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        digitalWrite(ledPin, LOW);
    }
};

class MyCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
        unsigned long currentTime = millis();
        if (currentTime - lastActionTime < 10500) {
            return;
        }
        lastActionTime = currentTime;

        std::string value = pCharacteristic->getValue();
        char direction = value[0];
        int distance = std::stoi(value.substr(2));

        int buzzDuration = 1000; // default 1 second buzz
        if (distance > 0 && distance <= 50) {
            buzzDuration = 500; // half-second buzz for very close distance
        } else if (distance > 50 && distance <= 100) {
            buzzDuration = 1000; // 1 second buzz for mid-distance
        } else if (distance > 100) {
            buzzDuration = 2000; // 2 second buzz for far distance
        }

        switch (direction) {
            case 'b':
                digitalWrite(leftBuzzerPin, HIGH);
                digitalWrite(rightBuzzerPin, HIGH);
                delay(buzzDuration);
                digitalWrite(leftBuzzerPin, LOW);
                digitalWrite(rightBuzzerPin, LOW);
                break;
            case 's':
                delay(buzzDuration);
                break;
            case 'r':
                if (distance == 0) {
                    // Special case for r:0
                    buzzDuration = 10000; // 10 second buzz
                }
                digitalWrite(rightBuzzerPin, HIGH);
                delay(buzzDuration);
                digitalWrite(rightBuzzerPin, LOW);
                break;
            case 'l':
                if (distance == 0) {
                    // Special case for r:0
                    buzzDuration = 10000; // 10 second buzz
                }
                digitalWrite(leftBuzzerPin, HIGH);
                delay(buzzDuration);
                digitalWrite(leftBuzzerPin, LOW);
                break;
            case 'u':
                digitalWrite(leftBuzzerPin, HIGH);
                digitalWrite(rightBuzzerPin, HIGH);
                delay(buzzDuration);
                digitalWrite(leftBuzzerPin, LOW);
                digitalWrite(rightBuzzerPin, LOW);
                delay(500);
                digitalWrite(leftBuzzerPin, HIGH);
                digitalWrite(rightBuzzerPin, HIGH);
                delay(buzzDuration);
                digitalWrite(leftBuzzerPin, LOW);
                digitalWrite(rightBuzzerPin, LOW);
                break;
            case 'e':
                digitalWrite(leftBuzzerPin, HIGH);
                delay(buzzDuration);
                digitalWrite(leftBuzzerPin, LOW);
                delay(500);
                digitalWrite(rightBuzzerPin, HIGH);
                delay(buzzDuration);
                digitalWrite(rightBuzzerPin, LOW);
                break;
            default:
                break;
        }
    }
};

void setup() {
    pinMode(ledPin, OUTPUT);
    pinMode(leftBuzzerPin, OUTPUT);
    pinMode(rightBuzzerPin, OUTPUT);

    NimBLEDevice::init("ESP32_BT");
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                        DIRECTION_UUID,
                        NIMBLE_PROPERTY::READ |
                        NIMBLE_PROPERTY::WRITE |
                        NIMBLE_PROPERTY::NOTIFY
                    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();
    pServer->getAdvertising()->start();
}

void loop() {
    if (!deviceConnected) {
        digitalWrite(ledPin, !digitalRead(ledPin));
        delay(500);
    } else {
        delay(10);
    }
}