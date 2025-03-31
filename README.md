# iot
IoT Test

# IoT
IoT Test

# CSC2106 Labs Cheatsheet

## 1. REpresentational State Transfer (REST)

- **Definition**: REST is an architectural style for designing networked applications, relying on stateless communication and standard HTTP methods.

- **Key Characteristics**:
  - **Statelessness**: Each request from client to server must contain all information to understand and process the request.
  - **Client-Server Architecture**: The client and server are independent; UI and data storage are separated.
  - **Uniform Interface**: Uses standard methods like HTTP verbs for communication, making the API accessible and predictable.

Stateless communication, HTTP-based. Useful for exposing sensors or actuator APIs. Easily testable using Postman, browser, curl, etc.

- **Common HTTP Methods**:
  - `GET`: Retrieve data from the server.
  - `POST`: Send new data to the server.
  - `PUT`: Update existing data on the server.
  - `DELETE`: Remove data from the server.

- **Usage in IoT**: Allows devices to exchange data using web standards like HTTP and JSON.

-
```
// Setup WiFi
WiFi.begin(ssid, password);
server.begin();

// Route setup
server.on("/temp/", handle_TempResponse);
server.on("/led/1", handle_LedOn);

// Sample JSON response
void handle_TempResponse() {
  String response = "{ \"temperature\": " + String(temp, 2) + " }";
  server.send(200, "application/json", response);
}

```

## 2. Bluetooth Low Energy (BLE)

- **Definition**: BLE is a low-power wireless communication protocol used for short-range communication, part of the Bluetooth 4.0 standard.

- **Key Features**:
  - **Low Energy Consumption**: Ideal for battery-powered devices, it transmits small amounts of data with minimal power.
  - **Operating Frequency**: Uses the 2.4 GHz ISM band.
  - **Network Topologies**: Supports star and mesh topologies for various applications.

BLE is event-based (onNotify, onWrite, onRead).Consists of Peripheral (Server) and Central (Client).

- **BLE vs. Classic Bluetooth**:
  - **BLE**: Optimized for infrequent data transmission with ultra-low power consumption.
  - **Classic Bluetooth**: Suitable for continuous data streaming like audio but uses more power.

- **Usage in IoT**: Used in fitness trackers, beacons, smart locks, and sensors to transmit data efficiently.

```
// Key Code Patterns
// Server side
BLECharacteristic tempChar(UUID_TEMP, BLECharacteristic::PROPERTY_NOTIFY);
tempChar.setValue("25.4");
tempChar.notify();

// Client side
temperatureCharacteristic->registerForNotify(callback);
std::string val = ledCharacteristic->readValue();
ledCharacteristic->writeValue("1");
```

```
//Callback Example
void temperatureNotifyCallback(BLERemoteCharacteristic* p, uint8_t* pData, size_t len, bool isNotify) {
  strncpy(tempStr, (char*)pData, len);
}
```

## 3. LoRa (Long Range)

- **Definition**: LoRa is a long-range, low-power wireless protocol used to transmit small packets over distances up to several kilometers.

- **Key Characteristics**:
  - **Long Range**: Coverage in rural areas can reach up to 15-20 km.
  - **Low Power**: Devices can operate on batteries for years.
  - **Operates in License-Free ISM Bands**: Uses unlicensed frequencies (e.g., 868 MHz, 915 MHz).

Transmitter → sends message with checksum. Receiver → verifies checksum, sends ACK

- **Network Topology**: Typically uses a star topology in LoRaWAN; also supports point-to-point and mesh configurations.

- **Usage in IoT**: Common in agriculture, environmental monitoring, and smart cities for long-distance sensor communication.

```
// Checksum
uint8_t computeChecksum(uint8_t* data, uint8_t length) {
  uint8_t sum = 0;
  for (int i = 0; i < length - 1; i++) sum += data[i];
  return sum;
}

// Send message with ACK loop
rf95.send(message, length);
rf95.waitPacketSent();
```

```
if (receiver == myID && checksum == computeChecksum(buf, len)) {
  sendAck(sender);
}
```

## 4. Message Queue Telemetry Transport (MQTT)

- **Definition**: MQTT is a lightweight publish/subscribe messaging protocol designed for low-bandwidth, high-latency networks.

- **Key Components**:
  - **Broker**: Central server that routes messages between publishers and subscribers.
  - **Client**: Devices that publish data to or subscribe to topics.
  - **Topic**: String identifier used to categorize messages.

Requires a broker (e.g., Mosquitto). Clients subscribe and publish to topics

- **Quality of Service (QoS) Levels**:
  - `0`: At most once – no acknowledgment.
  - `1`: At least once – message may be duplicated.
  - `2`: Exactly once – ensures message is received once and only once.

- **Usage in IoT**: Perfect for real-time telemetry data in resource-constrained environments.

```
// Setup MQTT
client.setServer(mqtt_server, 1883);
client.setCallback(callback);

// Publish and Subscribe
client.publish("M5StackLab4", "TRUE");
client.subscribe("M5StackLab4");
```

```
// Callback Handler
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  if (msg == "TRUE") digitalWrite(10, LOW);
}
```

## 5. Wireless Mesh Networks

- **Definition**: A network topology where each node relays data for the network, enabling multiple pathways for data to travel.

- **Key Features**:
  - **Self-Healing**: Automatically reroutes around failed nodes.
  - **Scalability**: Easy to add more nodes without performance drops.
  - **Decentralized**: No central authority is required for communication.

Supports auto-routing, fault tolerance. Useful in unstructured or mobile IoT networks

- **Usage in IoT**: Used in home automation, industrial control, and disaster recovery where connectivity may vary.

```
// Init mesh
mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
mesh.onReceive(&receivedCallback);

// Message broadcast
mesh.sendBroadcast("0|Emergency!");
```

```
// Priority Queue Processing
if (!highPriorityQueue.empty()) {
  show(highPriorityQueue.front());
  highPriorityQueue.pop_front();
}
```

## 6. M5StickC Plus

- **Definition**: A compact ESP32-based IoT development board with a screen, battery, and various peripherals.

- **Key Features**:
  - **Display**: 1.14” full-color TFT display.
  - **Sensors**: Built-in IMU, RTC, and infrared transmitter.
  - **Battery**: 120mAh rechargeable lithium battery.

- **Usage in Labs**: Used in BLE, REST, LoRa, MQTT, and mesh projects as a flexible and portable IoT device.

```
M5.begin();
M5.Lcd.print("Hello");
M5.BtnA.isPressed();
M5.IMU.getTempData(&temp);
M5.Beep.tone(100);  // buzzer ON
```