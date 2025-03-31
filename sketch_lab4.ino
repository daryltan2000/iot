/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5StickC-Plus sample source code
*                             M5StickC-Plus
* Visit for more information: https://docs.m5stack.com/en/core/m5stickc_plus
*
* Describe: MQTT
* Date: 2021/11/5
*******************************************************************************
*/
#include "M5StickCPlus.h"
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

// Configure the name and password of the connected wifi and your MQTT Serve
// host.  
const char* ssid        = "kfnam";
const char* password    = "godamnit";
const char* mqtt_server = "192.168.8.83";

bool ledStatus = false;
unsigned long lastButtonPress = 0;
int lineCount = 0;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE]; 
int value = 0;

void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();

void setup() {
    M5.begin();
    M5.Lcd.setRotation(3);
    setupWifi();
    client.setServer(mqtt_server, 1883);  // Sets the server details.  
    client.setCallback(callback);  		  // Sets the message callback function.  

    //LED
    pinMode(10, OUTPUT);
}

void loop() {
    if (!client.connected()) {
        reConnect();
    }
    M5.update();

    
    if(M5.BtnA.isPressed()&& millis() - lastButtonPress>500){
      lastButtonPress = millis();
      ledStatus = !ledStatus;
      digitalWrite(10,ledStatus?LOW:HIGH);
      Serial.println(ledStatus ? "LED turned ON" : "LED turned OFF");
      client.publish("M5StackLab4", ledStatus? "TRUE" : "FALSE" );  
    }
   
    client.loop(); 
}

void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);  		 // Set the mode to WiFi station mode.  
    WiFi.begin(ssid, password);  // Start Wifi connection.  

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }
    M5.Lcd.printf("\nSuccess\n");
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    M5.Lcd.print("Message arrived [");
    M5.Lcd.print(topic);
    M5.Lcd.print("] ");

    for (int i = 0; i < length; i++) {
        message+= (char)payload[i] ;  
    }

    M5.Lcd.print(message);
    
    if (message == "TRUE"){
      ledStatus = true;
      digitalWrite(10,LOW);
    }
    else if(message == "FALSE"){
      ledStatus = false;
      digitalWrite(10,HIGH);
    }
    M5.Lcd.println();
    lineCount++;

    if(lineCount>=15){
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      lineCount=0;
    }
    
}

void reConnect() {
    while (!client.connected()) {
        M5.Lcd.print("Attempting MQTT connection...");
        // Create a random client ID.  
        String clientId = "M5StackLab4-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect.  
        if (client.connect(clientId.c_str())) {
            M5.Lcd.printf("\nSuccess\n");
            // Once connected, publish an announcement to the topic.
            
            client.publish("M5StackLab4", "Hello here for Lab4");
            // ... and resubscribe.  
            client.subscribe("M5StackLab4");
        } else {
            M5.Lcd.print("failed, rc=");
            M5.Lcd.print(client.state());
            M5.Lcd.println("try again in 5 seconds");
            delay(5000);
        }
    }
}