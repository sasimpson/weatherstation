package main

import (
	"bufio"
	"fmt"
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

	scanner := bufio.NewScanner(stream)
	for scanner.Scan() {
		fmt.Println(scanner.Text())
	}
	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}
}
