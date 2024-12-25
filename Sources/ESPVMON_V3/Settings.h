
const char* settingsFile = "/Settings.json";

struct Settings {
  bool WfifMode;
  double ShuntValue;
  double ShuntCurrent;
  int VBusAVG;
  int VShuntAVG;
  int TimeAVG;
};

Settings deviceSettings;


bool InitSPIFFS() {
  return SPIFFS.begin();
}

//Save settings return false if had any errors
bool SaveSettings() {
  DynamicJsonDocument jsonDoc(256);

  //Store settings in json doc
  jsonDoc["WfifMode"] = deviceSettings.WfifMode;
  jsonDoc["ShuntValue"] = deviceSettings.ShuntValue;
  jsonDoc["ShuntCurrent"] = deviceSettings.ShuntCurrent;
  jsonDoc["VBusAVG"] = deviceSettings.VBusAVG;
  jsonDoc["VShuntAVG"] = deviceSettings.VShuntAVG;
  jsonDoc["TimeAVG"] = deviceSettings.TimeAVG;

  File file = SPIFFS.open(settingsFile, "w");
  if (!file) {
    //Serial.println("Failed to open settings file for writing");
    return false;
  }

  if (serializeJson(jsonDoc, file) == 0) {
    //Serial.println("Failed to write settings to file");
    return false;
  }

  file.close();
  return true;
}

// Load settings, return false if had any errors
bool LoadSettings() {
  bool HasError = false;

  File file = SPIFFS.open(settingsFile, "r");

  if (!file) {
    HasError = true;
  }

  //Nice, we has file with settings
  DynamicJsonDocument jsonDoc(256);
  DeserializationError error = deserializeJson(jsonDoc, file);

  if (error) {
    HasError = true;
  }

  //If we had any errors - just reset settings to default
  if (HasError) {
    //Serial.println("Settings error");
    deviceSettings.WfifMode = false;
    deviceSettings.ShuntValue = 0.0015;
    deviceSettings.ShuntCurrent = 50;
    deviceSettings.VBusAVG = 4;
    deviceSettings.VShuntAVG = 4;
    deviceSettings.TimeAVG = 4;

    SaveSettings();
    return false;
  }

  deviceSettings.WfifMode = jsonDoc["WfifMode"];
  deviceSettings.ShuntValue = jsonDoc["ShuntValue"];
  deviceSettings.ShuntCurrent = jsonDoc["ShuntCurrent"];
  deviceSettings.VBusAVG = jsonDoc["VBusAVG"];
  deviceSettings.VShuntAVG = jsonDoc["VShuntAVG"];
  deviceSettings.TimeAVG = jsonDoc["TimeAVG"];

  file.close();
  return true;
}