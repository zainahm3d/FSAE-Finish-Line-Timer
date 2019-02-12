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
int inPin = A2; // Use with 10k ohm resistor
double startTime = 1;
double endTime = 1;
double testTime = 1;
bool flag = false;

int arrCounter = 0;   // keeps track of array position. reset on boot
double lapTimes[250]; // Empty set of laptimes

double lapTime = 1;
BLEServer *pServer;
BLECharacteristic *pCharacteristic;

// Sends data over BLE and notifies central device
void uploadTime(double time) {
  char valToSend[10] = "";
  String(time, DEC).toCharArray(valToSend, 10);
  pCharacteristic->setValue(valToSend);
  pCharacteristic->notify();
  Serial.println("time uploaded");
}

void setup() {

  for (int i = 0; i < 250; i++) { // added for safety
    lapTimes[i] = 0.00;
  }

  BLEDevice::init("FSAE Gate Timer");
  delay(200);
  Serial.begin(9600);

  delay(100);

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

void gateTripped() {
  flag = true;
  endTime = millis();
  testTime = ((endTime - startTime) / 1000);
  startTime = millis();

  if ((testTime > 6) && (flag == true)) {
    lapTime = testTime;
    Serial.println(lapTime);

    lapTimes[arrCounter] = lapTime;
    arrCounter++;

    for (int i = 0; i < arrCounter; i++) {
      uploadTime(lapTimes[i]);
    }

    uploadTime(0.00); // Tells iPhone that the data list has finished sending
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

  // reset onboard data if command is sent from iPhone
  if (pCharacteristic->getValue() == "clear") {
    for (int i = 0; i < 250; i++) { // added for safety
      lapTimes[i] = 0.00;
    }
    arrCounter = 0;
    uploadTime(0.00);
    Serial.println("data cleared");
  }

  // send data request to iPhone
  if (pCharacteristic->getValue() == "request") {
    Serial.println("data requested");

    for (int i = 0; i < arrCounter; i++) {
      uploadTime(lapTimes[i]);
    }

    uploadTime(0.00); // Tells iPhone that the data list has finished sending
  }
}