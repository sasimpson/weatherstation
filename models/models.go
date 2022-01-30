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
		Celsius float64 `json:"temp_c"`
		Fahrenheit string  `json:"temp_f"`
	} `json:"temperature"`
	Rain struct {
		Hour float64 `json:"inches"`
		Daily float64 `json:"daily"`
	} `json:"rainfall"`
	Wind struct {
		Now struct {
			Speed float64 `json:"speed"`
			Direction int `json:"dir"`
			GustSpeed float64 `json:"gust_speed"`
			GustDir int `json:"gust_dir"`
		} `json:"now"`
		TwoMin struct {
			SpeedAvg float64 `json:"speed_avg"`
			DirAvg int `json:"dir_avg"`
		} `json:"2m"`
		LastGust10m struct {
			GustSpeed float64 `json:"gustspeed_avg"`
			GustDirAvg float64 `json:"gustdir_avg"`
		} `json:"10m"`
	} `json:"wind"`
	Light struct {
		Value float64 `json:"value"`
	} `json:"light"`
}


func (wd WeatherData) String() string {
	return fmt.Sprintf("%0.2f degF, %0.2f %s, %0.2f %s, %0.2f in Rain, %0.2f mph in %d degrees", wd.Temperature.Fahrenheit, wd.Pressure.Value, wd.Pressure.Units, wd.Humidity.Value, wd.Humidity.Units, wd.Rain.Hour, wd.Wind.Now.Speed, wd.Wind.Now.Direction)
}
