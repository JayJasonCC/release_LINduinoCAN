// this node transmits a message on the CAN bus every button press
// it is ONLY attached to CAN
// the other NODE is attached to CAN and LIN bus

#include <SPI.h> // include the SPI library
#include <mcp2515.h> // include the CAN library

struct can_frame canMsg;
MCP2515 mcp2515(10); // CS pin is connected to digital pin 10

const int buttonPin = 7; // pushbutton is connected to digital pin 7

const int RED_PIN = 4; // Arduino pin connected to the red pin on the RGB LED
const int GREEN_PIN = 5; // Arduino pin connected to the green pin on the RGB LED
const int BLUE_PIN = 6; // Arduino pin connected to the blue pin on the RGB LED

///////////////////////// SETUP //////////////////////////

void setup() {
  Serial.begin(9600); // initialize serial communication
  SPI.begin(); // initialize SPI communication

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // set CAN baud rate to 500 kbps (CLASS C CAN NETWORK (High Speed CAN > 125kbp/s)) and Clock 8MHz
  mcp2515.setNormalMode();

  pinMode(buttonPin, INPUT_PULLUP); // set pin 2 as input and enable pull-up resistor

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

///////////////////////// MAIN LOOP //////////////////////////

void loop() {
  if (digitalRead(buttonPin) == LOW) { // If button is pressed
    Serial.println("Button pressed."); // Print a message to the serial monitor

    canMsg.can_id  = 0x0F; // CAN id of the message
    canMsg.can_dlc = 8; // Data length of the message
    canMsg.data[0] = 0x01; // Data to be sent
    mcp2515.sendMessage(&canMsg); // Send CAN message

    // Print the CAN message to the serial monitor
    Serial.print("CAN message sent: ID = 0x");
    Serial.print(canMsg.can_id, HEX);
    Serial.print(", Data = 0x");
    for (int i = 0; i < canMsg.can_dlc; i++) {
      Serial.print(canMsg.data[i], HEX);
    }
    Serial.println();

    delay(1000); // Delay to debounce the button
  }

    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) { // If a CAN message is received
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

    if (canMsg.can_id == 0xAA && canMsg.can_dlc >= 2) { // Check if the message has the correct ID and data length
      int distance = (canMsg.data[0] << 8) | canMsg.data[1]; // Combine the high and low bytes into the distance value
      Serial.print("Distance = ");
      Serial.println(distance);

      // Set the color of the RGB LED based on the distance
      if (distance < 15) { //red
        analogWrite(RED_PIN, 255);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
      } else { 
        .//lights green on LOW
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 50);
        analogWrite(BLUE_PIN, 0);
      }
    }
  }

}