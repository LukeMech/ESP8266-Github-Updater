//      <-- Libraries -->
#include "secrets.h"

// Wifi and certificates
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
BearSSL::CertStore certStore;
// Time for certificate validation
#include <time.h>

//HTTP
#include <ESP8266HTTPClient.h>

// Updater
#include <ESP8266httpUpdate.h>

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


//                      <--- Firmware updater --->

// DigiCert High Assurance EV Root CA
const char trustRoot[] PROGMEM = R"EOF( 
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";
X509List cert(trustRoot);

void firmwareUpdate()  // Updater
{

  WiFiClientSecure client;  // Create secure wifi client
  client.setTrustAnchors(&cert);

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // Set time via NTP, as required for x.509 validation
  time_t now = time(nullptr);

  if (!client.connect(host, httpsPort)) {  // Connect to github
    return;
  }

  HTTPClient http;  // Connect to release API
  http.begin(client, updaterVersionCtrlUrl);
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    return;
  }

  String new_version = http.getString();  //Download version tag
  new_version.trim();
  http.end();

  if (!strcmp(new_version.c_str(), firmwareVer)) {  // Check if version is the same
    return;
  } 
  else if (!new_version.c_str() || new_version.c_str() == "") {
    return;
  }

  ESPhttpUpdate.setLedPin(LED_BUILTIN);
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, updaterFirmwareUrl);  // Update firmware
  if (ret) {  // Error
    return();
  }
}


//                      <--- Setup and loop --->

void setup() {
  Serial.begin(9600);  // Begin serial
  Serial.println("[STATUS] Start!");

  if (!SPIFFS.begin()) ESP.restart();              // Begin filesystem 
    
  if (WiFi_UpdateCredentialsFile) saveWifiCfg();  // Save network config
  wiFiInit();  // Connect to wifi

  pinMode(LED_BUILTIN, OUTPUT);  // Set pin modes
}


unsigned long previousMillis = 0;  // will store last time firmware updater was called
void loop() {

  const unsigned long currentMillis = millis();

  if ((currentMillis - previousMillis) >= 10000) {  // If interval time passed
    previousMillis = currentMillis;
    firmwareUpdate();  // Call updater function
  }
}