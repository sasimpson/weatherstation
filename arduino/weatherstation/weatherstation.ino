#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "SparkFunMPL3115A2.h"
#include "SparkFun_Si7021_Breakout_Library.h"

const byte DOUT = 2;
const byte DIN = 3;
const byte STAT_BLUE = 7;
const byte STAT_GREEN = 8;
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;


MPL3115A2 myPressure; //Create an instance of the pressure sensor
Weather myHumidity;//Create an instance of the humidity sensor
SoftwareSerial XBee(DOUT, DIN); // RX, TX

unsigned long lastSecond;

void setup() {
  // put your setup code here, to run once:
  XBee.begin(9600);
  Serial.begin(9600);
  Serial.println("Weather Station KE5EO");

  pinMode(STAT_BLUE, OUTPUT);
  pinMode(STAT_GREEN, OUTPUT);
  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);

  myPressure.begin();
  myPressure.setModeBarometer();
  myPressure.setOversampleRate(7);
  myPressure.enableEventFlags();

  myHumidity.begin();

  lastSecond = millis();

  Serial.println("Weather Station online!");
}


void loop()
{
  //Print readings every second
  if (millis() - lastSecond >= 1000)
  {
    digitalWrite(STAT_BLUE, HIGH); //Blink stat LED
    lastSecond += 1000;
    float humidityValue = myHumidity.getRH();
    if (humidityValue == 998)
    {
      Serial.println("I2C communication to sensors is not working. Check solder connections.");

      //Try re-initializing the I2C comm and the sensors
      myPressure.begin();
      myPressure.setModeBarometer();
      myPressure.setOversampleRate(7);
      myPressure.enableEventFlags();
      myHumidity.begin();
    }
    else
    {
      Serial.print("Humidity = ");
      Serial.print(humidityValue);
      Serial.print("%,");
      float temp_h = myHumidity.getTempF();
      Serial.print(" temp_h = ");
      Serial.print(temp_h, 2);
      Serial.print("F,");

      float pressureValue = myPressure.readPressure();
      Serial.print(" Pressure = ");
      Serial.print(pressureValue);
      Serial.print("Pa,");
      float temp_p = myPressure.readTempF();
      Serial.print(" temp_p = ");
      Serial.print(temp_p, 2);
      Serial.print("F,");

      //Check light sensor
      float light_lvl = get_light_level();
      Serial.print(" light_lvl = ");
      Serial.print(light_lvl);
      Serial.print("V,");

      //Check batt level
      float batt_lvl = get_battery_level();
      Serial.print(" VinPin = ");
      Serial.print(batt_lvl);
      Serial.print("V");
      
      Serial.println();

      digitalWrite(STAT_GREEN, HIGH);
      DynamicJsonDocument doc(100);
      JsonObject humidity = doc.createNestedObject("humidity");
      humidity["value"] = humidityValue;
      humidity["temperature"] = temp_h;
      
      JsonObject pressure = doc.createNestedObject("pressure");
      pressure["value"] = pressureValue;
      pressure["temperature"] = temp_p;
      doc["light"]["value"] = light_lvl;
      doc["power"]["battery"] = batt_lvl;
      serializeJson(doc, XBee);
      XBee.println();
      digitalWrite(STAT_GREEN, LOW);
    }

    digitalWrite(STAT_BLUE, LOW); //Turn off stat LED
  }

  delay(100);
}

//Returns the voltage of the light sensor based on the 3.3V rail
//This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
float get_light_level()
{
  float operatingVoltage = analogRead(REFERENCE_3V3);
  float lightSensor = analogRead(LIGHT);
  operatingVoltage = 3.3 / operatingVoltage; //The reference voltage is 3.3V
  lightSensor = operatingVoltage * lightSensor;
  return (lightSensor);
}

//Returns the voltage of the raw pin based on the 3.3V rail
//This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
//Battery level is connected to the RAW pin on Arduino and is fed through two 5% resistors:
//3.9K on the high side (R1), and 1K on the low side (R2)
float get_battery_level()
{
  float operatingVoltage = analogRead(REFERENCE_3V3);
  float rawVoltage = analogRead(BATT);
  operatingVoltage = 3.30 / operatingVoltage; //The reference voltage is 3.3V
  rawVoltage = operatingVoltage * rawVoltage; //Convert the 0 to 1023 int to actual voltage on BATT pin
  rawVoltage *= 4.90; //(3.9k+1k)/1k - multiple BATT voltage by the voltage divider to get actual system voltage
  return (rawVoltage);
}
