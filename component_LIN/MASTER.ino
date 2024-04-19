const int CS = 4; // MCP2003 CS pin
const int buttonPin = 8; // button1 pin
const int buttonPin2 = 9; // button2 pin
const int ledPin1 = 10; // LED1 pin

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
                            

                            float temp, humidity;

                            // Copy the first 4 bytes from inByte to temp
                            memcpy(&temp, inByte, 4);

                            // Copy the next 4 bytes from inByte to humidity
                            memcpy(&humidity, inByte + 4, 4);

                            // Print the temperature and humidity
                            Serial.print("Temperature: ");
                            Serial.println(temp);
                            Serial.print("Humidity: ");
                            Serial.println(humidity);
                        }

                        break;
                break;
        }
    }
}}


void loop() {

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

}