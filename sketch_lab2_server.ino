// Question
// In this lab session, M5StickC Plus kits are used to develop a BLE solution that 
// utilizes various sensors included in the kit. The pinout diagram in Figure 2 shows 
// the pins used to access these sensors.

// Please note that this lab session only covers point-to-point communication,
//  and other modes like broadcast and mesh networks will not be covered. The lab guides you 
// through establishing a BLE connection between two M5StickC Plus kits, designating one as the 
// Server and the other as the Client. The Server advertises its presence and contains 
// data that the Client can read. The Client scans for nearby devices, establishes a 
// connection with the Server, and listens for incoming data. You can find the source code 
// for both Server and Client are here.

// Steps:
// Step 1: Compile the Server code and upload it to a M5Stick-C. This device will be referred
//  to as M5StickC A.

// Step 2: Compile the Client code and upload it to another M5Stick-C. This device
//  will be referred to as M5StickC B.

// Step 3: Modify the Server code to:

// Allow another client to connect to the Server. Currently, it hangs after one connect and disconnect. [Hint: Look at Advertising]
// Obtain data from actual sensors (e.g. IMU) and be notified of new readings.

// Extend your lab to incorporate the following functionality:
// 1.Designate M5Stick A as Server and M5Stick B as Client.
// 2.Press the Server Button A to change the Server LED state and notify status updates (temperature, voltage) to the Client.
// 3.Press the Client Button A to read characteristics (led) from the Server & display.
// 4.Press the Client Button B to write characteristics (led) to the Server, changing the LED state on the Server.

// Hints
// You'll need to use BLE characteristics to represent the LED state, temperature, and voltage. Remember to update these characteristics whenever Button A is pressed on the Server.
// Use the M5.Lcd.print() function to display the received LED state on the Client's screen; helps with debugging.
// Ensure that the Client correctly identifies the "LED State" characteristic on the Server to control the LED.

#include <BLEDevice.h>     // Include BLE device management library
#include <BLEServer.h>     // Include BLE server functionality
#include <M5StickCPlus.h>  // Include M5StickC Plus functions (LCD, buttons, sensors)

#define temperatureCelsius  // Define flag to use Celsius temperature format

#define bleServerName "JQ_CSC2106-BLE#01"  // Unique BLE name to identify the server

// Simulated sensor values
float tempC = 25.0;           // Default temperature in Celsius
float tempF;                  // Variable to store Fahrenheit temperature
float vBatt = 5.0;            // Simulated battery voltage

// Timer for refreshing sensor data periodically
unsigned long lastTime = 0;   // Timestamp of last update
unsigned long timerDelay = 15000; // 15 seconds delay for updates

bool deviceConnected = false; // Track if a BLE client is connected
bool ledStatus = false;       // Track the current state of the LED

// UUID for the BLE service (generated from uuidgenerator.net)
#define SERVICE_UUID "4d1ee3fa-62d6-4cbc-b83d-e9f67cce2404"

// Define BLE Characteristics and Descriptors based on selected temperature unit
#ifdef temperatureCelsius
BLECharacteristic imuTemperatureCelsiusCharacteristics("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2405", BLECharacteristic::PROPERTY_NOTIFY); // Characteristic for temperature in Celsius
BLEDescriptor imuTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902)); // Standard descriptor for notify
#else
BLECharacteristic imuTemperatureFahrenheitCharacteristics("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2406", BLECharacteristic::PROPERTY_NOTIFY); // Fahrenheit alternative
BLEDescriptor imuTemperatureFahrenheitDescriptor(BLEUUID((uint16_t)0x2902));
#endif

// Battery voltage characteristic
BLECharacteristic axpVoltageCharacteristics("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2407", BLECharacteristic::PROPERTY_NOTIFY); // Notify battery voltage
BLEDescriptor axpVoltageDescriptor(BLEUUID((uint16_t)0x2903)); // Descriptor for battery voltage

// LED state characteristic (supports read/write/notify)
BLECharacteristic ledStateCharacteristic("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2408", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor ledStateDescriptor(BLEUUID((uint16_t)0x2904)); // Descriptor for LED state

// Server callback class to handle connection and disconnection events
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true; // Set flag when client connects
    Serial.println("MyServerCallbacks::Connected..."); // Debug log
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false; // Clear flag when client disconnects
    Serial.println("MyServerCallbacks::Disconnected...");
    pServer -> getAdvertising()->start(); // Restart advertising
  }
};

