**CSC2106 Mock Lab Quiz (40 min Practice)** 

This quiz includes a mix of **MCQs**, **multi-select**, and **short answer** questions. It’s based on Labs 1 to 5: REST, BLE, LoRa, MQTT, and Wireless Mesh. ![](Aspose.Words.62b91937-4d44-48b3-bf59-5dfdbc37dc82.001.png)

📘 **Section A: Multiple Choice Questions (1 mark each)** 

**Q1.** Which HTTP method is used in REST to update an existing resource? 

- **Answer**: C. PUT 

**Q2.** Which of the following describes the main difference between BLE and Classic Bluetooth? 

- **Answer**: C. BLE is optimized for short data bursts 

**Q3.** In LoRa communication, what is the purpose of the checksum byte? 

- **Answer**: C. To verify message integrity 

**Q4.** MQTT operates on which of the following protocols? 

- **Answer**: B. TCP/IP 

**Q5.** In a mesh network, which node is responsible for routing messages? 

- **Answer**: B. Any connected node 

**Q6.** What does client.setCallback(callback) do in MQTT? 

- **Answer**: B. Defines a function to handle incoming messages 

**Q7.** Which component in BLE contains actual sensor data? 

- **Answer**: C. Characteristic 

**Q8.** Which of the following is a valid mesh message format from Lab 5? 

- **Answer**: C. 1|Emergency Message ![](Aspose.Words.62b91937-4d44-48b3-bf59-5dfdbc37dc82.002.png)

📗 **Section B: Multi-Select Questions (2 marks each)** 

**Q9.** Which of the following are RESTful API principles? (Select 2) 

- **Answer**: Stateless communication, Uniform interface 

**Q10.** What features are offered by MQTT protocol? (Select 2) 

- **Answer**: Publish/subscribe messaging, Lightweight overhead 

**Q11.** Which are valid BLE operations by the client? (Select 3) 

- **Answer**: Read characteristic, Register for notification, Write characteristic 

**Q12.** Which features are true for LoRa in Lab 3? (Select 2) 

- **Answer**: Long-range low-power transmission, Acknowledgement mechanism via checksum 

  **Q13.** Which of the following describe mesh network benefits? (Select 3) 

- **Answer**: Fault-tolerant routing, Self-organizing node structure, Broadcast messaging support ![](Aspose.Words.62b91937-4d44-48b3-bf59-5dfdbc37dc82.003.png)

  📙 **Section C: Short Answer Questions (3–5 marks each)** 

  **Q14.** Explain how the REST lab uses endpoints to expose sensor data and control actuators. 

- **Answer**: The REST server uses predefined routes like /gyro/, /temp/, /led/1 to respond to HTTP GET requests. Each endpoint is mapped to a handler that reads 

  sensor data or triggers actuators like LEDs or buzzers, and sends a response in JSON format. 

  **Q15.** What are the differences between BLE descriptors and characteristics? Provide an example of how each is used. 

- **Answer**: Characteristics contain the actual data (e.g., temperature), while descriptors provide metadata (e.g., format or units). A characteristic might be 0x2405 

  for temperature, and its descriptor (0x2902) enables notification. 

  **Q16.** Describe how the checksum and acknowledgment mechanism works in LoRa communication. 

- **Answer**: The transmitter includes a checksum (sum of all data bytes) in each message. The receiver verifies it upon reception and sends an ACK message back if it matches. If no ACK is received, the sender retries up to 3 times. 

  **Q17.** How does a client subscribe to a topic in MQTT and respond to a message? Include a code snippet. 

- **Answer**: 

client.subscribe("M5StackLab4"); client.setCallback(callback); 

void callback(char\* topic, byte\* payload, unsigned int length) {   String msg = String((char\*)payload).substring(0, length); 

`  `if (msg == "TRUE") digitalWrite(10, LOW); 

} 

**Q18.** In the mesh lab, what happens when a message with priority 0 is received? Explain how the queue system is managed. 

- **Answer**: Priority 0 is high. The message is added to highPriorityQueue and processed before medium or low priority queues. The display is updated with the 

  highest available priority message. 

  **Q19.** Write a code snippet to: 

- Connect to WiFi on M5StickC 
- Start a WebServer 
- Expose a temperature endpoint 
- **Answer**: 

WiFi.begin(ssid, password); 

while (WiFi.status() != WL\_CONNECTED) delay(500); WebServer server(80); 

server.on("/temp", []() { 

`  `float t; 

`  `M5.IMU.getTempData(&t); 

`  `server.send(200, "application/json", String("{\"temp\": ") + t + "}"); }); 

server.begin(); 

**Q20.** Describe 2 use cases where MQTT is better suited than REST and why. 

- **Answer**: (1) Real-time sensor data updates — MQTT’s low latency and persistent connections make it ideal. (2) Battery-powered IoT devices — MQTT 

  reduces power consumption due to lightweight messaging. 

  **Q21.** What is the benefit of using random message priority and interval in mesh message broadcasting? 

- **Answer**: Random priorities simulate real-world event importance and prevent congestion. Random intervals reduce collisions and allow fair scheduling across nodes. ![](Aspose.Words.62b91937-4d44-48b3-bf59-5dfdbc37dc82.004.png)
- Total: 40 marks (Recommended time: 40 minutes) Good luck! 🍀 
