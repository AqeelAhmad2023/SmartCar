#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "aa";
const char* password = "abcd1234";
WiFiUDP udp;
unsigned int localUdpPort = 4210;
IPAddress broadcastIP(192, 168, 115, 255);
char incomingPacket[255];

int PWMA = 5;
int PWMB = 4;
int DA = 0;
int DB = 2;
//because i m using two sensors
const int trigPin1 = D5;
const int echoPin1 = D6;
const int trigPin2 = D7;
const int echoPin2 = D8;

void setup() {
  Serial.begin(9600);
  setupPins();
  setupWiFi();
  udp.begin(localUdpPort);
}

void loop() {
  checkWiFiAndReconnect();
  long distance1 = measureDistance(trigPin1, echoPin1);
  long distance2 = measureDistance(trigPin2, echoPin2);
  actBasedOnDistance(distance1, distance2);
  checkForMessages();
  delay(100);
}

void setupPins() {
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(DA, OUTPUT);
  pinMode(DB, OUTPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
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
      moveForward();
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
  return duration * 0.034 / 2;
}

void actBasedOnDistance(long distance1, long distance2) {
  if (distance1 < 20 || distance2 < 20) {
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
  delay(1000); // Adjust based on actual turn needed
  digitalWrite(PWMA, LOW);
  digitalWrite(PWMB, LOW);
}

void checkWiFiAndReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting reconnection...");
    setupWiFi();
  }
}