// Function to simulate sensor updates and notify the client
void sendData(){
  if (random(2)>0)
    tempC += random(10)/100.0; // Randomly increase temperature by 0.0–0.09
  else
    tempC -= random(10)/100.0; // Or randomly decrease temperature

  tempF = 1.8 * tempC + 32; // Convert Celsius to Fahrenheit

  if (vBatt < 1.0)
    vBatt = 5.0; // Reset voltage if too low
  else
    vBatt -= 0.01; // Simulate discharge

#ifdef temperatureCelsius
  static char temperatureCTemp[6]; // Buffer to hold string value
  dtostrf(tempC, 6, 2, temperatureCTemp); // Format float as string
  imuTemperatureCelsiusCharacteristics.setValue(temperatureCTemp); // Set characteristic value
  imuTemperatureCelsiusCharacteristics.notify(); // Notify connected client
  Serial.print("Temperature = "); Serial.print(tempC); Serial.println(" C"); // Serial output
  M5.Lcd.setCursor(0, 20, 2); M5.Lcd.printf("Temperature = %.2f C", tempC); // Display on screen
#else
  static char temperatureFTemp[6];
  dtostrf(tempF, 6, 2, temperatureFTemp);
  imuTemperatureFahrenheitCharacteristics.setValue(temperatureFTemp);
  imuTemperatureFahrenheitCharacteristics.notify();
  Serial.print("Temperature = "); Serial.print(tempF); Serial.println(" F");
  M5.Lcd.setCursor(0, 20, 2); M5.Lcd.printf("Temperature = %.2f F", tempF);
#endif

  static char voltageBatt[6];
  dtostrf(vBatt, 6, 2, voltageBatt); // Format voltage as string
  axpVoltageCharacteristics.setValue(voltageBatt); // Set battery value
  axpVoltageCharacteristics.notify(); // Notify client
  Serial.print(" - Battery Voltage = "); Serial.print(vBatt); Serial.println(" V");
  M5.Lcd.setCursor(0, 40, 2); M5.Lcd.printf("Battery Voltage = %.2f V", vBatt);

  static char ledStatusReading[6];
  dtostrf(ledStatus, 6, 2, ledStatusReading); // Format LED state
  ledStateCharacteristic.setValue(ledStatusReading); // Set LED state
  ledStateCharacteristic.notify(); // Notify client
  Serial.print(" - LED Status = "); Serial.println(ledStatus);
  M5.Lcd.setCursor(0, 60, 2); M5.Lcd.printf("LED Status = %d", ledStatus);
}

// Callback class to handle write requests from the client to control the LED
class LEDStateWriteCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    std::string value = pCharacteristic->getValue(); // Read the written value
    Serial.print("Received data: "); Serial.println(value.c_str());

    if (value == "1") {
      digitalWrite(10, LOW); // Turn LED ON (active LOW)
      ledStatus = true;
      Serial.println("LED turned ON");
    } else if (value == "0") {
      digitalWrite(10, HIGH); // Turn LED OFF
      ledStatus = false;
      Serial.println("LED turned OFF");
    } else {
      Serial.println("Invalid value received");
    }

    ledStateCharacteristic.setValue(ledStatus ? "1" : "0"); // Update characteristic
    ledStateCharacteristic.notify(); // Notify the new state
    sendData(); // Refresh other sensor readings too
  }
};

void setup() {
  Serial.begin(115200); // Start serial debugging
  pinMode(10, OUTPUT); // LED pin output mode
  M5.begin(); // Initialize M5Stick
  M5.Lcd.setRotation(3); // Set screen orientation
  M5.Lcd.fillScreen(BLACK); // Clear screen
  M5.Lcd.setCursor(0, 0, 2); M5.Lcd.println("BLE Server"); // Title

  BLEDevice::init(bleServerName); // Initialize BLE stack with custom name
  BLEServer *pServer = BLEDevice::createServer(); // Create server instance
  pServer->setCallbacks(new MyServerCallbacks()); // Assign server callbacks

  BLEService *bleService = pServer->createService(SERVICE_UUID); // Create service with UUID

#ifdef temperatureCelsius
  bleService->addCharacteristic(&imuTemperatureCelsiusCharacteristics);
  imuTemperatureCelsiusDescriptor.setValue("IMU Temperature(C)");
  imuTemperatureCelsiusCharacteristics.addDescriptor(&imuTemperatureCelsiusDescriptor);
#else
  bleService->addCharacteristic(&imuTemperatureFahrenheitCharacteristics);
  imuTemperatureFahrenheitDescriptor.setValue("IMU Temperature(F)");
  imuTemperatureFahrenheitCharacteristics.addDescriptor(&imuTemperatureFahrenheitDescriptor);
#endif

  bleService->addCharacteristic(&axpVoltageCharacteristics); // Add battery characteristic
  axpVoltageDescriptor.setValue("AXP Battery(V)");
  axpVoltageCharacteristics.addDescriptor(&axpVoltageDescriptor);

  bleService->addCharacteristic(&ledStateCharacteristic); // Add LED control characteristic
  ledStateDescriptor.setValue("LED State(ON/OFF)");
  ledStateCharacteristic.addDescriptor(&ledStateDescriptor);
  ledStateCharacteristic.setCallbacks(new LEDStateWriteCallback()); // Assign callback for write

  bleService->start(); // Start the service

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); // Start advertising the service
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting for client connection...");
}

void loop() {
  if (deviceConnected) { // If client is connected
    M5.update(); // Read button states
    if(M5.BtnA.isPressed()) { // If button A is pressed
      ledStatus = !ledStatus; // Toggle LED
      digitalWrite(10, ledStatus ? LOW : HIGH);
      Serial.println(ledStatus ? "LED turned ON" : "LED turned OFF");
      sendData(); // Send updated data
    }
    delay(500); // Delay between loop cycles
  }
}
