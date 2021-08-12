package main

import (
	"encoding/json"
	"fmt"
	"github.com/sasimpson/weatherstation/models"
	"github.com/tarm/serial"
	"log"
)

func main() {
	//serial config
	config := &serial.Config{
		Name:     "COM5",
		Baud:     9600,
		Size:     8,
		Parity:   0,
		StopBits: 1,
	}

	//open serial stream
	stream, err := serial.OpenPort(config)
	if err != nil {
		log.Fatal(err)
	}

	//open json decoder
	decoder := json.NewDecoder(stream)

	for decoder.More() {
		var weather models.WeatherData
		err = decoder.Decode(&weather)
		if err != nil {
			log.Println(err.Error())
		}
		fmt.Printf("weather: %s\n", weather)
	}
}
