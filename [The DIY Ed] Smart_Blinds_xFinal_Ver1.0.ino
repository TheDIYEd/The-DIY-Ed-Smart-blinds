//The DIY Ed 
//http://www.youtube.com/channel/UCMSgVhCZs5Rd2fwrPaEA3yw
#include <Arduino.h>
#include <IRremoteESP8266.h> 
#include <IRrecv.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"


fauxmoESP fauxmo; //Declare fauxmoESP object
const char* deviceZero="Blinds";
#define WIFI_SSID "Wifi SSID" //Wifi SSID
#define WIFI_PASS "TheDIYEd" //Wifi Password

int iRPin = 13;            //IR sensor connected to GIPO 13  Pin D7
IRrecv irrecv(iRPin);     //Create an IR object of the class
decode_results results;

int stepPin = 14;      //A4988 Driver Stepper motor step pin to GIPO 14  Pin D5
int dirPin = 5;       //A4988 Driver Stepper motor direction pin to GIPO 5  Pin D1
int alexaValue = 0;
int alexaState = 2;
int countflag = 0;
int stopflag = 0;
int blindDir = 1;         //Blind direction, set to 0 for reverse if needed
int blindLength = 52000;   //Number of steps for full blind length
int blindPosition = 0;    //Initial blind position
int blindPositionB = 0;   
int blindSpeed = 2;     //Delay between pulses, smaller delay, higher speed
int enPin = 4;          // Enable pin for the A4899 driver on GIPO 4  Pin D2


void setup()
   {
  Serial.begin(115200);       //To get HEX value form the remote
  Serial.println();
  Serial.println();
  irrecv.enableIRIn();        //Start the IR receiver
  pinMode(stepPin, OUTPUT);   //Define the stepper motor pins
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  Serial.println("Setting up wifi!");
      wifiSetup(); //Set up wifi

      //Setup fauxmo
      fauxmo.createServer(true); 
      fauxmo.setPort(80); 
      fauxmo.enable(true);

      // Add virtual devices
      fauxmo.addDevice(deviceZero); //Add deviceZero
      fauxmo.onSetState(callback);   
    }

void loop()
    {
  if (irrecv.decode(&results))  //Wait for an IR signal
    {
      serialPrintUint64(results.value, HEX);   //Only used to show HEX value on serial
      driveMotor(results.value); 
      irrecv.resume(); 
      delay(200); 
    }
      fauxmo.handle();
      delay(100);    
 if (alexaValue>0)
    {
      alexaAction();
    }
      stopflag = 0;
      alexaState = 2;
      delay(100);
      digitalWrite(enPin, LOW);
      blindPositionB = blindPosition;  
    }

void callback(unsigned char device_id, const char * device_name, bool state, unsigned char value)
    {
  Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    serialPrintUint64(results.value);
    
      alexaValue = value;
      alexaState = state;   
    }
  
void alexaAction()
    {
      alexaValue = alexaValue * 203.9 ; // Devide the numbers of total steps with 255 to get procentage number


 if ((blindPosition > alexaValue) && (stopflag == 0) && (alexaState == 1))
    
    {
      countflag = blindPosition - alexaValue;
      blindSpeed = 2;
      moveMotor(0,countflag);
      blindPosition = alexaValue;
      stopflag = 1;
    }
     
 if ((blindPosition < alexaValue) && (stopflag == 0) && (alexaState == 1))
    {
      countflag = alexaValue - blindPosition;
      blindSpeed = 1;
      moveMotor(1,countflag);
      blindPosition = alexaValue;
      stopflag = 1;
     }
 if ((blindPosition = alexaValue) && (stopflag == 0) && (alexaState == 1))
    {
      stopflag = 1;
    }
 if (alexaState == 0)
    {

      blindSpeed = 2;
      moveMotor(0,blindPositionB);
      blindPosition = 0;
      alexaState = 2;    
    } 
      alexaValue = 0;
    }

void driveMotor(unsigned long value)  //Read the IR code and decide steps
    {
    
  switch (value)  //IR switch value to action
    {
      break;
      case 0x17FE8:      //Open Fully
      blindSpeed = 2;
      moveMotor(0,blindPosition);
      blindPosition = 0;
      break;
      case 0xFFF00:    //Close Fully
      blindSpeed = 1;
      moveMotor(1,blindLength-blindPosition);
      blindPosition = blindLength;
      break;
    }
    }

void moveMotor(int moveDir, int noSteps)
    {
 if (moveDir == blindDir)
    {
      digitalWrite(dirPin, HIGH);
    }
  else{
      digitalWrite(dirPin, LOW);
    }
 Serial.println("part 2");
  for(int i=0 ; i<=noSteps ; i++)
    {
 Serial.println(i);
      digitalWrite(stepPin, HIGH);
      digitalWrite(enPin, HIGH);
      delay(blindSpeed);
      digitalWrite(stepPin, LOW);
      delay(blindSpeed);
     }
     }

//Wifi Setup
void wifiSetup() 
    {
      // Set WIFI module to STA mode
      WiFi.mode(WIFI_STA);
      wifi_set_sleep_type(LIGHT_SLEEP_T);

      // Connect
      Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
      WiFi.begin(WIFI_SSID, WIFI_PASS);

      // Wait
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
     }
        Serial.println();

       // Connected!
       Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}
