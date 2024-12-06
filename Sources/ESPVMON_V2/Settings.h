
const char* settingsFile = "/settings.json";

struct Settings {
  bool WfifMode;
  int brightness;
};

Settings deviceSettings;


bool InitSPIFFS() {
  return SPIFFS.begin();
}

bool saveSettings() {
  DynamicJsonDocument jsonDoc(256);  // Adjust size as needed
  jsonDoc["WfifMode"] = deviceSettings.WfifMode;

  File file = SPIFFS.open(settingsFile, "w");
  if (!file) {
    Serial.println("Failed to open settings file for writing");
    return false;
  }

  if (serializeJson(jsonDoc, file) == 0) {
    Serial.println("Failed to write settings to file");
    return false;
  }
  file.close();
  return true;
}

// Load settings from SPIFFS
bool loadSettings() {
  File file = SPIFFS.open(settingsFile, "r");

  if (!file) {
    deviceSettings.WfifMode = false;
    saveSettings();
    return false;
  }

  DynamicJsonDocument jsonDoc(256);  // Adjust size as needed
  DeserializationError error = deserializeJson(jsonDoc, file);
  if (error) {
    return false;
  }

  deviceSettings.WfifMode = jsonDoc["WfifMode"];

  file.close();
  return true;
}