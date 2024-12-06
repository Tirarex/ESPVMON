
//OLED Screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int ScreenPage = 0;

void InitDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

char timeString[16];
int animationIndex = 0;
int ScreenBrightness = 1;
bool ShowConnection = false;
void drawProgressBar(int progress) {
  // Draw background
  display.fillRect(0, 32, 128, 32, BLACK);
  // Draw border
  display.drawRect(2, 34, 124, 28, WHITE);
  // Draw progress bar
  int barWidth = (progress * 120) / 100;  // Map progress (0-100) to bar width (120px inside the border)
  display.fillRect(4, 36, barWidth, 24, WHITE);
  // Update display
  display.display();
}

// Arrow drawing
void drawArrow(int x, int y, bool up) {
  if (up) {
    display.drawLine(x, y, x + 2, y - 2, SSD1306_WHITE);
    display.drawLine(x, y, x - 2, y - 2, SSD1306_WHITE);
  } else {
    display.drawLine(x, y, x + 2, y + 2, SSD1306_WHITE);
    display.drawLine(x, y, x - 2, y + 2, SSD1306_WHITE);
  }
}

int ofs = 10;

// WiFi icon bitmap (7x7)
const uint8_t wifiIcon[] = {
  0b00101000,  //   XXX
  0b00101000,  //  XXXXX
  0b01111100,  //  XXXXX
  0b01111100,  //  XXXXX
  0b00101000,  //   X X
  0b00010000,  //    X
  0b00010000,  //    X
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

void DrawHeader(String Msg) {

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(Msg);

  for (int x = 0; x < 128; x += 2) {
    display.drawPixel(x, 8, WHITE);
  }

  if (deviceSettings.WfifMode) display.drawBitmap(110, 0, wifiIcon, 7, 7, SSD1306_WHITE);

  if (ShowConnection) display.drawBitmap(100, 0, connectionIcon, 7, 7, SSD1306_WHITE);

  // display.drawLine(0, 8, display.width(), 8, SSD1306_WHITE);
}


void MainScreen() {


  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(120, 0);
  display.print("/-\\|"[animationIndex % 4]);
  animationIndex++;

  if (ScreenBrightness == 1) {
    DrawHeader(F("Measure"));
    display.setTextSize(2);
    display.setCursor(0, 3 + ofs);
    display.print(F("V:"));
    display.print(voltage_V, 3);
    display.println(" V");

    display.setCursor(0, 20 + ofs);
    display.print(F("I:"));
    display.print(current_A, 3);
    display.println(F(" A"));

    display.setCursor(0, 37 + ofs);
    display.print(F("W:"));
    display.print(power_W, 3);
    display.println(F(" W"));
  }
  display.display();
}

void PeaksScreen() {

  display.clearDisplay();
  DrawHeader(F("Peak values"));

  // Line 1: Voltage Peak High/Low
  display.setCursor(0, 0 + ofs);
  display.print(F("V H:"));
  display.print(voltage_peak_high, 2);
  display.print(F("V L:"));
  display.print(power_peak_low, 2);

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
  display.display();
}

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


  elapsedMillis = millis() - startMillis;  // Calculate elapsed time since last reset

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
  display.display();  // Вывод на экран
}

void WifiScreen() {


  display.clearDisplay();
  DrawHeader(F("Settings"));



  display.setCursor(0, 0 + ofs);
  display.print(F("Wifi: "));
  if (!deviceSettings.WfifMode) {
    display.println(F("Disabled"));

    display.setCursor(0, 10 + ofs);
    display.print(F("Heap:"));
    display.println(ESP.getFreeHeap());

  } else {
    display.println("Enabled");

    display.setCursor(0, 10 + ofs);
    display.print(F("SSID: "));
    display.println(WiFi.SSID());

    display.setCursor(0, 20 + ofs);
    display.print(F("IP: "));
    display.println(WiFi.localIP());

    display.setCursor(0, 30 + ofs);
    display.print(F("Signal: "));
    display.print(WiFi.RSSI());

    display.setCursor(0, 40 + ofs);
    display.print(F("Heap:"));
    display.println(ESP.getFreeHeap());
  }
  display.display();
}

void ShowAlert(String Msg, int time) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.invertDisplay(true);
  display.print(Msg);
  display.display();
  delay(time);
  display.invertDisplay(false);
}

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

void ShowLogo() {

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.invertDisplay(true);
  display.setCursor(28, 20);
  display.print("ESPMON");
  display.display();
  delay(2000);
  display.invertDisplay(false);
}

void setBrightness(uint8_t brightness) {
  // Send contrast command to adjust brightness
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(brightness);
}

void ShowText(String Msg) {
  display.setTextSize(1);
  display.println(Msg);
  display.display();
}
