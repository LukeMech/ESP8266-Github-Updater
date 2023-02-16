const char* firmwareVer = "0.0.1";                                                                                        // Version number

// ------------------------
// -------- WiFi ----------
// ------------------------

const bool WiFi_UpdateCredentialsFile = false;  // Update network_config.txt in filesystem?
const char* ssid = "";                          // Network name
const char* password = "";                      // Network password


// ------------------------
// ------- Updater --------
// ------------------------

const char* host = "github.com";        // Host to check connection, leave as is if using github
const int httpsPort = 443;              // Host port

const char* updaterVersionCtrlUrl = "https://raw.githubusercontent.com/LukeMech/ESP8266-Github-Updater/main/updater/build-version.txt";  // Link to version.txt
const char* updaterFirmwareUrl = "https://raw.githubusercontent.com/LukeMech/ESP8266-Github-Updater/main/updater/firmware.bin";    // File to firmware.bin