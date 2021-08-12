package main

import (
	"fmt"
	"github.com/sasimpson/weatherstation/models"
	"github.com/sasimpson/weatherstation/weatherstation"
	"github.com/tarm/serial"
	"io"
	"log"
)

func main() {

	var weatherDataWorkers = 1

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
		log.Fatalf("XBeeSerial: OpenPort Error: %s", err.Error())
	}

	weatherDataWorkerPool := make(chan *models.WeatherData, 1)
	ws := weatherstation.NewSerialReader(stream)

	for wdp := 0; wdp <= weatherDataWorkers; wdp++ {
		go readWeather(ws, weatherDataWorkerPool)
	}
	for data := range weatherDataWorkerPool {
		fmt.Println(data)
	}
}

func readWeather(reader *weatherstation.SerialReader, out chan<- *models.WeatherData) {
	for {
		weatherReading, err := reader.ReadSerial()
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Fatal(err)
		}
		log.Println(weatherReading)
		out <- weatherReading
	}
}
