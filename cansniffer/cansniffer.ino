  
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h> 
#include <SD.h>


const int chipSelect = 9;

#define UP      A1
#define DOWN    A3
#define LEFT    A2
#define RIGHT   A5
#define CLICK   A4

#define LED2    8
#define LED3    7

File dataFile;

char buffer[64];

void setup() 
{
  int flshSpd = 500;
  
  Serial.begin(9600);
  Serial.println("Sniffing canbus for ECU Messages");

  pinMode(chipSelect, OUTPUT);
  pinMode(CLICK, INPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  digitalWrite(CLICK, HIGH);

  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  delay(1000);

  Serial.println("Spooling Up!");

  if (Canbus.init(CANSPEED_500))
  {
    Serial.println("CAN Init success.");
    delay(1500);
  }
  else
  {
    Serial.println("Unable to init CAN");
    return;
  }

  Serial.println("Press to begin");
  
  while(digitalRead(CLICK)==HIGH)
  {
    switchLights(50);
  }

  delay(1000);
}

void loop() 
{
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW);

  Serial.println(digitalRead(CLICK));
  while(digitalRead(CLICK)!=HIGH)
  {
    Serial.println(digitalRead(CLICK));
    delay(100);
  }
  
  Serial.println("Starting sniff loop");
  while(digitalRead(CLICK)==HIGH)
  {
    dataCap();
  }
  Serial.println("Exiting sniff loop.");

  while(true)
  {
    switchLights(500);
  }
}

void switchLights(int delayms)
{
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW);
  delay(delayms);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, HIGH);
  delay(delayms);
}

void dataCap()
{
  dataCap(false, 0,0);
}

void dataCap(int filterA, int filterB)
{
  dataCap(true, filterA, filterB);
}
void dataCap(bool filter, int filterA, int filterB)
{
  tCAN message;

  if (mcp2515_check_message())
    {
      if (mcp2515_get_message(&message))
      {
        //0x620   0xff
        if(!filter || (message.id == filterA && message.data[2] == filterB))
        {
          Serial.print("ID: ");
          Serial.print(message.id, HEX);
          Serial.print(", ");
          Serial.print("Data: ");
          Serial.print(message.header.length, DEC);
          for (int i = 0; i < message.header.length; i++)
          {
            Serial.print(message.data[i], HEX);
            Serial.print(" ");
          }
          Serial.println("");
        }
      }
    }
}