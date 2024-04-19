#include "DHT.h"

// pin assignments
const int ledPin1 = 8; // LED connected to digital pin 8
const int ledPin2 = 9; // LED connected to digital pin 9
const int CS = 4; // High enables MCP2003
#define DHT11_PIN 5 // DHT11 data pin is connected to digital pin 5

DHT dht11(DHT11_PIN, DHT11);    // Initialize DHT sensor

//declare variables/ constants/ etc
char inByte[8]; // array to store the received data

void setup(){
  Serial.begin(9600); // open the serial port at 9600 bps
  dht11.begin(); // initialize the sensor

  //pin modes
  pinMode(ledPin1, OUTPUT); // sets the digital pin as output
  pinMode(ledPin2, OUTPUT); // sets the digital pin as output
  pinMode(CS, OUTPUT); // intialize the CS pin as output

  //activate MCP2003, state machine
  digitalWrite(CS, HIGH); // enable MCP2003

  //set LED's to high (ON), so if they remain ON there is an error...
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, HIGH);

  delay (500); // wait for 0.5 second - ensure lights turn on and off before starting
}

void slave_transmit_data(byte data[], int dataSize){
    if (dataSize > 0) { // only send data if arraySize is greater than 0
        byte checksum = 0; //init checksum
        for (int i = 0; i < 8; i++) { // loop through each byte in data
            if (i < dataSize && data[i] != 0x00) { // if data is not blank
                Serial.write(data[i]); // send DATA to LIN bus
                checksum += data[i]; // add data to checksum
            } else {
                Serial.write(0x00); // send padding byte
                checksum += 0x00; // add padding to checksum
            }
        }
        //then send checksum?
        checksum = ~checksum; // invert the checksum
        Serial.write(checksum);
    }
}

void loop(){
    //main program, SLAVE -> actions are performed based on the masters command (schedule)
    if (Serial.available() >= 1) { 
        Serial.readBytes(inByte,1);
        switch (inByte[0]){
            case 0x55: // if the sync is detected, the next must be the ID
                Serial.readBytes(inByte,1);

                //define data and dataSize
                byte* data;
                int dataSize;

                if (inByte[0] == 0x01) { // this is the debug button to test the LED's

                  Serial.readBytes(inByte,8);
                        digitalWrite(ledPin1, HIGH); // turn LED on
                        delay(25); // wait for 25 ms
                        if (inByte[0] == 0x10 && inByte[1] == 0x11 && inByte[2] == 0x12) {
                            digitalWrite(ledPin1, HIGH); // turn LED on
                            delay(500); // wait for 25 ms
                            digitalWrite(ledPin2, HIGH); // turn LED on
                            delay(500); // wait for 25 ms
                        }

                }

                if (inByte[0] == 0x02){
                  digitalWrite(ledPin2, HIGH); // quickly flash to indicate ID is received (debugging purposes)

                        float temp = dht11.readTemperature(); // read temperature
                        float humidity = dht11.readHumidity(); // read humidity

                        // Convert temperature to byte array
                        byte tempData[sizeof(temp)];
                        memcpy(tempData, &temp, sizeof(temp));

                        // Convert humidity to byte array
                        byte humidityData[sizeof(humidity)];
                        memcpy(humidityData, &humidity, sizeof(humidity));

                        // Combine temperature and humidity data into one array
                        byte data1[sizeof(temp) + sizeof(humidity)];
                        memcpy(data1, tempData, sizeof(temp));
                        memcpy(data1 + sizeof(temp), humidityData, sizeof(humidity));

                        int dataSize = sizeof(data1); // the byte array size

                        // function called to transmit data
                        slave_transmit_data(data1, dataSize);
                }

                if (inByte[0] == 0x03) { // if ID is 0x03
                    digitalWrite(ledPin1, HIGH); // turn LED on
                }

                if (inByte[0] == 0x04) { // if ID is 0x03
                    digitalWrite(ledPin1, LOW); // turn LED off
                }
        } // end of outer switch-case block
    }
    else{
        digitalWrite(ledPin2, LOW); // turn LED off
    }
}