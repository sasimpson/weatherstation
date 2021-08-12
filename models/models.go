package models

import (
	"fmt"
)

// WeatherData
// ex: {
//			"humidity":{
//				"value":45.79596,
//				"temperature":80.66336
//			},
//			"pressure":{
//				"value":97347.25,
//				"temperature":-1766.2
//			},"light":{
//				"value":0.00494
//			},"power":{
//				"battery":4.454012
//			}
//		}
type WeatherData struct {
	Humidity struct {
		Value       float64 `json:"value"`
		Temperature float64 `json:"temperature"`
	} `json:"humidity"`
	Pressure struct {
		Value       float64     `json:"value"`
		Temperature float64 `json:"temperature"`
	} `json:"pressure"`
	Light struct {
		Value float64 `json:"value"`
	} `json:"light"`
	Power struct {
		Battery float64 `json:"battery"`
	} `json:"power"`
}

func (wd WeatherData) String () string {
	return fmt.Sprintf("%0.2f deg F, %0.2f Pa, %0.2f%% humidity", wd.Humidity.Temperature, wd.Pressure.Value, wd.Humidity.Value)
}
