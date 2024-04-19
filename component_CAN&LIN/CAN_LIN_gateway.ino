/////////////////////////////////////////// CAN BUS DECLARATIONS/////////////////////////////////////////
#include <SPI.h> // include the SPI library
#include <mcp2515.h> // include the CAN library

struct can_frame canMsg;
MCP2515 mcp2515(53); // CS pin is connected to digital pin 53

const int CANbuttonPin = 40; // pushbutton is connected to digital pin 40

const int RED_PIN = 42; // Arduino pin connected to the red pin on the RGB LED
const int GREEN_PIN = 43; // Arduino pin connected to the green pin on the RGB LED
const int BLUE_PIN = 44; // Arduino pin connected to the blue pin on the RGB LED


//////////////////////////////////////////// LIN BUS DELCARATIONS/////////////////////////////////////////
const int CS = 4; // MCP2003 CS pin
const int buttonPin = 5; // button1 pin
const int buttonPin2 = 6; // button2 pin
const int ledPin1 = 7; // LED1 pin

//declare variables/ constants/ etc
char inByte[9]; // array to store the received data (1Sync, 1ID, 8data, 1checksum)

//declary FACTORY possible counter start positions
const int counter_array[5] = {0x00, 0x10, 0x20, 0x30, 0x40}; // stored counter start points
int start_counter; //global variable to store the counter start point - once definied

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

/////////////////////////////////////////// CAN BUS SETUP ///////////////////////////////////////////

    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // set CAN baud rate to 500 kbps (CLASS C CAN NETWORK (High Speed CAN > 125kbp/s)) and Clock 8MHz
    mcp2515.setNormalMode();

    pinMode(CANbuttonPin, INPUT_PULLUP); // set pin as input and enable pull-up resistor

    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);

/////////////////////////////////////////// LIN BUS SETUP ///////////////////////////////////////////

    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(buttonPin2, INPUT_PULLUP);
    pinMode(ledPin1, OUTPUT);

    pinMode(CS, OUTPUT); // set CS pin as output
    digitalWrite(CS, HIGH); // set CS pin to HIGH -> activating MCP2003 (ready mode)
    digitalWrite(ledPin1, HIGH); // turn LED1 on    

    // Initialize the random num gen, using analogue pin 0 as a seed for entropy
    randomSeed(analogRead(0));
    //counter initilisation mechanism
    int randomIndex = random(5); // random number between 0 and 4, since there are 5 possible counter start points
    
    Serial.write(0x55); // send sync byte (01010101) to LIN bus -> would be interpreted by HW
    Serial.write(0x3B); // 0x3B is ID for MASTER counter sync init -> sending this ID forces the SLAVES to receive the counter start point
    for (int i = 0; i < 8; i++) {
        if (i == 0) {
            Serial.write(randomIndex); // send random counter start point to LIN bus
        } else {
            Serial.write(0x00); // padding
        }
    }

    // Access a random element from the array
    int start_counter = counter_array[randomIndex];

    delay(25); // wait for 25ms
    // Flash the LED randomIndex times
    for (int i = 0; i <= randomIndex; i++) {
        digitalWrite(ledPin1, HIGH); // turn LED1 on
        delay(500); // wait for 500ms
        digitalWrite(ledPin1, LOW); // turn LED1 off
        delay(500); // wait for 500ms before next flash
    }

    previousMillis = millis();  // initialize previousMillis

    digitalWrite(ledPin1, LOW); // ensure LED1 is off at the end of setup phase
}

///////////////////////// MASTER TRANSMIT FUNCTIONS //////////////////////////

//function for when transmitting data over LIN (primarlily used for the debug/ test button)
void master_transmit_data(byte id, byte data[], int dataSize){
    Serial.write(0x55); // send sync byte to LIN bus (01010101)
    Serial.write(id); // send ID to LIN bus
    if (dataSize > 0) { // only send data if arraySize is greater than 0
        for (int i = 0; i < 8; i++) { // loop through each byte in data
            if (i < dataSize && data[i] != 0x00) { // if data is not blank
                Serial.write(data[i]); // send DATA to LIN bus
            } else {
                Serial.write(0x00); // send padding byte
            }
        }
    }
}

