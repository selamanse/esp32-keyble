#include <Arduino.h>
#include <esp_log.h>
#include <sstream>
#include <queue>
#include <string>
#include "eQ3.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include <esp_wifi.h>
#include <WiFiClient.h>
#include <BLEDevice.h>
#include "secrets.h"

// mqtt
string MQTT_SUB = "/command";
string MQTT_PUB = "/state";
string MQTT_PUB2 = "/task";
string MQTT_PUB3 = "/battery";
string MQTT_PUB4 = "/rssi";

String COMMAND_OPEN = "open";

// ---[Variables]---------------------------------------------------------------
eQ3 *keyble;


bool do_open = false;
bool do_lock = false;
bool do_unlock = false;
bool do_status = false;
bool do_toggle = false;
bool do_pair = false;
bool wifiActive = false;
bool cmdTriggered = false;
unsigned long timeout = 0;
bool statusUpdated = false;
bool waitForAnswer = false;
unsigned long starttime = 0;
int status = 0;
int rssi = 0;
int greenLED = 33;
int redLED = 32;

String mqtt_sub = "";
String mqtt_pub = "";
String mqtt_pub2 = "";
String mqtt_pub3 = "";
String mqtt_pub4 = "";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

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
  //get lockstatus on boot
  do_status = true;
}
// ---[loop]--------------------------------------------------------------------
void loop()
{
  if (Serial.available()) {
    String data = "";
    char str1[] = "\r\n";
    char* cp = str1;
    char c = *cp;
    data = Serial.readStringUntil(c);
    data.trim();
    Serial.println("received: " + data);
    

  if (!waitForAnswer){
    Serial.println("*** check command ***");
    if (data == "open") {
      waitForAnswer = true;
      starttime = millis();
      Serial.println("*** open ***");
      keyble->open();
    } else if (data == "lock")
    {
      waitForAnswer = true;
      starttime = millis();
      Serial.println("*** lock ***");
      keyble->lock();
    } else if (data == "unlock")
    {
      waitForAnswer = true;
      starttime = millis();
      Serial.println("*** unlock ***");
      keyble->unlock();
    } else if (data == "status")
    {
      waitForAnswer = true;
      starttime = millis();
      Serial.println("*** status ***");
      keyble->updateInfo();
    } else if (data == "toggle")
    {
      waitForAnswer = true;
      starttime = millis();
      Serial.println("*** toggle ***");
      if ((status == 2) || (status == 4))
      {
        keyble->lock();
      }
      if (status == 3)
      {
        keyble->unlock();
      }
    } else if (data == "pair")
    {
      waitForAnswer = true;
      starttime = millis();
      Serial.println("*** pair ***");
      //Parse key card data
      std::string cardKey = CARD_KEY;
      if (cardKey.length() == 56)
      {
        std::string pairMac = cardKey.substr(1, 12);

        pairMac = pairMac.substr(0, 2) + ":" + pairMac.substr(2, 2) + ":" + pairMac.substr(4, 2) + ":" + pairMac.substr(6, 2) + ":" + pairMac.substr(8, 2) + ":" + pairMac.substr(10, 2);
        std::string pairKey = cardKey.substr(14, 32);
        std::string pairSerial = cardKey.substr(46, 10);
        keyble->pairingRequest(cardKey);
      }
      else
      {
        Serial.println("# invalid CardKey! Pattern example:");
        Serial.println("  M followed by KeyBLE MAC length 12");
        Serial.println("  K followed by KeyBLE CardKey length 32");
        Serial.println("  Serialnumber");
        Serial.println("  MxxxxxxxxxxxxKxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxSSSSSSSSSS");
      }
    }
  }


  if (waitForAnswer)
  {
    Serial.println("*** waiting for answer ***");
    bool timeout = (millis() - starttime > LOCK_TIMEOUT * 2000 + 1000);
    bool finished = false;

    if ((keyble->_LockStatus != -1) || timeout)
    {
      if (keyble->_LockStatus == 1)
      {
        //Serial.println("Lockstatus 1");
        if (timeout)
        {
          finished = true;
          Serial.println("!!! Lockstatus 1 - timeout !!!");
        }
      }
      else if (keyble->_LockStatus == -1)
      {
        //Serial.println("Lockstatus -1");
        if (timeout)
        {
          keyble->_LockStatus = 9; //timeout
          finished = true;
          Serial.println("!!! Lockstatus -1 - timeout !!!");
        }
      }
      else if (keyble->_LockStatus != 1)
      {
        finished = true;
        //Serial.println("Lockstatus != 1");
      }

      if (finished)
      {
        Serial.println("# Done!");
        data = "";
        delay(100);
        yield();
        greenLEDOn(100);
        statusUpdated = true;
        waitForAnswer = false;
      }
    }
  }

  }
}
