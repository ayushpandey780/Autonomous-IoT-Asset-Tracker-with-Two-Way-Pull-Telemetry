/*
  * Project: Autonomous IoT Asset Tracker
  * Description: Two-Way Pull Telemetry Firmware for ESP32 + BN-220 GPS
  * Author: Ayush Pandey
  * Hardware: ESP32 Dev Board, BN-220 GNSS Module
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>

// ---------------- SETTINGS ----------------
// 🔥 UPDATE THESE WITH YOUR CREDENTIALS BEFORE UPLOADING TO YOUR PHYSICAL BOARD
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// 🔥 UPDATE WITH YOUR ACTIVE NGROK URL
String SERVER_URL = "http://YOUR_NGROK_URL.ngrok-free.dev";

// ---------------- HARDWARE ----------------
#define GPS_RX 18
#define GPS_TX 19
#define GPS_BAUD 9600

// ---------------- GLOBALS ----------------
TinyGPSPlus gps;

// ------------------------------------------------
// SETUP
// ------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  delay(2000);
  Serial.println("\n--- WHATSAPP INTERROGATOR DEMO ---");

  // 1️⃣ WAIT FOR GPS LOCK FIRST (Prevents Wi-Fi RF Interference)
  Serial.print("⏳ Waiting for GPS Lock...");
  
  while (!gps.location.isValid()) {
    while (Serial1.available() > 0) {
      gps.encode(Serial1.read());
    }
    // Print a dot every 1 second to indicate active polling
    static unsigned long lastDotTime = 0;
    if (millis() - lastDotTime > 1000) {
      Serial.print(".");
      lastDotTime = millis();
    }
  }

  Serial.println("\n✅ GPS LOCKED!");

  // 2️⃣ CONNECT TO WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
}

// ------------------------------------------------
// LOOP
// ------------------------------------------------
void loop() {
  // 1. Keep reading GPS constantly so data is never stale
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // 2. Poll the Node.js server every 5 seconds
  static unsigned long lastCheckTime = 0;
  if (millis() - lastCheckTime > 5000) {
    lastCheckTime = millis();

    if (checkForWhatsAppCommand()) {
      Serial.println("\n🚨 COMMAND RECEIVED! Dispatching Location...");
      
      float lat = 0.0;
      float lng = 0.0;
      float spd = 0.0;

      // Verify GPS is still valid before extracting coordinates
      if (gps.location.isValid()) {
        lat = gps.location.lat();
        lng = gps.location.lng();
        spd = gps.speed.kmph();
      }

      // Dispatch the payload
      sendHTTP(lat, lng, spd);
    }
  }
}

// ------------------------------------------------
// 📡 CHECK SERVER FOR INCOMING WHATSAPP TRIGGER
// ------------------------------------------------
bool checkForWhatsAppCommand() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = SERVER_URL + "/check";
    
    http.begin(url);
    int httpResponseCode = http.GET();
    bool shouldTrigger = false;

    if (httpResponseCode == 200) {
      String response = http.getString();
      if (response == "YES") {
        shouldTrigger = true;
      }
    }
    http.end();
    return shouldTrigger;
  }
  return false;
}

// ------------------------------------------------
// 🌐 SEND TELEMETRY DATA TO NODE.JS GATEWAY
// ------------------------------------------------
void sendHTTP(float lat, float lng, float speed) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = SERVER_URL + "/track?lat=" + String(lat, 6) + "&lon=" + String(lng, 6) + "&spd=" + String(speed);
    
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      Serial.println("✅ Location Successfully Dispatched to API Gateway.");
    } else {
      Serial.println("❌ Failed to dispatch.");
    }
    http.end();
  }
}
