#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "SparkFunMPL3115A2.h"
#include "SparkFun_Si7021_Breakout_Library.h"

const byte WSPEED = 2;
const byte RAIN = 3;
const byte STAT_BLUE = 7;
const byte STAT_GREEN = 8;
const byte WDIR = A0;
const byte LIGHT = A1;
const byte BATT = A2;
const byte REFERENCE_3V3 = A3;

MPL3115A2 myPressure; //Create an instance of the pressure sensor
Weather myHumidity;//Create an instance of the humidity sensor

long lastSecond;
unsigned int minutesSinceLastReset;
byte seconds;
byte seconds_2m;
byte minutes;
byte minutes_10m;

long lastWindCheck = 0;
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;

byte windspeedavg[120];
#define WIND_DIR_AVG_SIZE 120
int winddiravg[WIND_DIR_AVG_SIZE];
float windgust_10m[10];
int windgustdirection_10m[10];
volatile float rainHour[10];

int winddir; // [0-360 instantaneous wind direction]
float windspeedmph; // [mph instantaneous wind speed]
float windgustmph; // [mph current wind gust, using software specific time period]
int windgustdir; // [0-360 using software specific time period]
float windspdmph_avg2m; // [mph 2 minute average wind speed mph]
int winddir_avg2m; // [0-360 2 minute average wind direction]
float windgustmph_10m; // [mph past 10 minutes wind gust mph ]
int windgustdir_10m; // [0-360 past 10 minutes wind gust direction]
float rainin; // [rain inches over the past hour)] -- the accumulated rainfall in the past 60 min
volatile float dailyrainin; // [rain inches so far today in local time]
//float baromin = 30.03;// [barom in] - It's hard to calculate baromin locally, do this in the agent
float pressure;
//float dewptf; // [dewpoint F] - It's hard to calculate dewpoint locally, do this in the agent

volatile unsigned long raintime, rainlast, raininterval, rain;

void rainIRQ()
{
	raintime = millis();
	raininterval = raintime - rainlast;
	if (raininterval > 10) 
	{
		dailyrainin += 0.011; //each dump is .011" of water
		rainHour[minutes] += 0.011; //increase this minute's amount of rain
		rainlast = raintime;
	}
}

void wspeedIRQ() 
{
	if(millis() - lastWindIRQ > 10) 
	{
		lastWindIRQ = millis();
		windClicks++;
	}
}

void setup() {
	Serial.begin(9600);
	Serial.println("Weather Station KE5EO");

	pinMode(WDIR, INPUT);
	pinMode(LIGHT, INPUT);
	pinMode(REFERENCE_3V3, INPUT);

	pinMode(STAT_BLUE, OUTPUT);
	pinMode(STAT_GREEN, OUTPUT);

	myPressure.begin();
	myPressure.setModeBarometer();
	myPressure.setOversampleRate(7);
	myPressure.enableEventFlags();

	myHumidity.begin();

	lastSecond = millis();

	attachInterrupt(0, rainIRQ, FALLING);
	attachInterrupt(1, wspeedIRQ, FALLING);

	interrupts();

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
      float temp_h = myHumidity.getTempF();
      float pressureValue = myPressure.readPressure();
      float temp_p = myPressure.readTempF();
      float light_lvl = get_light_level();
      float batt_lvl = get_battery_level();

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
      serializeJson(doc, Serial);
      Serial.println();
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
