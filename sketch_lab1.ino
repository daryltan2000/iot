// Question
// This Arduino code for the M5StickC Plus creates a simple web server that serves as a RESTful API to access sensor data from the device. The code establishes a Wi-Fi connection, initializes the device's IMU (Inertial Measurement Unit) for gyroscope and temperature data, and configures the LCD display. It listens for incoming HTTP requests on port 80, defining a single endpoints: '/' that would trigger the handle_JsonResponse function. When a request is made to '/', the server responds with JSON-formatted sensor data, including pitch, roll, yaw, and temperature readings. Additionally, it turns on an LED when data is requested. The loop function ensures continuous checks for incoming client requests and updates sensor data on the LCD. The device triggers an update to the sensor data when a physical button is pressed.

// In essence, this code transforms the M5StickC Plus into a remotely accessible sensor device. Users can access real-time sensor data over a local Wi-Fi network by making HTTP requests to the device's RESTful API endpoints. The code provides a straightforward way to gather essential sensor information, making it a versatile tool for various applications that require monitoring or control of physical parameters using the M5StickC Plus.

// Access the RESTful API: Once the code is running on your M5StickC Plus, you can access the RESTful API from a web browser or use a tool like Postman to make HTTP requests to your device. To retrieve sensor data, open a web browser and enter the IP address of your M5StickC Plus (displayed on the LCD), followed by a forward slash ("/"). For example, if the device's IP address is "192.168.1.100," enter "http://192.168.1.100/" in the browser's address bar. You should receive a JSON response containing sensor data in the format: {"imu": {"pitch": X, "roll": Y, "yaw": Z, "temperature": T}}.

// Test Button Functionality: The code also includes functionality to reset sensor data by pressing the M5_BUTTON_HOME button. The sensor data will only update when you press the button. Thus, verify that pressing the button updates the sensor data.

// By completing this lab, you have learned how to setup the M5StickC, connect it to WiFi, install the necessary libraries, and configure it for REST API access. You have also tested the M5StickC with a REST API, giving you a foundation for using it in your future projects.

// Extend your lab to incorporate both the functionality listed below:
// 1.Extend URL Endpoints: You can extend the RESTful APIs into different URL endpoints to interact with the device's sensors and obtain data. The available endpoints are:
// a. / - Root endpoint
// b. /gyro/ - Gyroscope data endpoint
// c. /accel/ - Accelerometer data endpoint
// d. /temp/ - Temperature data endpoint

// 2.Actuator Control: In addition to obtaining data from the device, the RESTful API can also trigger actuators on the device. You can extend the functionality to control various actuators, such as LED, buzzer, and LCD. The available actuator control endpoints are:
// a. /led/0 - Turn off the LED
// b. /led/1 - Turn on the LED
// c. /buzzer/0 - Turn off the buzzer
// d. /buzzer/1 - Turn on the buzzer

// Include necessary libraries
#include <WiFi.h>               // For connecting to WiFi
#include <WebServer.h>          // For creating the RESTful HTTP server
#include <M5StickCPlus.h>       // Library for M5StickC Plus functions (IMU, LCD, Buzzer, etc.)

/* WiFi Credentials */
const char* ssid = "Daryl";     // WiFi SSID
const char* password = "goodgoodgood"; // WiFi Password

// Global variables to store sensor values
float pitch, roll, yaw, temperature;
float accelX, accelY, accelZ;

// Create a WebServer object listening on port 80 (default HTTP port)
WebServer server(80);

void setup() {
  Serial.begin(115200); // Start serial monitor for debugging

  M5.begin(); // Initialize M5StickC Plus

  // Initialize the IMU (gyroscope, accelerometer, temperature sensor)
  int x = M5.IMU.Init();
  if(x != 0)
    Serial.println("IMU initialisation fail!");

  // Set LCD rotation and clear screen
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("RESTful API", 0); // Display "RESTful API" text on screen

  // Connect to WiFi
  WiFi.begin(ssid, password);
  WiFi.setHostname("group01-stick"); // Set custom hostname

  Serial.print("Start WiFi ..");
  // Wait until connected to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting ..");
  }

  // Display IP address on screen once connected
  M5.Lcd.setCursor(0, 20, 2);
  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());

  pinMode(M5_BUTTON_HOME, INPUT); // Set HOME button as input
  delay(100); // Short delay for stability

  // Define API endpoints and their handler functions
  server.on("/", handle_JsonResponse);
  server.on("/gyro/", handle_GyroResponse);
  server.on("/accel/", handle_AccelResponse);
  server.on("/temp/", handle_TempResponse);
  server.on("/led/0", handle_LedOff);
  server.on("/led/1", handle_LedOn);
  server.on("/buzzer/0", handle_BuzzerOff);
  server.on("/buzzer/1", handle_BuzzerOn);
  server.onNotFound(handle_NotFound); // Fallback handler

  server.begin(); // Start the server
  Serial.println("HTTP server started");
  Serial.print("Connected to the WiFi network. IP: ");
  Serial.println(WiFi.localIP());
}
 
