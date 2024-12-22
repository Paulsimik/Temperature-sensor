//##############################//
//    Transmitter Temp Sensor   //
//          Version 1.0         //
//            By Paul           //
//##############################//

#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>

#define CE_PIN PB13
#define CSN_PIN PB12
#define LED_STATUS PB9

enum ledStatusType
{
  LED_NORMAL,
  LED_SUCCESS,
  LED_ERROR
};

enum ledStatusType ledStatusState = LED_NORMAL;

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

int i = 0;
bool ledState = false;
unsigned long ledMillis, tempFilterMillis;
const float TMP36_VOUT_AT_25C = 0.75; 
const float TMP36_SLOPE = 0.010; 
int totalTemp;
int temp;
void ReadTemp();
int GetTemperature();
void LedStatus();
void LedOutput(bool value);

void setup()
{
  SPI.begin();

  pinMode(LED_STATUS, OUTPUT);
  pinMode(CE_PIN, OUTPUT);
  pinMode(CSN_PIN, OUTPUT);
  
  radio.begin();
  radio.openWritingPipe(address); 
  radio.setPALevel(RF24_PA_HIGH); 
  radio.stopListening();  
}

void loop()
{          
  LedStatus();
  ReadTemp();

  if(temp <= 0 || temp >= 200)
  {
    ledStatusState = LED_ERROR;
    return;
  }

  bool success = radio.write(&temp, sizeof(temp));
  if(success)
  {
    ledStatusState = LED_SUCCESS;
  }
  else
  {
    ledStatusState = LED_NORMAL;
  }
}

int GetTemperature()
{
  int val = analogRead(PA0);
  float voltage = val * (3.3 / 1024.0);
  int temperature = (voltage - TMP36_VOUT_AT_25C) / TMP36_SLOPE + 25.0;
  return temperature;
}

void LedStatus()
{
  switch (ledStatusState)
  {
  case LED_NORMAL:
    if(millis() - ledMillis >= 1000)
    {
      ledState = !ledState;
      ledMillis = millis();
    }

    break;
  case LED_SUCCESS:
    ledState = false;
    break;
  case LED_ERROR:
    if(millis() - ledMillis >= 100)
    {
      ledState = !ledState;
      ledMillis = millis();
    }

    break;
  }

  LedOutput(ledState);
}

void LedOutput(bool value)
{
  digitalWrite(LED_STATUS, value);
}

void ReadTemp()
{
  if(millis() - tempFilterMillis >= 50)
  {
    i++;
    totalTemp += GetTemperature();
    tempFilterMillis = millis();
  }

  if(i >= 10)
  {
    temp = totalTemp / 10;
    totalTemp = 0;
    i = 0;
  }
}