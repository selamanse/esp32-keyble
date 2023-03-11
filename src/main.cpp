#include <Arduino.h>
#include <esp_log.h>
#include <string>
#include "eQ3.h"
#include "eQ3_constants.h"
#include <BLEDevice.h>
#include "secrets.h"
#include <HardwareSerial.h>

// ---[Variables]---------------------------------------------------------------
eQ3 *keyble;

String data = "status";

unsigned long timeout = 0;
LockStatus desiredLockState = UNKNOWN;
unsigned long lastrun = 0;
int status = 0;
int rssi = 0;
int greenLED = 33;
int redLED = 32;

char strArrayCmdTerminator[] = "\r\n";
char* unsignedCharCmdTerminator = strArrayCmdTerminator;
char COMMAND_TEMRINATOR = *unsignedCharCmdTerminator;

// ---[UART Serial]---------------------------------------------------------------
HardwareSerial esphomeUART(2);  //using UART2

// ---[LED Stuff]------------------------------------------------------------
void initLED(){
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
}
void allLEDOff(){
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
}
void redLEDOn(int duration){
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, HIGH);
  if(duration > 0){
    delay(duration);
    allLEDOff();
  }
}
void greenLEDOn(int duration){
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  if(duration > 0){
    delay(duration);
    allLEDOff();
  }
}

// ---[Setup]-------------------------------------------------------------------
void setup()
{
  initLED();
  redLEDOn(0);
  delay(1000);
  Serial.begin(115200);
  Serial.println("--- Starting up ---");
  Serial.setDebugOutput(true);

  //UART
  esphomeUART.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("--- UART 1 Serial started ---");

  greenLEDOn(500);

  //Bluetooth
  BLEDevice::init("");
  keyble = new eQ3(KeyBleMac, KeyBleUserKey, KeyBleUserId);
  keyble->setOnStatusChange([](LockStatus status) {
    Serial.println("--- Status changed ---");
    Serial.println("Status: " + String(status));
    Serial.println("Desired Status: " + String(desiredLockState));
    esphomeUART.println(String(status));
  });

  keyble->connect();
  while(keyble->state.connectionState != CONNECTED){
    Serial.println("--- Connecting to KeyBle ---");
    delay(1000);
  }

  Serial.println("Connected to KeyBle");
  keyble->updateInfo();
  Serial.println("--- Startup finished ---");
  
}
// ---[loop]--------------------------------------------------------------------
void loop()
{
  if (esphomeUART.available()) {
    data = esphomeUART.readStringUntil(COMMAND_TEMRINATOR);
    data.trim();
    Serial.println("received: " + data);

    Serial.println("--- check command ---");
    if (data == "open") {
      desiredLockState = OPENED;
      Serial.println("--- open ---");
      keyble->open();
    } else if (data == "lock")
    {
      desiredLockState = LOCKED;
      Serial.println("--- lock ---");
      keyble->lock();
    } else if (data == "unlock")
    {
      desiredLockState = UNLOCKED;
      Serial.println("--- unlock ---");
      keyble->unlock();
    } else if (data == "status")
    {
      desiredLockState = UNKNOWN;
      Serial.println("--- status ---");
      keyble->updateInfo();
    }

    lastrun = millis();
  }

  if ((millis() - lastrun > LOCK_TIMEOUT * 1000))
  {
    Serial.println("--- force status update ---");
    keyble->updateInfo();
    lastrun = millis();
  }

}
