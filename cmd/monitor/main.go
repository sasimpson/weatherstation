package main

import (
	"bufio"
	"encoding/json"
	"log"
	"bytes"
	"net/http"
	"time"

	"github.com/tarm/serial"

	"github.com/sasimpson/weatherstation/models"
)

const (
	processWorkers int = 1
)

func main() {
	processWorkerPool := make(chan *models.WeatherData, processWorkers)
	for i := 0; i < processWorkers; i++ {
		go processWeather(i, processWorkerPool)
	}

	//serial config
	config := &serial.Config{
		Name:     "/dev/ttyACM0",
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
	defer stream.Close()
	stream.Flush()
	buf := make([]byte, 1024)
	_, err = stream.Read(buf)
	if err != nil {
		log.Fatal(err)
	}

	scanner := bufio.NewScanner(stream)
	for scanner.Scan() {
		var wd models.WeatherData
		err = json.NewDecoder(stream).Decode(&wd)
		if err != nil {
			log.Println(err.Error())
		}
		if (wd.Pressure.Value != 0) {
			now := time.Now()
			wd.Timestamp = now.Unix()
			processWorkerPool <- &wd
		}
	}
	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

}

func processWeather(w int, jobs <-chan *models.WeatherData) {

	var counter int
	for weatherData := range jobs {
		log.Println(weatherData)
		if counter > 15 {
			log.Println("xmit")
			data, err := json.Marshal(weatherData)
			if err != nil {
				log.Println(err)
			}
			resp, err := http.Post("http://chilli.simphouse.com:1880/weather", "application/json", bytes.NewBuffer(data))
			if err != nil {
				log.Println(err)
				continue
			}

			if resp.StatusCode != 200 {
				log.Println("invalid status code: %d", resp.StatusCode)
			}
			resp.Body.Close()
			counter = 0
		} else {
			counter++
		}
	}
}
