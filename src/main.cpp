#include <Arduino.h>
#include <esp_log.h>
#include <string>
#include "eQ3.h"
#include "eQ3_constants.h"
#include <BLEDevice.h>
#include "secrets.h"

// ---[Variables]---------------------------------------------------------------
eQ3 *keyble;

String data = "status";

unsigned long timeout = 0;
LockStatus desiredLockState = UNKNOWN;
unsigned long starttime = 0;
int status = 0;
int rssi = 0;
int greenLED = 33;
int redLED = 32;

char strArrayCmdTerminator[] = "\r\n";
char* unsignedCharCmdTerminator = strArrayCmdTerminator;
char COMMAND_TEMRINATOR = *unsignedCharCmdTerminator;

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
  Serial.println("---Starting up...---");
  Serial.setDebugOutput(true);

  greenLEDOn(500);

  //Bluetooth
  BLEDevice::init("");
  keyble = new eQ3(KeyBleMac, KeyBleUserKey, KeyBleUserId);
  keyble->updateInfo();
}
// ---[loop]--------------------------------------------------------------------
void loop()
{
  if (Serial.available()) {
    data = Serial.readStringUntil(COMMAND_TEMRINATOR);
    data.trim();
    Serial.println("received: " + data);

    Serial.println("*** check command ***");
    if (data == "open") {
      desiredLockState = OPENED;
      starttime = millis();
      Serial.println("*** open ***");
      keyble->open();
    } else if (data == "lock")
    {
      desiredLockState = LOCKED;
      starttime = millis();
      Serial.println("*** lock ***");
      keyble->lock();
    } else if (data == "unlock")
    {
      desiredLockState = UNLOCKED;
      starttime = millis();
      Serial.println("*** unlock ***");
      keyble->unlock();
    } else if (data == "status")
    {
      desiredLockState = UNKNOWN;
      starttime = millis();
      Serial.println("*** status ***");
      keyble->updateInfo();
    }
  }

  // bool timeout = (millis() - starttime > LOCK_TIMEOUT * 2000 + 1000);
}
