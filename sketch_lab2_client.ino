#include "BLEDevice.h"
#include <M5StickCPlus.h>

//Default Temperature in Celsius
#define temperatureCelsius

// change the BLE Server name to connect to
#define bleServerName "JQ_CSC2106-BLE#01"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID bleServiceUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2404");

// BLE Characteristics
#ifdef temperatureCelsius
  // Temperature Celsius Characteristic
  static BLEUUID temperatureCharacteristicUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2405");
#else
  // Temperature Fahrenheit Characteristic
  static BLEUUID temperatureCharacteristicUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2406");
#endif

// Battery Voltage Characteristic
static BLEUUID voltageCharacteristicUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2407");

static BLEUUID ledCharacteristicUUID("4d1ee3fa-62d6-4cbc-b83d-e9f67cce2408");

//Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

//Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;
 
//Characteristicd that we want to read
static BLERemoteCharacteristic* temperatureCharacteristic;
static BLERemoteCharacteristic* voltageCharacteristic;
static BLERemoteCharacteristic* ledCharacteristic;  // Add LED characteristic

//Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

//Variables to store temperature and voltage
// Variables to store temperature, voltage, and LED status
char temperatureStr[32];  // Buffer for temperature data
char voltageStr[32];      // Buffer for voltage data
char ledStr[32];          // Buffer for LED status

//Flags to check whether new temperature and voltage readings are available
boolean newTemperature = false;
boolean newVoltage = false;
boolean newLED = false;  // Track if new LED data is available
boolean ledState = false; // Track LED state (false = OFF, true = ON)

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bleServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bleServiceUUID.toString().c_str());
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.
  temperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);
  voltageCharacteristic = pRemoteService->getCharacteristic(voltageCharacteristicUUID);
  ledCharacteristic = pRemoteService->getCharacteristic(ledCharacteristicUUID);  // Get LED characteristic


  if (temperatureCharacteristic == nullptr || voltageCharacteristic == nullptr || ledCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
 
  //Assign callback functions for the Characteristics
  temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
  voltageCharacteristic->registerForNotify(voltageNotifyCallback);
  ledCharacteristic->registerForNotify(ledNotifyCallback);

  return true;
}

//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
    else
      Serial.print(".");
  }
};
 
//When the BLE Server sends a new temperature reading with the notify property
static void temperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                        uint8_t* pData, size_t length, bool isNotify) {
  //store temperature value
  strncpy(temperatureStr, (char*)pData, sizeof(temperatureStr) - 1);
  temperatureStr[sizeof(temperatureStr) - 1] = '\0';  // Ensure null-termination
  newTemperature = true;
//  Serial.println("temperatureNotifyCallback");
}

//When the BLE Server sends a new voltage reading with the notify property
static void voltageNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  //store voltage value
  strncpy(voltageStr, (char*)pData, sizeof(voltageStr) - 1);
  voltageStr[sizeof(voltageStr) - 1] = '\0';  // Ensure null-termination
  newVoltage = true;
//  Serial.println("voltageNotifyCallback");
}

static void ledNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  //store voltage value
  strncpy(ledStr, (char*)pData, sizeof(ledStr) - 1);
  ledStr[sizeof(ledStr) - 1] = '\0';  // Ensure null-termination
  newLED = true;
//  Serial.println("voltageNotifyCallback");
}

//function that prints the latest sensor readings in the OLED display
void printReadings(){
  M5.Lcd.setCursor(0, 20, 2);
  M5.Lcd.print("Temperature = ");
  M5.Lcd.print(temperatureStr);

  Serial.print("Temperature = ");
  Serial.print(temperatureStr);

  #ifdef temperatureCelsius
    //Temperature Celsius
    M5.Lcd.println(" C");
    Serial.print(" C");
  #else
    //Temperature Fahrenheit
    M5.Lcd.print(" F");
    Serial.print(" F");
  #endif

  //display voltage
  M5.Lcd.setCursor(0, 40, 2);
  M5.Lcd.print("Battery Voltage = ");
  M5.Lcd.print(voltageStr);
  M5.Lcd.println(" V");

  Serial.print(" - Battage Voltage = ");
  Serial.print(voltageStr); 
  Serial.println(" V");

  // Display LED status if available
  // if (ledStr != nullptr) {
  //   M5.Lcd.setCursor(0, 60, 2);
  //   M5.Lcd.print("LED Status = ");
  //   M5.Lcd.println(ledStr);
  //   Serial.print(" - LED Status = ");
  //   Serial.println(ledStr);
  // }
}

void setup() {
  //Start serial communication
  Serial.begin(115200);
  Serial.println("Starting BLE Client application...");

  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Client", 0);

  //Init BLE device
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server that we want to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("Connected to the BLE Server.");
      
      //Activate the Notify property of each Characteristic
      temperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      voltageCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2903))->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
    } else {
      Serial.println("Failed to connect to the server; Restart device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }

  // If Button A is pressed, read the LED characteristic
  M5.update();
  if (M5.BtnA.isPressed() && connected) {
    std::string ledValueStr = ledCharacteristic->readValue();
    strncpy(ledStr, ledValueStr.c_str(), sizeof(ledStr) - 1);  // Store the LED status
    ledStr[sizeof(ledStr) - 1] = '\0';  // Ensure null-termination
    Serial.print("LED status: ");
    Serial.println(ledStr);
    M5.Lcd.setCursor(0, 60, 2);
    M5.Lcd.print("LED Status = ");
    M5.Lcd.println(ledStr);
  }

  // If Button B is pressed, toggle the LED state and write it to the server
  if (M5.BtnB.isPressed() && connected) {
    // Write the new LED state to the server
    ledState = !ledState;  // Flip the LED state
    const char* ledCommand = ledState ? "1" : "0";  // Set command based on state
    ledCharacteristic->writeValue(ledCommand, strlen(ledCommand));  // Write to the characteristic
    Serial.print("Writing LED state: ");
    Serial.println(ledState ? "ON" : "OFF");

    // Display the new LED state on the OLED screen
    M5.Lcd.setCursor(0, 80, 2);
    M5.Lcd.print("LED Toggled: ");
    M5.Lcd.println(ledCommand);
    Serial.print("LED toggled: ");
    Serial.println(ledCommand);
  }

  //if new temperature readings are available, print in the OLED
  // If new temperature or voltage readings are available, print in the OLED
  if (newTemperature || newVoltage || newLED) {
    printReadings();
    newTemperature = false;  // Reset flags after displaying
    newVoltage = false;
    newLED = false;
  }
  delay(1000); // Delay one second between loops.
}