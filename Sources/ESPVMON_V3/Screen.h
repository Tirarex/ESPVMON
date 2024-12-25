
//OLED Screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int ScreenPage = 0;           //Page on screen
int ListValue = 0;            //Selected menu list item
char timeString[16];          //timer text
int animationIndex = 0;       //Activity indicator progress
int ScreenBrightness = 1;     //is screen on?
bool ShowConnection = false;  //data fetch event
int ofs = 10;                 //Offset betwen menu items

int averages[] = { 1, 4, 16, 64, 128, 256, 512, 1024 };
int avgtimes[] = { 140, 204, 332, 588, 1100, 2116, 4156, 8244 };

// WiFi icon bitmap (7x7)
const uint8_t wifiIcon[] = {
  0b00101000,
  0b00101000,
  0b01111100,
  0b01111100,
  0b00101000,
  0b00010000,
  0b00010000,
};

// Connection icon bitmap (7x7)
const uint8_t connectionIcon[] = {
  0b00010000,
  0b00111000,
  0b01111100,
  0b00010000,
  0b01111100,
  0b00111000,
  0b00010000,
};




//Helper functions

void drawProgressBar(int percentage) {

  // Draw a thin bar at the bottom of the screen
  int barX = 10;                     // Left margin for the bar
  int barY = SCREEN_HEIGHT - 12;     // Position the bar 12 pixels from the bottom
  int barWidth = SCREEN_WIDTH - 20;  // Reduce width for left and right margins
  int barHeight = 8;                 // Total height of the progress bar, including background


  display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
  display.fillRect(barX + 1, barY + 1, barWidth - 2, barHeight - 2, SSD1306_BLACK);

  int progressWidth = ((barWidth - 2) * percentage) / 100;  // Progress width
  display.fillRect(barX + 1, barY + 1, progressWidth, barHeight - 2, SSD1306_WHITE);
}

//Alert with centered text and blinking screen
void ShowAlert(String Msg) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.invertDisplay(true);


  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(Msg, 0, 0, &x1, &y1, &textWidth, &textHeight);

  // Calculate the X and Y position to center the text
  int16_t centerX = (SCREEN_WIDTH - textWidth) / 2;
  int16_t centerY = (SCREEN_HEIGHT - textHeight) / 2;

  // Set the cursor position and print the text
  display.setCursor(centerX, centerY);
  display.print(Msg);

  display.display();
  delay(300);
  display.invertDisplay(false);
  delay(300);
  display.invertDisplay(true);
  delay(300);
  display.invertDisplay(false);
  delay(300);
}

//i can't draw, just show text
void ShowLogo() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.invertDisplay(true);
  display.setCursor(28, 20);
  display.print("ESPMON");
  display.display();
  delay(1500);
  display.invertDisplay(false);
}

//Screen dimming
void setBrightness(uint8_t brightness) {
  // Send contrast command to adjust brightness
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(brightness);
}

//Print and draw text
void ShowText(String Msg) {
  display.setTextSize(1);
  display.println(Msg);
  display.display();
}

void InitDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

//Smartphone like header with icons and neat dotted line as spacer
void DrawHeader(String Msg) {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(Msg);

  for (int x = 0; x < 128; x += 2) {
    display.drawPixel(x, 8, WHITE);
  }
  if (deviceSettings.WfifMode) display.drawBitmap(110, 0, wifiIcon, 7, 7, SSD1306_WHITE);

  if (ShowConnection) display.drawBitmap(100, 0, connectionIcon, 7, 7, SSD1306_WHITE);
}





void MainScreen() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(120, 0);
  display.print("/-\\|"[animationIndex % 4]);
  animationIndex++;

  //Draw all data when screen on
  if (ScreenBrightness == 1) {
    DrawHeader(F("Measure"));
    display.setTextSize(2);
    display.setCursor(0, 3 + ofs);
    display.print(F("V:"));
    display.print(voltage_V, 2);

    display.setCursor(115, 3 + ofs);
    display.println(F("V"));

    display.setCursor(0, 20 + ofs);
    display.print(F("I:"));
    display.print(current_A, 2);

    display.setCursor(115, 20 + ofs);
    display.println(F("A"));

    display.setCursor(0, 37 + ofs);
    display.print(F("W:"));
    display.print(power_W, 1);

    display.setCursor(115, 37 + ofs);
    display.println(F("W"));
  }
  //display.display();
}

//It can be done better
void PeaksScreen() {
  display.clearDisplay();
  DrawHeader(F("Peak values"));

  // Line 1: Voltage Peak High/Low
  display.setCursor(0, 0 + ofs);
  display.print(F("V H:"));
  display.print(voltage_peak_high, 2);
  display.print(F("V L:"));
  display.print(voltage_peak_low, 2);

  // Line 2: Current Peak High/Low
  display.setCursor(0, 10 + ofs);
  display.print(F("I H:"));
  display.print(current_peak_high, 2);
  display.print(F("A L:"));
  display.print(current_peak_low, 2);

  // Line 3: Power Peak High/Low
  display.setCursor(0, 20 + ofs);
  display.print(F("W H:"));
  display.print(power_peak_high, 2);
  display.print(F("W L:"));
  display.print(power_peak_low, 2);
  //display.display();
}

