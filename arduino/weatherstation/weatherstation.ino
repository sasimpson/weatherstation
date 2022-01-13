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

byte windspdavg[120];
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
float humidity;
float temperature;
float light;
float battery;

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
    myPressure.setModeActive();

	myHumidity.begin();

	lastSecond = millis();

	attachInterrupt(0, rainIRQ, FALLING);
	attachInterrupt(1, wspeedIRQ, FALLING);

	interrupts();

	Serial.println("Weather Staion online!");
}


void loop()
{
    //Print readings every second
    if (millis() - lastSecond >= 1000)
    {
        digitalWrite(STAT_BLUE, HIGH); //Blink stat LED
        lastSecond += 1000;

        if(++seconds_2m > 119) seconds_2m = 0;

        windspeedmph = get_wind_speed();
        winddir = get_wind_direction();
        windspdavg[seconds_2m] = (int)windspeedmph;
        winddiravg[seconds_2m] = winddir;

        if(windspeedmph > windgust_10m[minutes_10m])
        {
            windgust_10m[minutes_10m] = windspeedmph;
            windgustdirection_10m[minutes_10m] = winddir;
        }

        if(windspeedmph > windgustmph)
        {
            windgustmph = windspeedmph;
            windgustdir = winddir;
        }

        delay(25);
        digitalWrite(STAT_BLUE, LOW); //Turn off stat LED


        if(++seconds > 59)
        {
            seconds = 0;
            if(++minutes > 59) minutes = 0;
            if(++minutes_10m > 9) minutes_10m = 0;

            rainHour[minutes] = 0;
            windgust_10m[minutes_10m] = 0;

            minutesSinceLastReset++;
        }
    }



    if (Serial.available())
    {
        byte incoming = Serial.read();
        if(incoming == '!')
        {

        }
    }

}

void calcWeather()
{
    float temp = 0;
    for (int i = 0; i < 120; i++)
        temp += windspdavg[i];
    temp /= 120.0;
    windspdavg_avg2m = temp;

    long sum = winddiravg[0];
    int D = winddiravg[0];
    for(int i = 1; i < WIND_DIR_AVG_SIZE; i++)
    {
        int delta = winddiravg[i] - D;
        if(delta < -180)
            D += delta + 360;
        else if (delta > 180)
            D += delta + 360;
        else
            D += delta;

        sum += D;
    }
    winddir_avg2m = sum / WIND_DIR_AVG_SIZE;
    if(winddir_avg2m >= 360) winddir_avg2m -= 360;
    if(winddir_avg2m < 0) winddir_avg2m += 360;

    windgustmph_10m = 0;
    windgustdir_10m = 0;
    for(int i = 0; i < 10; i++)
    {
        if (windgust_10m[i] > windgustmph_10m)
        {
            windgustmph_10m = windgust_10m[i];
            windgustdir_10m = windgustdirection_10m[i];
        }
    }

    humidity = myHumidity.getRH();
    if (humidity == 998)
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
        temperature = myHumidity.getTempF();
        pressure = myPressure.readPressure();
//      float temp_p = myPressure.readTempF();
        light = get_light_level();
        battery = get_battery_level();
    }

    rainin = 0;
    for(int i = 0; i < 60; i++)
        rainin += rainHour[i];
}

func reportWeather()
{
    digitalWrite(STAT_GREEN, HIGH);
    DynamicJsonDocument doc(100);
    JsonObject humidity = doc.createNestedObject("humidity");
    humidity["value"] = humidity;
    JsonObject temperature = doc.createNestedObject("temperature");
    humidity["temperature"] = temperature;
    JsonObject pressure = doc.createNestedObject("pressure");
    pressure["value"] = pressure;
    JsonObject rainfall = doc.createNestedObject("rainfall");
    rainfall["inches"] = rainin;
    rainfall["daily"] = dailyrainin;
    JsonObject wind = doc.createNestedObject("wind");
    wind["dir"] = winddir;
    wind["speed"] = windspeedmph;
    wind["gust_speed"] = windgustmph;
    wind["gust_dir"] = windgustdir;
    doc["light"]["value"] = light_lvl;
    doc["power"]["battery"] = batt_lvl;
    serializeJson(doc, Serial);
    Serial.println();
    digitalWrite(STAT_GREEN, LOW);

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

//Returns the instataneous wind speed
float get_wind_speed()
{
	float deltaTime = millis() - lastWindCheck; //750ms

	deltaTime /= 1000.0; //Covert to seconds

	float windSpeed = (float)windClicks / deltaTime; //3 / 0.750s = 4

	windClicks = 0; //Reset and start watching for new wind
	lastWindCheck = millis();

	windSpeed *= 1.492; //4 * 1.492 = 5.968MPH

	/* Serial.println();
	 Serial.print("Windspeed:");
	 Serial.println(windSpeed);*/

	return(windSpeed);
}

int get_wind_direction()
// read the wind direction sensor, return heading in degrees
{
	unsigned int adc;

	adc = averageAnalogRead(WDIR); // get the current reading from the sensor

	// The following table is ADC readings for the wind direction sensor output, sorted from low to high.
	// Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
	// Note that these are not in compass degree order! See Weather Meters datasheet for more information.

	if (adc < 380) return (113);
	if (adc < 393) return (68);
	if (adc < 414) return (90);
	if (adc < 456) return (158);
	if (adc < 508) return (135);
	if (adc < 551) return (203);
	if (adc < 615) return (180);
	if (adc < 680) return (23);
	if (adc < 746) return (45);
	if (adc < 801) return (248);
	if (adc < 833) return (225);
	if (adc < 878) return (338);
	if (adc < 913) return (0);
	if (adc < 940) return (293);
	if (adc < 967) return (315);
	if (adc < 990) return (270);
	return (-1); // error, disconnected?
}

