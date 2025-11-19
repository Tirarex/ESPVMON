//Minimal mode, only OLED Display and value measurement
//#define MINIMALMODE
#define ENABLESCREEN

#ifdef MINIMALMODE
#define MaxCurrent 20
#define ShuntValue 0.002
#endif

#ifndef MINIMALMODE
//Web server and updates
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>

//Wifi Manager
#include <WiFiManager.h>
#endif

//Devices
#include <Wire.h>
#include <GyverINA.h>

#ifdef ENABLESCREEN
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif


#ifndef MINIMALMODE
//Settings
#include <FS.h>           // Filesystem library for SPIFFS
#endif

//Sum code
#ifndef MINIMALMODE
#include "Settings.h"  // Logic for save and load settings from SPIFFS
#endif

#include "DataLogic.h"  // Ina226 data and settings

#ifdef ENABLESCREEN
#include "Screen.h"  //All oled0.96 logic and menu screens
#endif

#ifndef MINIMALMODE
#include "Buttons.h"  //Very basic buttons
#include "Network.h"  //Wifi, ota, serial things
#endif

unsigned long lastUpdateTime = 0;

void setup() {

  //Serial output
  Serial.begin(115200);
 

#ifdef ENABLESCREEN
       //Display and buttons
  InitDisplay();
  ShowLogo();
#endif

#ifndef MINIMALMODE
  InitButtons();

  //Load settings from memory
  if (InitSPIFFS()) {
    ShowText("Memory initialized");
  } else {
    ShowText("SPIFFS not wotking");
  }
  if (LoadSettings()) {
    ShowText("Settings loaded");

  } else {
    ShowText("Using default settings");
  }

  Shunt_Value = deviceSettings.ShuntValue;
  Shunt_Max_Current = deviceSettings.ShuntCurrent;

  //Wifi initialization
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
#else
#ifdef ENABLESCREEN
  ShowText("Minimal mode");
  delay(1000);
#endif
#endif


#ifndef MINIMALMODE

  //Ina226 initialization
  if (ina.begin()) {
    ShowText(F("Ina226 found"));
    SetInaSettings();
    updateShunt(deviceSettings.ShuntValue, deviceSettings.ShuntCurrent);
  } else {
    //May be auto reset will help?

#ifdef ENABLESCREEN
    ShowText(F("Ina226 not found"));
    delay(1000);
#endif
  }



#else
  ina.begin(MaxCurrent, ShuntValue);
  ina.setAveraging(INA226_AVG_X64);
  ina.setSampleTime(INA226_VBUS, INA226_CONV_588US);
  ina.setSampleTime(INA226_VSHUNT, INA226_CONV_1100US);
  updateShunt(MaxCurrent, ShuntValue);
#endif

#ifndef MINIMALMODE
  //init web server only if we have wifi
  if (deviceSettings.WfifMode) {
    InitServer();
  }
#endif
}


