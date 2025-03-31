#include <painlessMesh.h>                 // Include PainlessMesh library for wireless mesh networking
#include <M5StickCPlus.h>                // Include M5StickC Plus library for hardware support

// Mesh network configuration
#define MESH_PREFIX     "M5MeshMesh"     // Name of the mesh network (SSID-like string)
#define MESH_PASSWORD   "password123"    // Password for mesh network
#define MESH_PORT       5555             // Port for communication within mesh

Scheduler userScheduler;                // Task scheduler for painlessMesh
painlessMesh mesh;                      // Mesh object to manage networking

// Define priority levels for incoming messages
enum MessagePriority {
    PRIORITY_HIGH = 0,                  // Highest priority
    PRIORITY_MEDIUM = 1,
    PRIORITY_LOW = 2                    // Lowest priority
};

// Queues to store incoming messages based on their priority
SimpleList<String> highPriorityQueue;
SimpleList<String> mediumPriorityQueue;
SimpleList<String> lowPriorityQueue;

void processReceivedMessages();         // Function prototype for processing messages

// Callback function that triggers when a message is received
void receivedCallback(uint32_t from, String & msg) {
    int separatorIndex = msg.indexOf('|');             // Look for delimiter in the message
    if (separatorIndex == -1) {                        // If no delimiter, it's invalid
        Serial.println("Invalid message format");
        return;
    }

    String priorityStr = msg.substring(0, separatorIndex); // Extract priority from message
    String actualMsg = msg.substring(separatorIndex + 1);  // Extract the actual message

    // Convert priority string to enum
    MessagePriority priority;
    if (priorityStr == "0") priority = PRIORITY_HIGH;
    else if (priorityStr == "1") priority = PRIORITY_MEDIUM;
    else priority = PRIORITY_LOW;

    // Push message into appropriate priority queue
    switch (priority) {
        case PRIORITY_HIGH:
            highPriorityQueue.push_back(actualMsg);
            break;
        case PRIORITY_MEDIUM:
            mediumPriorityQueue.push_back(actualMsg);
            break;
        case PRIORITY_LOW:
            lowPriorityQueue.push_back(actualMsg);
            break;
    }

    Serial.printf("Received from %u: %s", from, actualMsg.c_str());

    processReceivedMessages();  // Always process the most important message to display
}

// Display the highest priority message on screen
void processReceivedMessages() {
    String msgToDisplay;

    // Select message based on priority queues
    if (!highPriorityQueue.empty()) {
        msgToDisplay = highPriorityQueue.front();
        highPriorityQueue.pop_front();
    } else if (!mediumPriorityQueue.empty()) {
        msgToDisplay = mediumPriorityQueue.front();
        mediumPriorityQueue.pop_front();
    } else if (!lowPriorityQueue.empty()) {
        msgToDisplay = lowPriorityQueue.front();
        lowPriorityQueue.pop_front();
    }

    // Display message if any
    if (!msgToDisplay.isEmpty()) {
        M5.Lcd.fillRect(0, 10, 160, 70, BLACK);   // Clear display area
        M5.Lcd.setCursor(10, 20);                 // Set cursor position
        M5.Lcd.setTextColor(WHITE);              // Set text color
        M5.Lcd.setTextSize(2);                   // Set text size
        M5.Lcd.print("Msg: ");                    // Print label
        M5.Lcd.println(msgToDisplay);            // Print the message content
    }
}

// Task: Sends a random message with random priority every 1–5 seconds
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, []() {
    MessagePriority priority = static_cast<MessagePriority>(random(0, 3)); // Random priority (0–2)
    String msg = String(priority) + "|" + "Hello from Daryl" + String(mesh.getNodeId());

    Serial.printf("Sending message: %s with priority %d", msg.c_str(), priority); // Log the message
    mesh.sendBroadcast(msg);                     // Send message to all nodes

    taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5)); // Randomize interval
});

void setup() {
    M5.begin();                                   // Initialize M5StickC
    M5.Lcd.setRotation(1);                        // Set screen orientation
    M5.Lcd.fillScreen(BLACK);                     // Clear the screen
    M5.Lcd.setCursor(10, 10);                     // Set cursor for initial display
    M5.Lcd.setTextColor(WHITE);                   // Set text color
    M5.Lcd.setTextSize(2);                        // Set font size
    M5.Lcd.println("M5StickC Mesh");             // Show welcome text

    Serial.begin(115200);                         // Start serial communication

    // Initialize mesh network
    mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION); // Enable debug logging
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT); // Initialize mesh with settings
    mesh.onReceive(&receivedCallback);            // Register callback function for receiving

    userScheduler.addTask(taskSendMessage);       // Add the message sending task to the scheduler
    taskSendMessage.enable();                     // Start the task
}

void loop() {
    mesh.update();                                // Required loop to keep mesh network alive
}
