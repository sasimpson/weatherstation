package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"time"

	"github.com/tarm/serial"

	"github.com/sasimpson/weatherstation/models"
)

func main() {
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
	n, err := stream.Read(buf)
	if err != nil {
		log.Fatal(err)
	}
	log.Printf("%q", buf[:n])

	scanner := bufio.NewScanner(stream)
	for scanner.Scan() {
		var wd models.WeatherData
		_, err = stream.Write([]byte("!"))
		if err != nil {
			log.Fatal(err)
		}

		err = json.NewDecoder(stream).Decode(&wd)
		if err != nil {
			log.Println(err.Error())
		}
		fmt.Println(wd)
		stream.Flush()
		time.Sleep(1 * time.Second)
	}
	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

}
