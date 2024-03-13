#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "aa"; // Your WiFi network name
const char* password = "abcd1234"; // Your WiFi network password

WiFiUDP udp;
unsigned int localUdpPort = 4210; 
IPAddress broadcastIP(192, 168, 115, 255);
char incomingPacket[255]; 

int PWMA = 5; // Motor A PWM control
int PWMB = 4; // Motor B PWM control
int DA = 0;   // Motor A Direction control
int DB = 2;   // Motor B Direction control

const int trigPin = D8; // Ultrasonic Trigger Pin
const int echoPin = D7; // Ultrasonic Echo Pin

void setup() {
  Serial.begin(9600);
  setupPins();
  setupWiFi();
  udp.begin(localUdpPort);
  Serial.println("Car 2 Setup complete");
}

void loop() {
  checkWiFiAndReconnect();
  long distance = measureDistance(trigPin, echoPin);
  actBasedOnDistance(distance);
  checkForMessages();
  delay(100);
}

void setupPins() {
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(DA, OUTPUT);
  pinMode(DB, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(PWMA, LOW);
  digitalWrite(PWMB, LOW);
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void sendMessage(const char* message) {
  if (udp.beginPacket(broadcastIP, localUdpPort)) {
    udp.write(message);
    if (!udp.endPacket()) {
      Serial.println("Failed to send message");
    }
  } else {
    Serial.println("UDP Begin Packet failed");
  }
}

void checkForMessages() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    if (String(incomingPacket) == "OBSTACLE") {
      stop();
      delay(500); // Short pause after stopping
      turnRight(); // Execute a right turn
      delay(500); // Allow time for the turn before moving forward again
      moveForward(); // Proceed forward after the turn
    }
  }
}

long measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2; // Convert to distance
}

void actBasedOnDistance(long distance) {
  if (distance < 20) {
    stop();
    sendMessage("OBSTACLE");
    delay(500);
    turnRight();
    delay(500);
    moveForward();
  } else {
    moveForward();
  }
}

void moveForward() {
  digitalWrite(DA, LOW);
  digitalWrite(DB, LOW);
  analogWrite(PWMA, 1023);
  analogWrite(PWMB, 1023);
}

void stop() {
  digitalWrite(PWMA, LOW);
  digitalWrite(PWMB, LOW);
}

void turnRight() {
  digitalWrite(DA, HIGH);
  digitalWrite(DB, LOW);
  analogWrite(PWMA, 1023);
  analogWrite(PWMB, 0);
  delay(1000); // Adjust based on the actual turn needed
  digitalWrite(PWMA, LOW);
  digitalWrite(PWMB, LOW);
}

void checkWiFiAndReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting reconnection...");
    setupWiFi(); // Attempt to reconnect to WiFi
  }
}