// Handler for root endpoint "/"
void handle_JsonResponse(){
  String response;
  response += "{ \"imu\": { \"pitch\": ";
  response += String(pitch, 6); // Include pitch with 6 decimal precision
  response += ", \"roll\": ";
  response += String(roll, 6);
  response += ", \"yaw\": ";
  response += String(yaw, 6);
  response += ", \"temperature\": ";
  response += String(temperature, 6);
  response += " } }";
  Serial.println(response); // Debug log
  digitalWrite(M5_LED, 1);  // Toggle onboard LED

  server.send(200, "application/json", response); // Send JSON response
}

// Handler for "/gyro/" endpoint
void handle_GyroResponse(){
  String response = "{ \"gyro\": { \"pitch\": ";
  response += String(pitch, 6);
  response += ", \"roll\": ";
  response += String(roll, 6);
  response += ", \"yaw\": ";
  response += String(yaw, 6);
  response += " } }";
  Serial.println(response);
  server.send(200, "application/json", response);
}

// Handler for "/accel/" endpoint
void handle_AccelResponse(){
  String response = "{ \"accel\": { \"x\": ";
  response += String(accelX, 6);
  response += ", \"y\": ";
  response += String(accelY, 6);
  response += ", \"z\": ";
  response += String(accelZ, 6);
  response += " } }";
  Serial.println(response);
  server.send(200, "application/json", response);
}

// Handler for "/temp/" endpoint
void handle_TempResponse(){
  String response = "{ \"temperature\": ";
  response += String(temperature, 6);
  response += " }";
  Serial.println(response);
  server.send(200, "application/json", response);
}

// Turn ON LED handler (endpoint: /led/1)
void handle_LedOn(){
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, LOW); // LED is active LOW
  server.send(200, "text/plain", "LED is ON");
}

// Turn OFF LED handler (endpoint: /led/0)
void handle_LedOff(){
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, HIGH); // LED is OFF
  server.send(200, "text/plain", "LED is OFF");
}

// Buzzer ON (endpoint: /buzzer/1)
void handle_BuzzerOn(){
  M5.Beep.tone(100); // Produce a tone
  server.send(200, "text/plain", "Buzzer is ON");
}

// Buzzer OFF (endpoint: /buzzer/0)
void handle_BuzzerOff(){
  M5.Beep.tone(0); // Stop tone
  server.send(200, "text/plain", "Buzzer is OFF");
}

// Handle unknown endpoints
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

// Read gyroscope data
bool readGyro(){
  M5.IMU.getAhrsData(&pitch, &roll, &yaw); // Get pitch, roll, yaw
  return true;
}

// Read temperature from IMU sensor and convert F to C
float readTemperature(){
  float t;
  M5.IMU.getTempData(&t); // Read temperature (°F)
  t = (t - 32.0) * 5.0 / 9.0; // Convert to Celsius
  return t;
}

// Read accelerometer data
bool readAccel(){
  M5.IMU.getAccelData(&accelX, &accelY, &accelZ); // Read X, Y, Z acceleration
  return true;
}

uint8_t setup_flag = 1; // Used to control update on screen

// Reads all sensor data and displays on LCD
bool readSensors() {
  bool status = readGyro() && readAccel();
  if (status == true) {
    M5.Lcd.setCursor(0, 40, 2);
    M5.Lcd.printf("                                        "); // Clear area
  }

  // Debug prints for gyro data
  Serial.println("Gyro:");
  Serial.print("Pitch[X]: "); Serial.println(pitch);
  Serial.print("Roll[Y]: "); Serial.println(roll);
  Serial.print("Yaw[Z]: "); Serial.println(yaw);

  // Read and print temperature
  temperature = readTemperature();
  M5.Lcd.setCursor(0, 60, 2);
  M5.Lcd.printf("Temperature = %2.1f", temperature);
  Serial.print("Temperature: ");
  Serial.println(temperature);

  return status;
}
 
void loop() {
  server.handleClient(); // Handle any incoming HTTP requests

  // Update LCD once after boot
  if (setup_flag == 1) {
    M5.Lcd.setCursor(0, 40, 2);
    M5.Lcd.printf("X = %3.2f, Y = %3.2f, Z = %3.2f", pitch, roll, yaw);
    M5.Lcd.setCursor(0, 60, 2);
    M5.Lcd.printf("Temperature = %2.1f", temperature);
  }

  // If button was pressed, refresh sensor data
  if (!setup_flag) {
    setup_flag = 1;
    bool status = readSensors();
    if (status)
      Serial.print("\n\rRead Sensors success...\n");
    else
      Serial.print("\n\rRead Sensors failed...\n");
  }

  // Button press logic: detect press to update sensor values
  if (digitalRead(M5_BUTTON_HOME) == LOW) {
    setup_flag = 0;
    while (digitalRead(M5_BUTTON_HOME) == LOW); // Wait for release
  }
}
