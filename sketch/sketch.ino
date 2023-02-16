//      <-- Libraries -->
#include "secrets.h"

// Wifi
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// Filesystem
#include <FS.h>


//                      <--- WiFi connector --->

void wiFiInit() {

  File file = SPIFFS.open("/network_config.txt", "r");  // Open wifi config file
  String ssidFromFile = file.readStringUntil('\n');            // Read network info
  String passwordFromFile = file.readStringUntil('\n');
  file.close();

  ssidFromFile.trim();  // Delete spaces at the beginning and end
  passwordFromFile.trim();

  if (strcmp(ssidFromFile.c_str(), "")) {  // Check if SSID provided
    WiFi.begin(ssidFromFile.c_str(), passwordFromFile.c_str());  // If yes, try to connect
    Serial.print("[INFO] Connecting to: ");
    Serial.print(ssidFromFile.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }
  }
  else {
    Serial.println("[ERROR] No wifi info saved in storage!");
    for(; ;);   // Loop forever
  }
}

void saveWifiCfg()  //Save network info into file
{

  SPIFFS.remove("/network_config.txt");  // Recreate config file
  File file = SPIFFS.open("/network_config.txt", "w");
  file.println(ssid);  // Save info into file
  file.println(password);

  file.close();
}

//                      <--- Setup and loop --->

void setup() {
  Serial.begin(9600);  // Begin serial
  Serial.println("[STATUS] Start!");

  if (!SPIFFS.begin()) ESP.restart();              // Begin filesystem 

  if (WiFi_UpdateCredentialsFile) saveWifiCfg();   // Save network config

  wiFiInit();  // Connect to wifi

  pinMode(LED_BUILTIN, OUTPUT);  // Set onboard lamp
}


unsigned long previousMillis = 0;  // will store last time firmware updater was called
void loop() {

  const unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= 10000) {  // If interval time passed
    previousMillis = currentMillis;
 //   firmwareUpdate();  // Call updater function
  }

}