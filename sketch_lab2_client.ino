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

// Include the necessary BLE and M5StickCPlus libraries
#include "BLEDevice.h"                       // BLE functions for client communication
#include <M5StickCPlus.h>                    // M5StickCPlus functions (LCD, buttons, etc.)

#define temperatureCelsius                   // Define a macro to use Celsius for temperature (comment out for Fahrenheit)

#define bleServerName "JQ_CSC2106-BLE#01"    // The expected BLE server name to connect to

// Define BLE service UUID
static BLEUUID bleServiceUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2404");

// Define characteristic UUID for temperature based on the temperature unit
#ifdef temperatureCelsius
static BLEUUID temperatureCharacteristicUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2405");
#else
static BLEUUID temperatureCharacteristicUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2406");
#endif

// Define UUIDs for voltage and LED state characteristics
static BLEUUID voltageCharacteristicUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2407");
static BLEUUID ledCharacteristicUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2408");

// Flags to track BLE connection states
static boolean doConnect = false;              // Flag indicating when to initiate connection
static boolean connected = false;              // Flag indicating if client is connected

static BLEAddress *pServerAddress;             // Server address found during scanning

// Remote characteristics we want to access from the server
static BLERemoteCharacteristic* temperatureCharacteristic;
static BLERemoteCharacteristic* voltageCharacteristic;
static BLERemoteCharacteristic* ledCharacteristic;

// Notification control values
const uint8_t notificationOn[] = {0x1, 0x0};   // Enable notification
const uint8_t notificationOff[] = {0x0, 0x0};  // Disable notification

// Buffers to hold sensor data received from server
char temperatureStr[32];
char voltageStr[32];
char ledStr[32];

// Flags to check if new data has been received
boolean newTemperature = false;
boolean newVoltage = false;
boolean newLED = false;
boolean ledState = false;                      // Current LED state on the server

// Function to connect to the BLE server using its address
bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();        // Create BLE client instance
   pClient->connect(pAddress);                            // Connect to server
   Serial.println(" - Connected to server");

   BLERemoteService* pRemoteService = pClient->getService(bleServiceUUID); // Access service
   if (pRemoteService == nullptr) {
     Serial.print("Failed to find service UUID: ");
     Serial.println(bleServiceUUID.toString().c_str());
     return false;                                        // Return if service not found
   }

   // Access the required characteristics
   temperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);
   voltageCharacteristic = pRemoteService->getCharacteristic(voltageCharacteristicUUID);
   ledCharacteristic = pRemoteService->getCharacteristic(ledCharacteristicUUID);

   if (temperatureCharacteristic == nullptr || voltageCharacteristic == nullptr || ledCharacteristic == nullptr) {
     Serial.println("Failed to find characteristics");
     return false;
   }
   Serial.println(" - Found characteristics");

   // Register callbacks for notification data
   temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
   voltageCharacteristic->registerForNotify(voltageNotifyCallback);
   ledCharacteristic->registerForNotify(ledNotifyCallback);

   return true;
}

// Callback handler when a BLE device is discovered
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) {           // Check if the device is our BLE server
      advertisedDevice.getScan()->stop();                        // Stop scanning if found
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Save the server address
      doConnect = true;                                          // Trigger connection
      Serial.println("Device found. Connecting!");
    } else {
      Serial.print(".");                                         // Print dot for each scanned device
    }
  }
};

// Callback: handle temperature notifications
static void temperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                      uint8_t* pData, size_t length, bool isNotify) {
  strncpy(temperatureStr, (char*)pData, sizeof(temperatureStr) - 1); // Copy data into buffer
  temperatureStr[sizeof(temperatureStr) - 1] = '\0';                // Ensure null termination
  newTemperature = true;                                            // Set update flag
}

// Callback: handle voltage notifications
static void voltageNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                  uint8_t* pData, size_t length, bool isNotify) {
  strncpy(voltageStr, (char*)pData, sizeof(voltageStr) - 1);
  voltageStr[sizeof(voltageStr) - 1] = '\0';
  newVoltage = true;
}

// Callback: handle LED status notifications
static void ledNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                              uint8_t* pData, size_t length, bool isNotify) {
  strncpy(ledStr, (char*)pData, sizeof(ledStr) - 1);
  ledStr[sizeof(ledStr) - 1] = '\0';
  newLED = true;
}

// Function to display received data on M5 LCD and serial
void printReadings(){
  M5.Lcd.setCursor(0, 20, 2);
  M5.Lcd.print("Temperature = ");
  M5.Lcd.print(temperatureStr);
  Serial.print("Temperature = ");
  Serial.print(temperatureStr);

  #ifdef temperatureCelsius
    M5.Lcd.println(" C");
    Serial.println(" C");
  #else
    M5.Lcd.println(" F");
    Serial.println(" F");
  #endif

  M5.Lcd.setCursor(0, 40, 2);
  M5.Lcd.print("Battery Voltage = ");
  M5.Lcd.print(voltageStr);
  M5.Lcd.println(" V");

  Serial.print(" - Battery Voltage = ");
  Serial.print(voltageStr);
  Serial.println(" V");
}

void setup() {
  Serial.begin(115200);                             // Start serial debug
  Serial.println("Starting BLE Client application...");

  M5.begin();                                       // Init M5StickC Plus
  M5.Lcd.setRotation(3);                            // Set screen orientation
  M5.Lcd.fillScreen(BLACK);                         // Clear screen
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Client", 0);                   // Show title

  BLEDevice::init("");                              // Init BLE device stack

  BLEScan* pBLEScan = BLEDevice::getScan();         // Create BLE scanner
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); // Set scan callback
  pBLEScan->setActiveScan(true);                    // Enable active scanning
  pBLEScan->start(30);                              // Scan for 30 seconds
}

void loop() {
  if (doConnect == true) {                          // If scan found the correct server
    if (connectToServer(*pServerAddress)) {         // Attempt connection
      Serial.println("Connected to the BLE Server.");

      // Enable notification for temperature and voltage
      temperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      voltageCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2903))->writeValue((uint8_t*)notificationOn, 2, true);

      connected = true;
    } else {
      Serial.println("Failed to connect. Please reset device to try again.");
    }
    doConnect = false;                              // Clear connection trigger
  }

  M5.update(); // Refresh button state

  // Button A pressed: fetch and display LED state from server
  if (M5.BtnA.isPressed() && connected) {
    std::string ledValueStr = ledCharacteristic->readValue();        // Read LED state
    strncpy(ledStr, ledValueStr.c_str(), sizeof(ledStr) - 1);
    ledStr[sizeof(ledStr) - 1] = '\0';

    Serial.print("LED status: ");
    Serial.println(ledStr);
    M5.Lcd.setCursor(0, 60, 2);
    M5.Lcd.print("LED Status = ");
    M5.Lcd.println(ledStr);
  }

  // Button B pressed: toggle LED on server
  if (M5.BtnB.isPressed() && connected) {
    ledState = !ledState;                                    // Flip LED state
    const char* ledCommand = ledState ? "1" : "0";          // Format as string
    ledCharacteristic->writeValue(ledCommand, strlen(ledCommand)); // Write to server

    Serial.print("Writing LED state: ");
    Serial.println(ledState ? "ON" : "OFF");

    M5.Lcd.setCursor(0, 80, 2);
    M5.Lcd.print("LED Toggled: ");
    M5.Lcd.println(ledCommand);
  }

  // Display new readings if updated
  if (newTemperature || newVoltage || newLED) {
    printReadings();
    newTemperature = false;
    newVoltage = false;
    newLED = false;
  }

  delay(1000); // Wait for 1 second
}
