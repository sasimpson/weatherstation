package weatherstation

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log"

	"github.com/sasimpson/weatherstation/models"
)

type SerialReader struct {
	scanner *bufio.Scanner
	buffer  []byte
	idx     int
}

func NewSerialReader(source io.Reader) *SerialReader {

	scanner := bufio.NewScanner(source)
	split := func(data []byte, atEOF bool) (advance int, token []byte, err error) {
		fmt.Println(string(data))
		if atEOF && len(data) == 0 {
			log.Println("atEOF and len zero")
			return 0, nil, nil
		}
		if i := bytes.Index(data, []byte("}}\n{")); i >= 0 {
			log.Println(data[0:i])
			log.Println("search found at ", i)
			return i + 3, data[0 : i+2], nil
		}
		if atEOF {
			log.Println("atEOF")
			return len(data), data, nil
		}
		// Request more data.
		return 0, nil, nil
	}
	// Set the split function for the scanning operation.
	scanner.Split(split)

	return &SerialReader{
		scanner: scanner,
	}
}

func (sr *SerialReader) ReadSerial() (*models.WeatherData, error) {
	if sr.scanner.Scan() {
		var weatherReading models.WeatherData
		event := sr.scanner.Bytes()
		err := json.Unmarshal(event, &weatherReading)
		if err != nil {
			return nil, err
		}
		return &weatherReading, nil
	}
	if err := sr.scanner.Err(); err != nil {
		return nil, err
	}
	return nil, io.EOF
}
