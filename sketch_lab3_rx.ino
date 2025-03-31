// Question
// 1.OLED Display Integration
  // Display the following information on the OLED for both LoRa Receiver & Sender:
    // "Setup Successful"
    // "Setup Failed"
    // "Sending Message"
    // "Waiting for Reply"
    // “Message Received”
  // Refer to ssd1306_i2c.ino for OLED display implementation. Install the “Adafruit SSD1306” library first.
// 2.Enhance Peer-to-Peer Reliability
  // Implement features to prevent crosstalk traffic interference, e.g., ACK and retransmit.
// 3.Improve Message Protocol
  // Develop ID-based messaging with a header, payload, and checksum supporting at least three devices, i.e. simple message filtering, only accepting messages that are directed to the node's agreed ID.


#include <SPI.h>                                // SPI communication for LoRa module
#include <RH_RF95.h>                            // RadioHead library for RFM95 LoRa module
#include <Wire.h>                               // I2C library for OLED display
#include <Adafruit_GFX.h>                       // Graphics library for OLED display
#include <Adafruit_SSD1306.h>                   // Library for SSD1306 OLED

#define SCREEN_WIDTH 128                        // OLED display width in pixels
#define SCREEN_HEIGHT 16                        // OLED display height (single row text)
#define OLED_RESET -1                           // OLED reset pin (not used)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED object

// LoRa module pin configuration
#define RFM95_CS 10                             // Chip Select pin for LoRa
#define RFM95_RST 9                             // Reset pin for LoRa
#define RFM95_INT 2                             // Interrupt pin for LoRa
#define RF95_FREQ 788.7                         // Frequency for LoRa communication (in MHz)

RH_RF95 rf95(RFM95_CS, RFM95_INT);              // Create RF95 object with defined pins

const uint8_t myID = 2;                         // This node's unique ID (Receiver)
const uint8_t senderIDs[] = {1, 3};             // Allowed sender IDs (valid transmitters)

// Define message types
const uint8_t MSG_TYPE_DATA = 0x01;             // Type: data message
const uint8_t MSG_TYPE_ACK  = 0x02;             // Type: acknowledgment message

// Display helper function to print message to OLED
void displayMessage(const char* message) {
  display.clearDisplay();                       // Clear OLED display
  display.setCursor(1, 5);                      // Set cursor to top-left
  display.println(message);                     // Print message
  display.display();                            // Update screen
}

// Function to calculate simple checksum of message
uint8_t calculateChecksum(uint8_t *data, uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < length - 1; i++) {
    sum += data[i];                             // Sum all bytes except the last (checksum byte)
  }
  return sum;
}

// Check if the sender ID is in the allowed list
bool isValidSender(uint8_t sender) {
  for (uint8_t i = 0; i < sizeof(senderIDs); i++) {
    if (sender == senderIDs[i]) {
      return true;                              // Return true if found
    }
  }
  return false;                                 // Not found
}

void setup() {
  Serial.begin(9600);                           // Start serial communication
  delay(100);                                   // Short delay for setup

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for display
    Serial.println(F("SSD1306 allocation failed"));
    while (1);                                  // Stop if OLED fails
  }
  display.setTextSize(1);                       // Set text size to small
  display.setTextColor(WHITE);                  // Set text color
  display.clearDisplay();                       // Clear screen
  displayMessage("Initializing...");            // Show boot message

  // Reset LoRa module
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW);                 // Pull reset pin LOW
  delay(10);                                    // Wait 10ms
  digitalWrite(RFM95_RST, HIGH);                // Pull reset pin HIGH
  delay(10);                                    // Wait again

  // Initialize RF95 module
  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    displayMessage("Setup Failed");             // Display failure on OLED
    while (1);
  }

  // Set frequency for RF95 module
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    displayMessage("Setup Failed");             // Display failure on OLED
    while (1);
  }

  rf95.setTxPower(13, false);                   // Set transmit power to 13 dBm
  displayMessage("Setup Successful");           // Show success message
  delay(100);                                   // Delay before entering loop
}

void loop() {
  if (rf95.available()) {                       // Check if message is available
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];       // Buffer to store incoming message
    uint8_t len = sizeof(buf);                  // Set length to max size
    if (rf95.recv(buf, &len)) {                 // Try to receive packet

      if (len >= 4) {                            // Ensure message is at least 4 bytes (sender, receiver, type, checksum)
        uint8_t sender = buf[0];                // Extract sender ID
        uint8_t receiver = buf[1];              // Extract receiver ID
        uint8_t msgType = buf[2];               // Extract message type

        // Check if message is intended for this receiver, from a known sender, and is a DATA type
        if (receiver == myID && isValidSender(sender) && msgType == MSG_TYPE_DATA) {
          uint8_t checksumReceived = buf[len - 1]; // Extract checksum byte

          // Validate checksum
          if (checksumReceived == calculateChecksum(buf, len)) {
            Serial.print("Message Received from Sender ");
            Serial.println(sender);

            char msg[20];
            sprintf(msg, "Msg from Sender %d", sender); // Format display message
            displayMessage(msg);                         // Show sender on OLED

            sendAck(sender);                             // Send acknowledgment
          } else {
            Serial.println("Invalid Checksum");          // If checksum mismatch
          }
        } else if (receiver == myID && !isValidSender(sender)) {
          Serial.println("Ignoring message from unknown sender"); // Unknown sender warning
          displayMessage("Unknown Sender");
        }
      }
    }
  }
}

// Function to send an acknowledgment message
void sendAck(uint8_t receiverID) {
  uint8_t ackMsg[4] = { myID, receiverID, MSG_TYPE_ACK, 0 }; // Format: sender, receiver, ACK, checksum
  ackMsg[3] = calculateChecksum(ackMsg, sizeof(ackMsg));     // Compute checksum and insert into message

  Serial.print("Sending ACK to Sender ");
  Serial.println(receiverID);
  displayMessage("Sending ACK");              // Show ACK status on screen

  rf95.send(ackMsg, sizeof(ackMsg));           // Send ACK packet
  rf95.waitPacketSent();                       // Wait until packet is completely sent
}
