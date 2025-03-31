//Question
// Node Interaction Using MQTT
// Configure two M5StickC devices (Node A and Node B).
// Node A should toggle Node B's LED (and vice-versa) upon button press.
// Implement and propose appropriate topic names for this interaction.


/*
*******************************************************************************
* MQTT Example using M5StickC-Plus
* Description: This sketch demonstrates how to connect the M5StickC-Plus to a 
* WiFi network and communicate using MQTT protocol. 
* Author: M5Stack
* Date: 2021/11/5
*******************************************************************************
*/

#include "M5StickCPlus.h"         // Include M5StickC Plus board support
#include <WiFi.h>                 // Include WiFi library for network connection
#include <PubSubClient.h>        // Include MQTT client library

WiFiClient espClient;            // Create WiFi client
PubSubClient client(espClient);  // Create MQTT client with WiFi client

// WiFi credentials and MQTT server IP
const char* ssid        = "kfnam";             // WiFi SSID
const char* password    = "godamnit";          // WiFi password
const char* mqtt_server = "192.168.8.83";      // Local MQTT broker IP address

bool ledStatus = false;          // Current state of the LED (true = ON)
unsigned long lastButtonPress = 0; // Timestamp of last button press
tint lineCount = 0;              // Counter for OLED lines

// Message buffer for sending values over MQTT
unsigned long lastMsg = 0;             // Timestamp for last sent message
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;                         // Placeholder value

// Function declarations
void setupWifi();                      // Function to connect to WiFi
void callback(char* topic, byte* payload, unsigned int length);  // MQTT callback handler
void reConnect();                      // Function to reconnect to MQTT server

void setup() {
    M5.begin();                        // Initialize M5StickC
    M5.Lcd.setRotation(3);            // Set LCD orientation

    setupWifi();                       // Connect to WiFi

    client.setServer(mqtt_server, 1883);  // Set MQTT broker server and port
    client.setCallback(callback);         // Set the callback function to handle messages

    pinMode(10, OUTPUT);              // Set GPIO10 as output for the LED
}

void loop() {
    if (!client.connected()) {        // If not connected to MQTT
        reConnect();                  // Try to reconnect
    }
    M5.update();                      // Update M5 state (for button detection)

    // Toggle LED and publish message on Button A press with debounce (500ms)
    if(M5.BtnA.isPressed() && millis() - lastButtonPress > 500){
        lastButtonPress = millis();  // Update timestamp
        ledStatus = !ledStatus;      // Toggle LED state
        digitalWrite(10, ledStatus ? LOW : HIGH);  // Update LED
        Serial.println(ledStatus ? "LED turned ON" : "LED turned OFF");

        // Publish LED status to MQTT topic
        client.publish("M5StackLab4", ledStatus ? "TRUE" : "FALSE");
    }

    client.loop();                    // Maintain MQTT connection and handle incoming messages
}

// Connect to WiFi network
void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);  // Display SSID being connected to
    WiFi.mode(WIFI_STA);                     // Set to Station mode (not AP)
    WiFi.begin(ssid, password);              // Start WiFi connection

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");                   // Print dots while connecting
    }
    M5.Lcd.printf("\nSuccess\n");           // Print success message
}

// Callback function triggered when a message is received from MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    M5.Lcd.print("Message arrived [");
    M5.Lcd.print(topic);                      // Print topic
    M5.Lcd.print("] ");

    for (int i = 0; i < length; i++) {
        message += (char)payload[i];         // Reconstruct message from payload
    }

    M5.Lcd.print(message);                   // Print received message

    // Update LED based on message content
    if (message == "TRUE"){
        ledStatus = true;
        digitalWrite(10, LOW);              // Turn ON LED
    }
    else if (message == "FALSE"){
        ledStatus = false;
        digitalWrite(10, HIGH);             // Turn OFF LED
    }

    M5.Lcd.println();                        // Newline after message
    lineCount++;                             // Increment line counter

    if(lineCount >= 15){                    // If too many lines, clear screen
        M5.Lcd.fillScreen(BLACK);           // Clear screen
        M5.Lcd.setCursor(0, 0);             // Reset cursor
        lineCount = 0;                      // Reset line count
    }
}

// Reconnect to MQTT server if disconnected
void reConnect() {
    while (!client.connected()) {
        M5.Lcd.print("Attempting MQTT connection...");
        // Create a unique client ID for each device
        String clientId = "M5StackLab4-";
        clientId += String(random(0xffff), HEX);

        if (client.connect(clientId.c_str())) {
            M5.Lcd.printf("\nSuccess\n");
            // Publish an initial message
            client.publish("M5StackLab4", "Hello here for Lab4");
            // Subscribe to the topic
            client.subscribe("M5StackLab4");
        } else {
            M5.Lcd.print("failed, rc=");
            M5.Lcd.print(client.state());
            M5.Lcd.println(" try again in 5 seconds");
            delay(5000);                     // Retry after delay
        }
    }
}
