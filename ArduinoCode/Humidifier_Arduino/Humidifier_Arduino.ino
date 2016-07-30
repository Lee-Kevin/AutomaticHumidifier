/*
 * File: Humidifier_Arduino.ino
 * 
 * Author: Jiankai Li
 * Date: 2016-07-30
 * In this demo, mt7688Duo as a humidifier, and connect a HDC1000 sensor save the data info to /tmp/Sensor
 *
 * Mertirials: HDC1000
 *             mt7688Duo
 *             Breakout for Linkit smart 7688 Duo
 *             Button
 *             Grove atomizer
 * 
 */
#include <Process.h>
#include <string.h>
#include "HDC1000.h"
#include <FileIO.h>
#include <avr/wdt.h>
#include <TimerOne.h>

#include "SoftwareSerial.h"

#define PIXIEL_PIN 6
#define PIXIEL_NUM 6
#define USER_MAXNUM   6

#define INTERVAL      1          // Time interval unit s
#define INTERVAL_DATA 5          // Time interval unit s
#define ATOMIZER_PIN     5          // 
#define BUTTON_PIN       4

unsigned long previousTime = 0;  // define the last time  unit s
unsigned long previousTimeData = 0;  // define the data last time  unit 

/* Define the state machine status */

enum Status 
{
    Standby          =  0,         
    UpdateSensorData =  1,
    UpdateAtomizer   =  2,
	Unknow           =  3,
};
typedef enum Status Systemstatus;
Systemstatus WorkingStatus;

struct SensorData 
{
    float Temperature   = 0;
    float Humidity      = 0;
};
typedef struct SensorData SensorData_t;
SensorData_t HomeSensor;
SensorData_t *pHomeSensor;

enum atomstatus 
{
    closed         = 0,
    open           = 1,
    other          = 2,
};
typedef enum atomstatus AtomizerStatus_t;
AtomizerStatus_t AtomizerStatus;

#define BUF_SIZE  8
char *g_Buffer;
char buff[30];
int g_BufferIndex;

HDC1000 mySensor;
  

void setup() {
  Serial.begin(9600);
  Serial.println("power on!");

  // Initialize Bridge
  Bridge.begin();
  // Initialize the FileSystem
  FileSystem.begin();
  // Initialize the HDC1000
  mySensor.begin();
  // Initialize the Relay
  pinMode(ATOMIZER_PIN,OUTPUT);
  // Init the button
  pinMode(BUTTON_PIN,INPUT);
  
  
  previousTime = millis()/1000;
  previousTimeData = millis()/1000 - random(3,10);
  
  bridgeMkdir();
  bridgeSaveData("atomizer",0);
  
  AtomizerStatus = closed;
  
  delay(1000);
  wdt_enable(WDTO_2S);
 
  Timer1.initialize(1000000); // set a timer of length 1000000 microseconds 1 second
  Timer1.attachInterrupt( timerIsrFeedFog ); // attach the service routine here
  wdt_reset();
}

void loop() {
	unsigned long currentTime = millis()/1000;

	if (currentTime - previousTime >= INTERVAL) {

		previousTime = currentTime;
		WorkingStatus = UpdateAtomizer;
	}
	if (currentTime - previousTimeData >= INTERVAL_DATA) {

		previousTimeData = currentTime;
		WorkingStatus = UpdateSensorData;
	}

	switch(WorkingStatus){
	case UpdateSensorData:
		bridgeMkdir();
		HomeSensor.Temperature = mySensor.getTemp();
		HomeSensor.Humidity    = mySensor.getHumi();
		Serial.print("Temp: ");
		Serial.println(HomeSensor.Temperature);
		Serial.print("Humi: ");
		Serial.println(HomeSensor.Humidity);
		
		bridgeSaveData("temp",HomeSensor.Temperature);
		bridgeSaveData("humi",HomeSensor.Humidity);

		WorkingStatus = Unknow;
		break;
	case UpdateAtomizer:
		uint8_t command;
        Serial.print("bridgeGetAtomizerCom");
		command = bridgeGetAtomizerCom();
		if (1 == command) {
			OpenAtomizer();
            AtomizerStatus = open;
		} else {
			CloseAtomizer();
            AtomizerStatus = closed;
		}

		WorkingStatus = Unknow;
		break;
		
	case Unknow:
        if (ifButtonPressed()) {
            if (open == AtomizerStatus) {
                CloseAtomizer();
                AtomizerStatus = closed;
                bridgeClearAtomizerCom();
            } else {
                OpenAtomizer();
                AtomizerStatus = open;
                bridgeSaveData("atomizer",1);
            }
        }
            
		break;
	default:
		break; 
	}
	
}

void bridgeMkdir() {
	Process p;
	p.begin("mkdir");
	p.addParameter("/tmp/Sensor");
	p.runAsynchronously();
}

void bridgeSaveData(char *sname,float data) {
	sprintf(buff,"/tmp/Sensor/%s",sname);
	File script = FileSystem.open(buff,FILE_WRITE);
	script.print(data);
	script.close();
}
/* Get the relay command from 7688 */
uint8_t bridgeGetAtomizerCom() {
	Process p;
	String str;
	uint8_t command;
	p.begin("cat");
	p.addParameter("/tmp/Sensor/atomizer");
	p.run();
	while (p.available() > 0) {
		char c = p.read();
		str += c;
	}
	command = str.toInt();
	return command;
	
}

/* Clear the printer command  */
void bridgeClearAtomizerCom() {
	File script = FileSystem.open("/tmp/Sensor/atomizer",FILE_WRITE);
	script.print(0);
	script.close();
}


void timerIsrFeedFog()
{
    wdt_reset();
    Serial.println("------------Time ISR");
}


void OpenAtomizer() {
	digitalWrite(ATOMIZER_PIN, HIGH);
}

void CloseAtomizer() {
	digitalWrite(ATOMIZER_PIN, LOW);
}

uint8_t ifButtonPressed() {
    uint8_t buttonState;
    buttonState = digitalRead(BUTTON_PIN);
    while (HIGH == buttonState) {
        delay(30);
        Serial.println("Button Pressed");
        if(LOW == digitalRead(BUTTON_PIN)) {
            break;
        } 
    }
    return buttonState;
}
