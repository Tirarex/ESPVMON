
//Buttons
const int button1Pin = 13;                     // Button 1 connected to D12
const int button2Pin = 12;                     // Button 2 connected to D13
bool button1State = false;                     // Current state of button 1
bool button2State = false;                     // Current state of button 2
unsigned long lastDebounceTime1 = 0;           // Debounce timer for button 1
unsigned long lastDebounceTime2 = 0;           // Debounce timer for button 2
const unsigned long debounceDelay = 5;         // 50ms debounce delay
unsigned long button1PressTime = 0;            // Time when button 1 was pressed
unsigned long button2PressTime = 0;            // Time when button 2 was pressed
const unsigned long longPressDuration = 2000;  // 2 seconds for long press

bool B1Click = false;
bool B2Click = false;
bool B1ClickLong = false;
bool B2ClickLong = false;

void InitButtons(){
   pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);

}

void HandleButtons() {
  // Read the buttons and handle debounce
  bool reading1 = digitalRead(button1Pin);
  bool reading2 = digitalRead(button2Pin);

  // Handle button 1
  if (reading1 != button1State && millis() - lastDebounceTime1 > debounceDelay) {
    lastDebounceTime1 = millis();  // Reset debounce timer
    button1State = reading1;
    if (button1State == HIGH) {  // Button 1 pressed
      button1PressTime = millis();
    } else {  // Button 1 released
      if (millis() - button1PressTime >= longPressDuration) {
        //Serial.println("Button 1 Long Press");
        B1ClickLong = true;
      } else {
        //Serial.println("Button 1 Short Press");
        B1Click = true;
      }
    }

   
  }



  // Handle button 2
  if (reading2 != button2State && millis() - lastDebounceTime2 > debounceDelay) {
    lastDebounceTime2 = millis();  // Reset debounce timer
    button2State = reading2;
    if (button2State == HIGH) {  // Button 2 pressed
      button2PressTime = millis();
    } else {  // Button 2 released
      if (millis() - button2PressTime >= longPressDuration) {
        //Serial.println("Button 2 Long Press");
        B2ClickLong = true;
      } else {
        //Serial.println("Button 2 Short Press");
        B2Click = true;
      }
    }
  }
}

void FlushButtons() {
  B1Click = false;
  B2Click = false;
  B1ClickLong = false;
  B2ClickLong = false;
}
