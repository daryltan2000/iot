#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 16  
#define OLED_RESET -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 788.7

RH_RF95 rf95(RFM95_CS, RFM95_INT);

const uint8_t myID = 2;        // Unique ID of this node (Receiver)
const uint8_t senderIDs[] = {1, 3}; // Allowed transmitter IDs (Two transmitters)

// Message Types
const uint8_t MSG_TYPE_DATA = 0x01;
const uint8_t MSG_TYPE_ACK  = 0x02;

void displayMessage(const char* message) {
  display.clearDisplay();
  display.setCursor(1, 5);
  display.println(message);
  display.display();
}

uint8_t calculateChecksum(uint8_t *data, uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < length - 1; i++) {
    sum += data[i];
  }
  return sum;
}

// Function to check if the sender is allowed
bool isValidSender(uint8_t sender) {
  for (uint8_t i = 0; i < sizeof(senderIDs); i++) {
    if (sender == senderIDs[i]) {
      return true;
    }
  }
  return false;
}

void setup() {
  Serial.begin(9600);
  delay(100);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  displayMessage("Initializing...");

  // Reset LoRa module
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    displayMessage("Setup Failed");
    while (1);
  }

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    displayMessage("Setup Failed");
    while (1);
  }

  rf95.setTxPower(13, false);
  displayMessage("Setup Successful");
  delay(100);
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      // Check that we have at least a header (3 bytes) and checksum (1 byte)
      if (len >= 4) {
        uint8_t sender = buf[0];
        uint8_t receiver = buf[1];
        uint8_t msgType = buf[2];

        // Check if message is for this receiver and from a valid sender
        if (receiver == myID && isValidSender(sender) && msgType == MSG_TYPE_DATA) {
          uint8_t checksumReceived = buf[len - 1];
          if (checksumReceived == calculateChecksum(buf, len)) {
            Serial.print("Message Received from Sender ");
            Serial.println(sender);
            
            char msg[20];
            sprintf(msg, "Msg from Sender %d", sender);
            displayMessage(msg);
            
            sendAck(sender);
          } else {
            Serial.println("Invalid Checksum");
          }
        } else if (receiver == myID && !isValidSender(sender)) {
          Serial.println("Ignoring message from unknown sender");
          displayMessage("Unknown Sender");
        }
      }
    }
  }
}

void sendAck(uint8_t receiverID) {
  uint8_t ackMsg[4] = { myID, receiverID, MSG_TYPE_ACK, 0 };
  ackMsg[3] = calculateChecksum(ackMsg, sizeof(ackMsg));
  
  Serial.print("Sending ACK to Sender ");
  Serial.println(receiverID);
  displayMessage("Sending ACK");
  
  rf95.send(ackMsg, sizeof(ackMsg));
  rf95.waitPacketSent();
}