//Capacity screen with timer from last reset
void CapacityScreen() {
  display.clearDisplay();
  DrawHeader(F("Energy"));

  display.setTextSize(2);
  display.setCursor(0, 3 + ofs);
  display.print(F("Ah:"));
  display.println(capacity_Ah, 3);

  display.setCursor(0, 20 + ofs);
  display.print(F("Wh:"));
  display.println(capacity_Wh, 3);

  // Calculate elapsed time since last reset
  elapsedMillis = millis() - startMillis;

  // Convert elapsed time to h:m:s
  seconds = elapsedMillis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;

  seconds = seconds % 60;  // Get remaining seconds
  minutes = minutes % 60;  // Get remaining minutes

  // Format the time as a string
  sprintf(timeString, "%02lu:%02lu:%02lu", hours, minutes, seconds);

  display.setCursor(0, 37 + ofs);
  display.print(timeString);
  // display.display();
}

//Settings for shunt resistor
void ShuntScreen() {
  display.clearDisplay();
  DrawHeader(F("Shunt"));

  display.setTextSize(1);

  //Current current for "precise" setup
  display.setCursor(0, 3 + ofs);
  display.print(F("Current:"));
  display.print(current_A, 3);
  display.print(F(" A"));

  display.setCursor(0, 54);
  display.print(F("Back"));
  if (ListValue == 4) display.fillRect(0, 62, 24, 1, WHITE);

  //it was fonr size2 but with 200a Shunt it just can't fit on screen...
  //Arrows and value
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print(F("Value:"));

  display.setCursor(35, 25);
  display.print(F("<"));
  display.setCursor(52, 25);
  display.print(String(Shunt_Value, 6));
  display.setCursor(120, 25);
  display.print(F(">"));

  if (ListValue == 0) display.fillRect(35, 33, 7, 1, WHITE);
  if (ListValue == 1) display.fillRect(120, 33, 7, 1, WHITE);



  //shunt amps
  display.setCursor(0, 35);
  display.print(F("Max current:"));
  display.setCursor(71, 35);
  display.print(F("<"));
  display.setCursor(90, 35);
  display.print(String(Shunt_Max_Current, 0));
  display.setCursor(120, 35);
  display.print(F(">"));

  if (ListValue == 2) display.fillRect(71, 43, 7, 1, WHITE);
  if (ListValue == 3) display.fillRect(120, 43, 7, 1, WHITE);



  //display.display();
}

//Wifi settings screen
void WifiScreen() {
  display.clearDisplay();
  DrawHeader(F("Wireless"));

  display.setCursor(0, 0 + ofs);

  if (ListValue == 1) display.print(F("> "));
  display.print(F("Wifi: "));

  if (deviceSettings.WfifMode) {
    display.println("Enabled");
  } else {
    display.println(F("Disabled"));
  }

  display.setCursor(0, 10 + ofs);
  if (ListValue == 2) display.print(F("> "));
  display.print(F("Reset wifi"));

  display.setCursor(0, 20 + ofs);
  if (ListValue == 3) display.print(F("> "));
  display.print(F("Back"));

  if (deviceSettings.WfifMode) {

    display.setCursor(0, 35 + ofs);
    display.print(F("SSID: "));
    display.println(WiFi.SSID());

    display.setCursor(0, 45 + ofs);
    display.print(F("IP: "));
    display.println(WiFi.localIP());

    //Can't fit, better do as icon but i too lazy to draw wifi icons;
    /* display.setCursor(0, 55 + ofs);
    display.print(F("Signal: "));
    display.print(WiFi.RSSI()); */
  }
  //display.display();
}

//Settings menu screen
void SettingsScreen() {
  display.clearDisplay();
  DrawHeader(F("Settings"));

  display.setCursor(0, 0 + ofs);
  if (ListValue == 1) display.print(F("> "));
  display.print(F("Sampling"));
  display.setCursor(0, 10 + ofs);
  if (ListValue == 2) display.print(F("> "));
  display.print(F("Shunt"));
  display.setCursor(0, 20 + ofs);
  if (ListValue == 3) display.print(F("> "));
  display.print(F("Wifi"));
  display.setCursor(0, 30 + ofs);

  //just enable it always, too cheap to make whole settings system for it
  /*   if (ListValue == 4) display.print(F("> "));
  display.print(F("Serial")); */

  display.setCursor(0, 40 + ofs);
  if (ListValue == 4) display.print(F("> "));
  display.print(F("Exit"));

  //display.display();
}

//Settings page for ina226 sampling
void SamplingScreen() {
  display.clearDisplay();
  DrawHeader(F("Sampling"));

  display.setCursor(0, 0 + ofs);
  if (ListValue == 1) display.print(F("> "));
  display.print(F("Averaging: x"));
  display.print(averages[deviceSettings.TimeAVG]);

  display.setCursor(1, 10 + ofs);
  if (ListValue == 2) display.print(F("> "));
  display.print(F("VBUS time:"));
  display.print(avgtimes[deviceSettings.VBusAVG]);
  display.print(" uS");

  display.setCursor(2, 20 + ofs);
  if (ListValue == 3) display.print(F("> "));
  display.print(F("VSHUNT time:"));
  display.print(avgtimes[deviceSettings.VShuntAVG]);
  display.print(" uS");

  display.setCursor(0, 30 + ofs);
  if (ListValue == 4) display.print(F("> "));
  display.print(F("Back"));

  display.setCursor(0, 45 + ofs);
  display.print(F("V:"));
  display.print(voltage_V, 4);

  display.print(F(" I:"));
  display.print(current_A, 4);

  //display.display();
}