void loop() {
  //Read ina226 data
  FetchIna();

  //Capacity update timer
  if (millis() - lastUpdateTime >= 1000) {
    calculateCapacity();
    lastUpdateTime = millis();
  }

#ifndef MINIMALMODE
  HandleButtons();

  switch (ScreenPage) {
    case 0:  // Main VAW Screen
      MainScreen();

      if (button1State) {
        unsigned long elapsedTime = millis() - button1PressTime;
        if (elapsedTime <= 2000) {
          int progress = (elapsedTime * 100) / 2000;
          drawProgressBar(progress);
        } else {
          button1State = false;
          ScreenBrightness--;
          if (ScreenBrightness < 0) ScreenBrightness = 1;
          if (ScreenBrightness == 1) setBrightness(255);
          if (ScreenBrightness == 0) setBrightness(1);
        }
      }

      if (B2Click) {
        if (ScreenBrightness > 0) {
          ScreenPage = 1;

          if (button1State) {
            ScreenPage = 3;
          }
        }
      }
      break;
    case 1:  //Capacity Measurements
      CapacityScreen();
      if (button1State) {
        unsigned long elapsedTime = millis() - button1PressTime;
        if (elapsedTime <= 2000) {
          int progress = (elapsedTime * 100) / 2000;
          drawProgressBar(progress);
        } else {
          FlushCapacity();
        }
      }
      if (B2Click) {
        ScreenPage = 2;
      }
      break;
    case 2:  //Peaks Measurements
      PeaksScreen();
      if (button1State) {
        unsigned long elapsedTime = millis() - button1PressTime;
        if (elapsedTime <= 2000) {
          int progress = (elapsedTime * 100) / 2000;
          drawProgressBar(progress);
        } else {
          FlushPeakData();
        }
      }
      if (B2Click) {
        ScreenPage = 0;
      }
      break;
    case 3:  //Settings
      SettingsScreen();
      if (ListValue == 0) ListValue = 1;
      if (B1Click) {
        if (ListValue == 1) {  //Sampling screen
          ListValue = 0;
          ScreenPage = 4;
        }
        if (ListValue == 2) {  // shunt screen
          ListValue = 0;
          ScreenPage = 5;
        }
        if (ListValue == 3) {  //Wifi screen
          ListValue = 0;
          ScreenPage = 6;
        }
        if (ListValue == 4) {  //Exit from list
          ListValue = 0;
          ScreenPage = 0;
        }
      }
      if (B2Click) {
        ListValue++;
        if (ListValue > 4) ListValue = 1;
      }
      break;
    case 4:  //Sampling settings
      SamplingScreen();
      if (B1Click) {
        if (ListValue == 1) {
          deviceSettings.TimeAVG++;
          if (deviceSettings.TimeAVG > 7) deviceSettings.TimeAVG = 0;
        }
        if (ListValue == 2) {
          deviceSettings.VBusAVG++;
          if (deviceSettings.VBusAVG > 7) deviceSettings.VBusAVG = 0;
        }
        if (ListValue == 3) {
          deviceSettings.VShuntAVG++;
          if (deviceSettings.VShuntAVG > 7) deviceSettings.VShuntAVG = 0;
        }
        SetInaSettings();
        if (ListValue == 4) {
          ScreenPage = 3;
          ListValue = 1;
          SaveSettings();
          ShowAlert(F("Saved"));
        }
      }
      if (B2Click) {
        ListValue++;
        if (ListValue > 4) ListValue = 1;
      }
      break;
    case 5:  //Shunt settings
      ShuntScreen();
      if (B1Click) {
        if (ListValue == 4) {
          ScreenPage = 3;
          ListValue = 2;
          deviceSettings.ShuntValue = Shunt_Value;
          deviceSettings.ShuntCurrent = Shunt_Max_Current;
          SaveSettings();
          ShowAlert(F("Saved"));
        }
        //Shunt resistance
        if (ListValue == 0) Shunt_Value = Shunt_Value - 0.000001f;
        if (ListValue == 1) Shunt_Value = Shunt_Value + 0.000001f;
        if (Shunt_Value < 0.0001) Shunt_Value = 0.00010f;
        //Shunt current
        if (ListValue == 2) Shunt_Max_Current = Shunt_Max_Current - 1;
        if (ListValue == 3) Shunt_Max_Current = Shunt_Max_Current + 1;
        if (Shunt_Max_Current < 1) Shunt_Max_Current = 1;
        updateShunt(Shunt_Value, Shunt_Max_Current);
      }
      //Long press of button go BRRR
      if (button1State & millis() - lastDebounceTime1 >= longPressDuration) {
        if (ListValue == 0) Shunt_Value = Shunt_Value - 0.00001f;
        if (ListValue == 1) Shunt_Value = Shunt_Value + 0.00001f;
        if (ListValue == 2) Shunt_Max_Current = Shunt_Max_Current - 1;
        if (ListValue == 3) Shunt_Max_Current = Shunt_Max_Current + 1;

        if (Shunt_Value < 0.0001) Shunt_Value = 0.0001f;
        if (Shunt_Max_Current < 1) Shunt_Max_Current = 1;

        //No way bro
        if (Shunt_Max_Current > 500) Shunt_Max_Current = 500;

        updateShunt(Shunt_Value, Shunt_Max_Current);
        delay(10);  //Make BRRR Slower
      }
      if (B2Click) {
        ListValue++;
        if (ListValue > 4) ListValue = 0;
      }

      break;
    case 6:  //Communication Settings
      WifiScreen();
      if (B1Click) {
        if (ListValue == 1) {
          deviceSettings.WfifMode = !deviceSettings.WfifMode;
          SaveSettings();
          ShowAlert(F("Reboot"));
          ESP.restart();
        }
        if (ListValue == 2) {
          deviceSettings.WfifMode = true;
          WiFiManager wifiManager;
          wifiManager.resetSettings();
          SaveSettings();
          ShowAlert(F("Reboot"));
          ESP.restart();
        }
        if (ListValue == 3) {
          ScreenPage = 3;
          ListValue = 3;
        }
      }
      if (B2Click) {
        ListValue++;
        if (ListValue > 3) ListValue = 1;
      }
      break;
  }
  ShowConnection = false;
#else

#ifdef ENABLESCREEN
     MainScreen();
#endif

 
#endif

#ifdef ENABLESCREEN
      display.display();
#endif
 

#ifndef MINIMALMODE
  FlushButtons();
  if (deviceSettings.WfifMode) {
    server.handleClient();
  }
#endif
  delay(10);  //10ms delay is important for power saving, but we can go more why not?
}
