/////////////////////////////////////////// CAN BUS DECLARATIONS/////////////////////////////////////////
#include <SPI.h> // include the SPI library
#include <mcp2515.h> // include the CAN library
#include <LiquidCrystal.h> 

const int RS = 12, EN = 13, D4 = 8, D5 = 9, D6 = 10, D7 = 11; // LCD pins
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

struct can_frame canMsg;
MCP2515 mcp2515(53); // CS pin is connected to digital pin 53

const int CANbuttonPin = 40; // pushbutton is connected to digital pin 40

//////////////////////////////////////////// LIN BUS DELCARATIONS/////////////////////////////////////////
//buttons logic 
const unsigned long debounceDelay = 50; // 50ms debounce delay
int buttonState = HIGH;
int lastButtonState = HIGH;
int buttonState2 = HIGH;
int lastButtonState2 = HIGH;

unsigned long lastDebounceTime = 0;

unsigned long previousMillis = 0;  // will store the last time the temperature was requested
const long interval = 2000;  // interval at which to request temperature (milliseconds)

void setup() {
    Serial.begin(9600); // initialize serial communication
    SPI.begin(); // initialize SPI communication

    lcd.begin(16, 2); // set up number of columns and rows

    lcd.setCursor(0, 0);         // move cursor to   (0, 0)
    lcd.print("Setup Loading");        // print message
    lcd.setCursor(3, 1);         // move cursor to   (3, 1)
    lcd.print("LINduinoCAN"); // print message


/////////////////////////////////////////// CAN BUS SETUP ///////////////////////////////////////////

    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // set CAN baud rate to 500 kbps (CLASS C CAN NETWORK (High Speed CAN > 125kbp/s)) and Clock 8MHz
    mcp2515.setNormalMode();

    pinMode(CANbuttonPin, INPUT_PULLUP); // set pin as input enable pull-up resistor

    delay (2000);
    lcd.clear(); // clear the LCD screen
}

///////////////////////// BUTTON HANDLING FUNCTIONS //////////////////////////

// Function to handle button debounce
bool debounceButton(int buttonPin, int &buttonState, int &lastButtonState, unsigned long &lastDebounceTime) {
    int reading = digitalRead(buttonPin);
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            return true;
        }
    }
    lastButtonState = reading;
    return false;
}

void loop() {

    /////////// CAN BUS ///////////

    // this is for the communication TEST button... -> 0x0F is a high priority message -> purely for debugging purposes
    if (digitalRead(CANbuttonPin) == LOW) { // If button is pressed
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

    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) { // If a CAN message is received (primarily for debugging purposes)
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

    //actual messages its expecting, ie, ones it will respond to/ process

    if (canMsg.can_id == 0xAA && canMsg.can_dlc >= 2) { // Check if the message has the correct ID and data length
      int distance = (canMsg.data[0] << 8) | canMsg.data[1]; // Combine the high and low bytes into the distance value
      Serial.print("Distance = ");
      Serial.println(distance);

      lcd.setCursor(0, 0); // Set cursor to top left
      lcd.print("Distance: "); // Print label
      lcd.setCursor(10, 0); // Set cursor to the position where the distance value starts
      lcd.print("   "); // Print spaces to clear the previous value
      lcd.setCursor(10, 0); // Set cursor back to the position where the distance value starts
      lcd.print(distance); // Print the distance value
      }

      if (canMsg.can_id == 0xAB && canMsg.can_dlc >= 8) { // Check if the message has the correct ID and data length for the temperature and humidity data
        float temp, humidity;
        memcpy(&temp, canMsg.data, 4); // copy the first 4 bytes into temp
        memcpy(&humidity, canMsg.data + 4, 4); // copy the next 4 bytes into humidity

        // print on the lcd
        lcd.setCursor(0, 1); // set the cursor to Line 1, col 1
        lcd.print("T:      H:      "); // print labels with spaces to clear old values
        lcd.setCursor(2, 1); // set the cursor back to where the temperature value starts
        lcd.print(temp); // print the temperature value
        lcd.setCursor(10, 1); // set the cursor back to where the humidity value starts
        lcd.print(humidity); // print the humidity value
      }
  }
}