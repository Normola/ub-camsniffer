  
#include <Arduino.h>
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

bool sdPresent = false;

void setup() 
{
  int flshSpd = 500;
  
  Serial.begin(9600);
  
  printUT("Sniffing canbus for ECU Messages");

  pinMode(chipSelect, OUTPUT);
  pinMode(CLICK, INPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  digitalWrite(CLICK, HIGH);

  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  delay(1000);

  printUT("Spooling Up!");

  printUT("Initialising CANBUS");
  if (Canbus.init(CANSPEED_500))
  {
    printUT("CAN Init success.");
    delay(1500);
  }
  else
  {
    printUT("Unable to init CAN");
    return;
  }

  printUT("Setting up data logging.");
  if (!SD.begin(chipSelect))
  {
    printUT("uSD card failed to initialise or is not present.");
    printUT("Messages will only be logged to serial.");
  }
  else
  {
    printUT("uSD card initialised.");
  
    sdPresent = true;

    printUT("Checking SD file structure.");

    if (SD.exists("/canlog"))
    {
      printUT("canlog directory exists");
      printUT("uSD card ready.");
    }
    else
    {
      SD.mkdir("/canlog");
    }
  }

  printUT("Press to begin");
  
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

  printUT(digitalRead(CLICK));
  while(digitalRead(CLICK)!=HIGH)
  {
    printUT(digitalRead(CLICK));
    delay(100);
  }
  
  printUT("Starting sniff loop");
  File datacapFile = GetDataFile();
  datacapFile.println("Starting sniff loop.");

  while(digitalRead(CLICK)==HIGH)
  {
    dataCap(datacapFile);
    CalculateUptime();
  }

  printUT("Exiting sniff loop.");

  printUT("Flushing uSD");
  datacapFile.println("Exiting sniff loop.");
  datacapFile.flush();
  datacapFile.close();

  printUT("Done.");

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


void dataCap(File datacap, bool filter, int filterA, int filterB)
{
  tCAN message;

  if (mcp2515_check_message())
    {
      if (mcp2515_get_message(&message))
      {
        //0x620   0xff
        if(!filter || (message.id == filterA && message.data[2] == filterB))
        {
          digitalWrite(LED3, HIGH);

          String msg = "ID: ";
          msg += String(message.id, HEX);
          msg += ", ";
          msg += "Data: ";
          msg += String(message.header.length, DEC);
          for (int i = 0; i < message.header.length; i++)
          {
            msg += String(message.data[i], HEX);
            msg += " ";
          }
          printUT(msg);
          datacap.println(msg);
          digitalWrite(LED3, LOW);

        }
      }
    }
}

void dataCap(File datacapFile)
{
  dataCap(datacapFile, false, 0,0);
}

void dataCap(File datacapFile, int filterA, int filterB)
{
  dataCap(datacapFile, true, filterA, filterB);
}

File GetDataFile()
{
  String datacapFilename = GetNextDataFile();

  printUT("Opening log file: " + datacapFilename);

  File datacapFile = SD.open(datacapFilename, FILE_WRITE);

  return datacapFile;
}

String GetNextDataFile()
{
  bool exitLoop = false;
  int fileNum = 0;
  while (!exitLoop)
  {
    String message = "Checking for /canlog/" + String(fileNum, DEC) + ".log";
    printUT(message);
    String fileName = "/canlog/";
    fileName += String(fileNum, DEC);
    fileName += ".log";
    if (!SD.exists(fileName))
    {
      exitLoop = true;
    }
    else
    {
      fileNum++;
    }
  }

  String fileName = "/canlog/";
  fileName += String(fileNum, DEC);
  fileName += ".log";


  printUT("Using " + fileName);
  return fileName;
}

unsigned long CalculateUptime() 
{
  static unsigned int  _rolloverCount   = 0;
  static unsigned long _lastMillis      = 0;

  unsigned long currentMilliSeconds = millis();

  if (currentMilliSeconds < _lastMillis) 
  {
    _rolloverCount++;
  }

  _lastMillis = currentMilliSeconds;    

  return 0xFFFFFFFF * _rolloverCount + _lastMillis;
}

void printUT(String message)
{
  printWithUptime(message);
}

void printUT(int message)
{
  printWithUptime(String(message, DEC));
}

void printWithUptime(String message)
{
  String uptime = String(CalculateUptime(), DEC);
  Serial.print(uptime);
  Serial.print(": ");
  Serial.println(message);
}
