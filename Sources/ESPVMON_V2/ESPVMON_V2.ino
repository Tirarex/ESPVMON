//Web server and updates
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

//Wifi credentials and connection manager
#include <WiFiManager.h>

//Devices
#include <Wire.h>
#include <GyverINA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Settings
#include <FS.h>           // Filesystem library for SPIFFS
#include <ArduinoJson.h>  // For JSON parsing and serialization

//Sum code
#include "DataLogic.h"
#include "Settings.h"
#include "Screen.h"
#include "Buttons.h"
#include "Network.h"

unsigned long lastUpdateTime = 0;


void setup() {

  InitDisplay();
  ShowText("ESPVMON V2");
  InitButtons();

  if (InitSPIFFS()) {
    ShowText("Memory initialized");
  } else {
    ShowText("SPIFFS not wotking");
  }

  if (loadSettings()) {
    ShowText("Settings loaded");
  } else {
    ShowText("Using default settings");
  }


  //Serial output
  Serial.begin(115200);

  if (deviceSettings.WfifMode) {
    ShowText(F("Connecting to wifi"));
    WiFiManager wifiManager;
    bool WifimanStatus = wifiManager.autoConnect("ESPVMON", "");
    if (!WifimanStatus) {
      ShowText(F("Failed to connect"));

      // no wifi no problem
      ShowText(F("Offline mode"));
      deviceSettings.WfifMode = false;
      WiFi.mode(WIFI_OFF);
      WiFi.forceSleepBegin();
    } else {
      ShowText(F("Wifi Connected"));
    }
  } else {
    //precious watts been saved
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
  }


  if (ina.begin()) {
    ShowText(F("Ina226 found"));
    // Configure INA226
    ina.begin();
    ina.setSampleTime(INA226_VBUS, INA226_CONV_1100US);
    ina.setSampleTime(INA226_VSHUNT, INA226_CONV_1100US);
    ina.setAveraging(INA226_AVG_X64);

  } else {
    ShowText(F("Ina226 not found"));
    delay(1000);
  }

  if (deviceSettings.WfifMode) {
    InitServer();
  }


  ShowLogo();
  
}


void loop() {

  if (millis() - lastUpdateTime >= 1000) {
    calculateCapacity();
    lastUpdateTime = millis();
  }

  HandleButtons();

  FetchIna();

  switch (ScreenPage) {
    case 0:  // Main VAW Screen

      MainScreen();

      if (B1Click) {
        ScreenBrightness--;
        if (ScreenBrightness < 0) ScreenBrightness = 1;

        if (ScreenBrightness == 1) setBrightness(255);
        if (ScreenBrightness == 0) setBrightness(1);
      }

      if (B2Click) {
        if (ScreenBrightness > 0) ScreenPage = 1;
      }
      break;
    case 1:  //Peaks Measurements

      PeaksScreen();

      if (B1Click) {
        FlushPeakData();
        ShowAlert(F("CLEARED"));
      }

      if (B2Click) {
        ScreenPage = 2;
      }

      break;
    case 2:  //Capacity Measurements

      CapacityScreen();

      if (B1Click) {
        FlushCapacity();
        ShowAlert(F("CLEARED"));
      }
      if (B2Click) {
        ScreenPage = 3;
      }
      break;

    case 3:  //Settings

      WifiScreen();

      if (B1Click) {
        deviceSettings.WfifMode = !deviceSettings.WfifMode;
        saveSettings();
        ShowAlert(F("Reboot"));
        ESP.restart();
      }

      if (B2Click) {
        ScreenPage = 0;
      }
      break;
  }
  ShowConnection = false;
  display.display();

  FlushButtons();

  if (deviceSettings.WfifMode) {
    server.handleClient();
  }

  delay(10);
}
