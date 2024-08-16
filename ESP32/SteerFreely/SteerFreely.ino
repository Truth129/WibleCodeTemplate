/*
  * Developer: Rupak Poddar
  * Wible Code Template for ESP32
  * Steer Freely
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

void _STOP(); // Forward declaration

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;

// UUIDs for the BLE Service and Characteristic
#define SERVICE_UUID           "66443771-D481-49B0-BE32-8CE24AC0F09C"
#define CHARACTERISTIC_UUID    "66443772-D481-49B0-BE32-8CE24AC0F09C"

/*
  * Set the following variable to false to disable print statements
*/
bool verbose = true;

/*
  * Connect motor driver 1 to pin 16, 17
  * Connect motor driver 2 to pin 18, 19
  * Connect light to pin 21
*/
#define M1A 16
#define M1B 17
#define M2A 18
#define M2B 19
#define LED 21

/*
  * Initialize the robot with the following values
*/
String receivedString = "";
unsigned char speed = 100;
String command = "STOP";
bool ignitionState = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      if (verbose) {
        Serial.print("----- Received String: ");
        Serial.print(rxValue);
        Serial.print(" -----\n\n");
      }

      speed = rxValue.substring(0, 3).toInt();
      command = rxValue.substring(3, 7);

      if (verbose) {
        Serial.print("Speed: ");
        Serial.println(speed);
        Serial.print("Command: ");
        Serial.print(command);
        Serial.print("\n\n");
      }

      if (command == "IGON") {
        ignitionState = true;
      }

      if (command == "IGOF") {
        ignitionState = false;
        _STOP();
      }

      if (command == "STOP") {
        _STOP();
      }

      if (command == "LTON") {
        digitalWrite(LED, HIGH);
      }

      if (command == "LTOF") {
        digitalWrite(LED, LOW);
      }

      /*
        * Utilize the Auxiliary buttons from below
      */
      // if (command == "AUX1"){}
      // if (command == "AUX2"){}
      // if (command == "AUX3"){}
      // if (command == "AUX4"){}
      // if (command == "AUX5"){}
      // if (command == "AUX6"){}

      /*
        * Uncomment the following line to disable ignition button
      */
      // ignitionState = true;

      if (ignitionState) {
        /*
          * FORWARD
        */
        if (command == "FWRD") {
          analogWrite(M1A, speed);
          analogWrite(M1B, 0);
          analogWrite(M2A, speed);
          analogWrite(M2B, 0);
        }

        /*
          * BACKWARD
        */
        if (command == "BKWD") {
          analogWrite(M1A, 0);
          analogWrite(M1B, speed);
          analogWrite(M2A, 0);
          analogWrite(M2B, speed);
        }

        /*
          * LEFT
        */
        if (command == "LEFT") {
          analogWrite(M1A, 0);
          analogWrite(M1B, speed);
          analogWrite(M2A, speed);
          analogWrite(M2B, 0);
        }

        /*
          * RIGHT
        */
        if (command == "RGHT") {
          analogWrite(M1A, speed);
          analogWrite(M1B, 0);
          analogWrite(M2A, 0);
          analogWrite(M2B, speed);
        }

        /*
          * Utilize the multi-touch capability of the app from below
        */
        // if (command == "FRLT") {} // FORWARD + LEFT
        // if (command == "FRRT") {} // FORWARD + RIGHT
        // if (command == "BKLT") {} // BACKWARD + LEFT
        // if (command == "BKRT") {} // BACKWARD + RIGHT
      } else {
        if (verbose) {
          Serial.println("+------------------------+");
          Serial.println("|                        |");
          Serial.println("|   IGNITION STATE OFF   |");
          Serial.println("|                        |");
          Serial.println("+------------------------+");
          Serial.println("\n\n");
        }
      }
    }
  }
};

void setup() {
  if (verbose) {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n\nWible ESP32 Steer Freely\n");
  }

  pinMode(LED, OUTPUT);
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  _STOP();

  // Initialize BLE
  BLEDevice::init("Wible");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_WRITE_NR |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  if (verbose) {
    Serial.println("BLE device active, waiting for connections...\n");
  }
}

void loop() {
  if (!deviceConnected) {
    /*
      * Waiting for connection
    */
    ignitionState = false;
    _STOP();
  }
}

void _STOP() {
  analogWrite(M1A, 0);
  analogWrite(M1B, 0);
  analogWrite(M2A, 0);
  analogWrite(M2B, 0);
}