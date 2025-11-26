#include <WiFi.h>
#include <WiFiClient.h>

// ===================================
// === 1. CONFIGURATION (Wi-Fi & Pins)
// ===================================

// **CHANGE THESE** to your Wi-Fi credentials
const char* ssid = "Aqui amiga";
const char* password = "iljo2753";

// Server configuration
const int serverPort = 8888;
WiFiServer server(serverPort);

// Built-in LED Pin (usually pin 2)
const int ledPin = 2;

// Motor pins (L298N)
#define ENA 14 // ENA/ENB are now just digital pins
#define IN1 27
#define IN2 26
#define ENB 32
#define IN3 33
#define IN4 25

// Encoder pins
#define ENC_R_DO 35
#define ENC_L_DO 34

// Calibration constants
const float CM_PER_TICK = 0.603;
const int MOVE_DISTANCE_CM = 30; // Standard distance for FWD/BACK commands

// ===================================
// === 2. VARIABLES & INTERRUPTS
// ===================================

volatile long ticksR = 0, ticksL = 0;
volatile int dirR = 1, dirL = 1;

// Encoder Interrupt Service Routines (ISRs)
void IRAM_ATTR onRight() { ticksR += dirR; }
void IRAM_ATTR onLeft()  { ticksL += dirL; }

// ===================================
// === 3. MOTOR CONTROL FUNCTIONS
// ===================================

// NOTE: The 'pwm' parameter is included but IGNORED, as we can only use full power (HIGH).
void setRight(int pwm, int dir) { 
  dirR = dir;
  digitalWrite(IN1, dir > 0 ? HIGH : LOW);
  digitalWrite(IN2, dir > 0 ? LOW : HIGH);
  digitalWrite(ENA, HIGH); // Motor Right ON (full power)
}

void setLeft(int pwm, int dir) {
  dirL = dir;
  digitalWrite(IN3, dir > 0 ? HIGH : LOW);
  digitalWrite(IN4, dir > 0 ? LOW : HIGH);
  digitalWrite(ENB, HIGH); // Motor Left ON (full power)
}

void stopMotors(){
  digitalWrite(ENA, LOW); // Motor Right OFF
  digitalWrite(ENB, LOW); // Motor Left OFF
  // Ensure the direction pins are LOW for safety
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, LOW);
}

// ===================================
// === 4. MOVEMENT EXECUTION FUNCTION
// ===================================

String executeMotion(String dir, int targetCm) {
  // Use a placeholder PWM value (like 200) since it's ignored anyway.
  const int PLACEHOLDER_PWM = 200; 
  
  long targetTicks = (long)(targetCm / CM_PER_TICK);
  int direction = dir.equalsIgnoreCase("forward") ? 1 : -1;
  
  // Reset encoders and start movement
  noInterrupts(); 
  ticksR = 0; 
  ticksL = 0; 
  interrupts();
  
  setRight(PLACEHOLDER_PWM, direction);
  setLeft(PLACEHOLDER_PWM, direction);

  // Block until target is met (Basic control)
  while (abs(ticksR) < targetTicks && abs(ticksL) < targetTicks) {
    // Wait for the encoders to count up.
  }
  
  stopMotors();

  // Report movement statistics
  long finalR, finalL; 
  noInterrupts(); 
  finalR = ticksR; 
  finalL = ticksL; 
  interrupts();
  
  float avgTicks = (abs(finalR) + abs(finalL)) / 2.0;
  float distCm = avgTicks * CM_PER_TICK;
  
  char buffer[150];
  snprintf(buffer, sizeof(buffer), 
    "ROGER: [%s] Moved %.2f cm. Ticks R=%ld L=%ld", 
    dir.c_str(), distCm, finalR, finalL);
    
  return String(buffer);
}

// ===================================
// === 5. SETUP (Initialization)
// ===================================

void setup() {
  Serial.begin(115200);
  
  // 5a. Motor Pin Setup (All pins are now simple digital OUTPUTs)
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); // ENA/ENB must be set as OUTPUT
  pinMode(ENB, OUTPUT); 

  // 5b. LED Pin Setup
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 5c. Encoder Interrupt Setup
  pinMode(ENC_R_DO, INPUT);
  pinMode(ENC_L_DO, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_R_DO), onRight, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_L_DO), onLeft, RISING);

  // 5d. Wi-Fi Connection
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("TCP Server started on port 8888.");
}

// ===================================
// === 6. LOOP (Main Execution)
// ===================================

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected.");
    String command = "";
    String response = "";
    bool received = false;

    // Read the incoming command until newline (\n)
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          received = true;
          break;
        }
        command += c;
      }
    }

    // Process the command
    if (received) {
      command.trim();
      Serial.print("Received command: [");
      Serial.print(command);
      Serial.println("]");

      // --- Command Handling ---
      if (command.equalsIgnoreCase("LED_ON")) {
        digitalWrite(ledPin, HIGH);
        response = "ROGER: LED is ON.";

      } else if (command.equalsIgnoreCase("LED_OFF")) {
        digitalWrite(ledPin, LOW);
        response = "ROGER: LED is OFF.";
      
      } else if (command.equalsIgnoreCase("forward") || command.equalsIgnoreCase("f")) {
        response = executeMotion("forward", MOVE_DISTANCE_CM);

      } else if (command.equalsIgnoreCase("back") || command.equalsIgnoreCase("b")) {
        response = executeMotion("backward", MOVE_DISTANCE_CM);

      } else if (command.equalsIgnoreCase("stop") || command.equalsIgnoreCase("s")) {
        stopMotors();
        response = "ROGER: Motors stopped.";

      } else {
        response = "ERROR: Unknown command. Send LED_ON, forward, back, or stop.";
      }
      
      // Send response back to the Python client
      client.println(response);
      Serial.print("Sent response: ");
      Serial.println(response);
    }
    
    client.stop();
    Serial.println("Client disconnected.");
  }
}