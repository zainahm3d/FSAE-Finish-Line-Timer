#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <iostream>
#include <stdlib.h>
#include <string.h>

#define SERVICE_UUID "0a197167-38cd-40a6-8e08-cc637b93b8ce"
#define CHARACTERISTIC_UUID "676e0287-815e-4f6f-b18a-64bcae972e90"

int threshold = 3000;

int inPin = A2;
double startTime = 1;
double endTime = 1;
double testTime = 1;
bool flag = false;

double lapTime = 1;
BLEServer *pServer;
BLECharacteristic *pCharacteristic;

void setup() {
  BLEDevice::init("FSAE Gate Timer");
  delay(200);
  Serial.begin(9600);

  delay(100);
  for (int i = 0; i < 100; i++) {
    Serial.println();
  }
  Serial.println("Initialization complete");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(inPin, INPUT);

  analogReadResolution(12);

  startTime = millis();
  Serial.println("\n");
  Serial.println("FSAE GATE TIMER");

  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ |
                               BLECharacteristic::PROPERTY_WRITE |
                               BLECharacteristic::PROPERTY_NOTIFY);

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

// Sends data over BLE and notifies central device
void uploadTime(double time) {
  char valToSend[10] = "";
  String(time, DEC).toCharArray(valToSend, 10);
  pCharacteristic->setValue(valToSend);
  pCharacteristic->notify();
}

void gateTripped() {
  flag = true;
  endTime = millis();
  testTime = ((endTime - startTime) / 1000);
  startTime = millis();

  if ((testTime > 6) && (flag == true)) {
    lapTime = testTime;
    Serial.println(lapTime);
    uploadTime(lapTime);
  }
}

void loop() {
  int lightData = analogRead(inPin);

  if (lightData > threshold) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (lightData < threshold) {
    digitalWrite(LED_BUILTIN, LOW);
    gateTripped();
    delay(3000);
  }
}