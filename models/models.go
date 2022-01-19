package models

import (
	"fmt"
)

// WeatherData
/*
   {
           "humidity" : {
                   "value":38.7464,
                   "units":"%"
           },
           "temperature": {
                   "value":75.43166,
                   "units":"F"
           },
           "pressure":{
                   "value":96543.25,
                   "units":"Pa"
           },
           "rainfall":{
                   "inches":null,
                   "daily":2.134002
           }
   }
*/

type WeatherData struct {
	Humidity struct {
		Value float64 `json:"value"`
		Units string  `json:"units"`
	} `json:"humidity"`
	Pressure struct {
		Value float64 `json:"value"`
		Units string  `json:"units"`
	} `json:"pressure"`
	Temperature struct {
		Value float64 `json:"value"`
		Units string  `json:"units"`
	} `json:"temperature"`
}

func (wd WeatherData) String() string {
	return fmt.Sprintf("%0.2f deg %s, %0.2f %s, %0.2f %s", wd.Temperature.Value, wd.Temperature.Units, wd.Pressure.Value, wd.Pressure.Units, wd.Humidity.Value, wd.Humidity.Units)
}
