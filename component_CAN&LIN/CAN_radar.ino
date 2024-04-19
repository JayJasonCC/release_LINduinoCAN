//this is the CAN radar code for the LINduinoCAN project

#include <SPI.h> // include the SPI library
#include <mcp2515.h> // include the CAN library
#include <NewPing.h> // include the Ultrasonic sensor library

struct can_frame canMsg;
MCP2515 mcp2515(10); // CS pin is connected to digital pin 10

#define TRIGGER_PIN  9  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     8 // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 500 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

unsigned int prevDistance = 0; // Previous distance measured by the ultrasonic sensor
#define N 25 // Number of measurements to average
unsigned int measurements[N]; // Array to hold measurements
unsigned int sum = 0; // Sum of measurements
unsigned int index = 0; // Index of oldest measurement

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void setup() {
    Serial.begin(9600);
    SPI.begin(); // initialize SPI communication

    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // set CAN baud rate to 500 kbps and Clock 8MHz
    mcp2515.setNormalMode();

    pinMode(7, OUTPUT); // LED connected to digital pin 7

    // Initialize measurements array and sum
    for (int i = 0; i < N; i++) {
      measurements[i] = 100; // initialises array with 100's instead of 0's
      sum += measurements[i];
    }
}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) { // If a CAN message is received (for debugging purposes)
    Serial.print("Received CAN message with ID: 0x");
    Serial.print(canMsg.can_id, HEX);
    Serial.print(", Data: 0x");
    for (int i = 0; i < canMsg.can_dlc; i++) {
      if (canMsg.data[i] < 0x10) { // Print leading zero for single-digit hex values
        Serial.print('0');
      }
      Serial.print(canMsg.data[i], HEX);
    }
    Serial.println();

    // 0x0F is the comms test message -> flashes LED
    if (canMsg.can_id == 0x0F) { // If the message has ID 0x0F
      digitalWrite(7, HIGH); // Turn on the LED
      delay(500); // Wait for 500ms
      digitalWrite(7, LOW); // Turn off the LED
    }
  }

  delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  unsigned int distance = uS / US_ROUNDTRIP_CM; // Convert ping time to distance in cm (US_ROUNDTRIP_CM = 57).

  // Subtract oldest measurement from sum
  sum -= measurements[index];
  // Replace oldest measurement with new one
  measurements[index] = distance;
  // Add new measurement to sum
  sum += measurements[index];
  // Calculate moving average
  unsigned int movingAverage = sum / N;
  // Increment index, wrapping around to 0 if it exceeds N-1
  index = (index + 1) % N;

  // Check if the current distance is different from the previous distance
  if (movingAverage != prevDistance) {
    // Prepare the CAN message
    canMsg.can_id  = 0xAA; // CAN id of the message
    canMsg.can_dlc = 8; // Data length of the message
    canMsg.data[0] = (movingAverage >> 8) & 0xFF; // High byte of distance
    canMsg.data[1] = movingAverage & 0xFF; // Low byte of distance

    // Send the CAN message
    mcp2515.sendMessage(&canMsg);

    // Print the CAN message to the serial monitor (for debugging purposes)
    Serial.print("CAN message sent: ID = 0x");
    Serial.print(canMsg.can_id, HEX);
    Serial.print(", Distance = ");
    Serial.print(movingAverage);
    Serial.println(" cm");

    // Update the previous distance
    prevDistance = movingAverage;
  }
  
}