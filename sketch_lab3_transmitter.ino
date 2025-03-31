#include <SPI.h>                      // Include the SPI library for communication with LoRa module
#include <RH_RF95.h>                  // Include RadioHead library for the RF95 LoRa module
#include <Wire.h>                     // Include the Wire library for I2C communication with the OLED display
#include <Adafruit_GFX.h>             // Include the Adafruit GFX library for the OLED display
#include <Adafruit_SSD1306.h>         // Include the SSD1306 library for the OLED display

#define SCREEN_WIDTH 128              // Width of the OLED screen (in pixels)
#define SCREEN_HEIGHT 16              // Height of the OLED screen (in pixels)
#define OLED_RESET -1                 // Reset pin for the OLED display (-1 means no reset pin is used)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 8); // Initialize the display

// Pin definitions for the LoRa module
#define RFM95_CS 10                   // Chip Select pin for LoRa
#define RFM95_RST 9                   // Reset pin for LoRa
#define RFM95_INT 2                   // Interrupt pin for LoRa
#define RF95_FREQ 788.7               // Frequency for the LoRa communication (in MHz)

// Initialize the LoRa module
RH_RF95 rf95(RFM95_CS, RFM95_INT);    // Using the defined pins for CS and INT

const uint8_t myID = 3;               // Unique ID for the transmitter node
const uint8_t targetID = 2;           // ID of the receiver node (target for the message)

// Message Types for communication
const uint8_t MSG_TYPE_DATA = 0x01;   // Type for sending data
const uint8_t MSG_TYPE_ACK  = 0x02;   // Type for acknowledgment message

// Function to calculate a simple checksum (add all bytes and return the sum)
uint8_t computeChecksum(uint8_t *data, uint8_t length) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < length - 1; i++) {  // Skip the checksum byte itself
    checksum += data[i];                      // Add all message bytes
  }
  return checksum;                            // Return the checksum value
}

void setup() {
  Serial.begin(9600);                      // Initialize Serial communication for debugging
  delay(200);                               // Small delay for serial startup

  // Initialize I2C communication for the OLED display
  Wire.begin();

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Check if the display initializes correctly
    Serial.println(F("SSD1306 allocation failed"));  // Print error if display fails
    while (1);  // Stop the program here if display fails
  }

  display.setTextSize(1);                  // Set text size to 1
  display.setTextColor(WHITE);             // Set text color to white
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);  // Clear screen
  displayMessage("Initializing...");       // Display a message on the screen

  // Reset the LoRa module
  pinMode(RFM95_RST, OUTPUT);              // Set reset pin as output
  digitalWrite(RFM95_RST, LOW);            // Set the reset pin to low
  delay(10);                               // Wait for 10 ms
  digitalWrite(RFM95_RST, HIGH);           // Set the reset pin to high
  delay(10);                               // Wait for 10 ms

  // Initialize the LoRa module and set its frequency
  if (!rf95.init()) {                      // Check if initialization was successful
    Serial.println("LoRa init failed");    // Print error message if it failed
    displayMessage("Setup Failed");        // Display error message on OLED
    while (1);                             // Stop the program here if initialization fails
  }

  if (!rf95.setFrequency(RF95_FREQ)) {     // Set the frequency for communication
    Serial.println("setFrequency failed"); // Print error message if frequency setting fails
    displayMessage("Setup Failed");        // Display error message on OLED
    while (1);                             // Stop the program here if frequency setup fails
  }

  rf95.setTxPower(13, false);              // Set transmit power to 13 dBm (adjustable as needed)
  displayMessage("Setup Successful");      // Display success message on OLED
  delay(200);                              // Wait for a brief moment
}

void loop() {
  sendMessageWithRetransmission(targetID, MSG_TYPE_DATA, "Hello from Transmitter!");  // Send message with retransmission
  delay(500);  // Wait 500 ms before sending the next message
}

// Function to send message with retransmission attempts
void sendMessageWithRetransmission(uint8_t receiverID, uint8_t msgType, const char* payload) {
  uint8_t payloadLen = strlen(payload);            // Calculate the length of the payload
  uint8_t message[payloadLen + 4];                 // Create a buffer to hold the message (with header and checksum)
  message[0] = myID;                              // Add the sender ID (my ID)
  message[1] = receiverID;                        // Add the receiver ID
  message[2] = msgType;                           // Add the message type
  memcpy(&message[3], payload, payloadLen);       // Copy the payload data into the message buffer
  message[3 + payloadLen] = computeChecksum(message, payloadLen + 4);  // Add checksum to the message

  const uint8_t maxRetries = 3;                   // Set maximum retry attempts
  bool ackReceived = false;                       // Flag to check if ACK is received
  uint8_t attempts = 0;                           // Counter for retry attempts

  while (!ackReceived && attempts < maxRetries) {  // Retry loop until ACK is received or max retries are reached
    Serial.println("Sending Message...");         // Print status to Serial Monitor
    displayMessage("Sending Message");            // Display status on OLED
    delay(500);                                   // Wait for a brief moment before sending

    rf95.send(message, sizeof(message));          // Send the message
    rf95.waitPacketSent();                        // Wait until the message is sent

    // Wait for ACK response
    displayMessage("Waiting for Reply");          // Display "Waiting" message on OLED
    unsigned long startTime = millis();           // Get the current time to implement timeout
    while (millis() - startTime < 3000) {         // Timeout after 3 seconds
      if (rf95.available()) {                     // Check if a message is available
        uint8_t buffer[RH_RF95_MAX_MESSAGE_LEN];   // Buffer to store the received message
        uint8_t len = sizeof(buffer);              // Buffer length
        if (rf95.recv(buffer, &len)) {             // Receive the message
          // Validate that this is an ACK coming from the receiver
          if (len >= 4 && buffer[0] == receiverID && buffer[1] == myID && buffer[2] == MSG_TYPE_ACK) {
            uint8_t checksumReceived = buffer[len - 1];  // Get the received checksum
            if (checksumReceived == computeChecksum(buffer, len)) {  // Validate checksum
              Serial.println("ACK Received");      // Print success message
              ackReceived = true;                 // Mark ACK as received
              displayMessage("Message Received"); // Display success message on OLED
              break;                              // Break out of the waiting loop
            }
          }
        }
      }
    }

    if (!ackReceived) {                          // If no ACK received after waiting
      attempts++;                                 // Increment retry counter
      Serial.println("No ACK Received, retrying...");  // Print retry message
      displayMessage("No ACK, Retry");           // Display retry message on OLED
      delay(500);                                // Brief pause before retrying
    }
  }

  // After retries, if no ACK received, display failure message
  if (!ackReceived) {
    Serial.println("Message failed after retries.");  // Print failure message
    displayMessage("Send Failed");                    // Display failure message on OLED
  }
}

// Function to display a message on the OLED screen
void displayMessage(const char* msg) {
  display.clearDisplay();                      // Clear the display
  display.setCursor(0, 0);                     // Set the cursor to the top-left corner
  display.println(msg);                         // Print the message on the screen
  display.display();                            // Update the display
}