// function for standard LIN operation, transmits sync and ID, no data -> slave(s) would respond accordingly
void master_transmit(byte id){
    Serial.write(0x55); // send sync byte to LIN bus (01010101)
    Serial.write(id); // send ID to LIN bus
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

// Function to handle button press with data
void handleButton(int buttonPin, int &buttonState, int &lastButtonState, unsigned long &lastDebounceTime, byte id, byte data[], int dataSize) {
    if (debounceButton(buttonPin, buttonState, lastButtonState, lastDebounceTime) && buttonState == LOW) {
        master_transmit_data(id, data, dataSize);
    }
}

// Overloaded function to handle button press without data
void handleButton(int buttonPin, int &buttonState, int &lastButtonState, unsigned long &lastDebounceTime, byte id) {
    if (debounceButton(buttonPin, buttonState, lastButtonState, lastDebounceTime) && buttonState == LOW) {
        master_transmit(id);
    }
}

///////////////////////////// MASTER LISTENING FUNCTION ///////////////////////////////

// returns true if checksum is validated
bool master_validate_checksum(byte data[], int dataSize){
    byte checksum = 0; // initialize checksum
    for (int i = 0; i < dataSize - 1; i++) { // loop through each byte in data, except the last one
        checksum += data[i]; // add to checksum
    }
    // Calculate checksum: invert the 8-bit sum of all bytes
    checksum = ~checksum;

    // Compare calculated checksum with received checksum
    if (checksum == data[dataSize - 1]) {
        // Checksum is correct, data is valid
        return true;
    } else {
        // Checksum is incorrect, data is invalid
        return false;
    }
}

void master_listen(){
    
    if (Serial.available() >= 1) { /// changin this from 2 to 1 -> allows for odd num  of bytes
        Serial.readBytes(inByte,1);
        switch (inByte[0]){
            case 0x55: // if the sync is detected, the next must be the ID
                Serial.readBytes(inByte,1);
                switch (inByte[0]) {
                    case 0x02: // if ID is 0x02 -> itself in this case
                        Serial.readBytes(inByte,9); // or changing it to read 9 bytes... ONLY ON MASTER
                        
                        // if master_validate_checksum function returns true (passing the inByte array and its size as arguments)
                        // sending the DATA and the CHECKSUM (8 bytes of data, 1 checksum byte)
                        if (master_validate_checksum(inByte, 9)) {
                            // do this, if true (if the checksum is correct)
                            digitalWrite(ledPin1, HIGH);
                            //delay(500);
                            digitalWrite(ledPin1, LOW);

                        // Print the inByte array's contents in hexadecimal format
                        for (int i = 0; i < 9; i++) {
                            Serial.print(inByte[i], HEX);
                            Serial.print(" "); // optional: print a space for readability
                        }
                        Serial.println(); // print a newline for readability
                        }

                        float temp, humidity;

                        // Copy the first 4 bytes from inByte to temp
                        memcpy(&temp, inByte, 4);

                        // Copy the next 4 bytes from inByte to humidity
                        memcpy(&humidity, inByte + 4, 4);

                        // Print the temperature and humidity (debugging purposes)
                        Serial.print("Temperature: ");
                        Serial.println(temp);
                        Serial.print("Humidity: ");
                        Serial.println(humidity);

                        // to send over CAN
                        byte CANdata[8]; // array to store the data to be sent over CAN

                        memcpy(CANdata, &temp, 4); // copy the temperature to the first 4 bytes of CANdata
                        memcpy(CANdata + 4, &humidity, 4); // copy the humidity to the next 4 bytes of CANdata

                        //setup can message structure
                        canMsg.can_id  = 0xAB; // CAN id of the message
                        canMsg.can_dlc = 8; // Data length of the message
                        memcpy(canMsg.data, CANdata, 8); // Copy the data into the CAN message

                        mcp2515.sendMessage(&canMsg); // Send CAN message

                        // Print the CAN message to the serial monitor (debugging purposes)
                        Serial.print("CAN message sent: ID = 0x");
                        Serial.print(canMsg.can_id, HEX);
                        Serial.print(", Data = 0x");
                        for (int i = 0; i < canMsg.can_dlc; i++) {
                            Serial.print(canMsg.data[i], HEX);
                        }
                        Serial.println();
                        break;
                break;
        }
    }
}}


void loop() {

    //scheduling table consists of the temp and humidity request every 2 seconds

    /////////// LIN BUS ///////////

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        // save the last time the temperature was requested
        previousMillis = currentMillis;

        // request temperature by sending 0x02 as the ID
        master_transmit(0x02);
    }

    byte id = 0x01; //id
    byte data[] = {0x10, 0x11, 0x12}; //data
    int dataSize = sizeof(data);
    handleButton(buttonPin, buttonState, lastButtonState, lastDebounceTime, id, data, dataSize);

    byte id3 = 0x03;
    handleButton(buttonPin2, buttonState2, lastButtonState2, lastDebounceTime, id3);
    
    master_listen(); // listen for response -> function performs required action

    /////////// CAN BUS ///////////

    // this is the debug/ test button for comms
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

            // perform actions based on distance measurement
            if (distance < 15) {
                // send LIN command to toggle (blindspot monitoring) LED
                master_transmit(0x03); // send LIN command to turn on LED

                //debugging lights purposes
                analogWrite(RED_PIN, 255);
                analogWrite(GREEN_PIN, 0);
                analogWrite(BLUE_PIN, 0);
            } else {
                // send LIN command to toggle (blindspot monitoring) LED
                master_transmit(0x04); // send LIN command to turn off LED

                //debugging lights purposes
                analogWrite(RED_PIN, 0);
                analogWrite(GREEN_PIN, 50);
                analogWrite(BLUE_PIN, 0);
            }
        }
    }
}