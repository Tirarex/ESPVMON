
//INA226
INA226 ina(0.0015f, 50.0f);

// Data Variables
float current_A = 0.0, voltage_V = 0.0, power_W = 0.0, shunt_voltage_mV = 0.0;

float voltage_peak_high = 0;
float voltage_peak_low = 0;

float current_peak_high = -999;
float current_peak_low = 999;

float power_peak_high = 0;
float power_peak_low = -1;

float capacity_Ah = 0.0, capacity_Wh = 0.0;
unsigned long elapsedMillis, seconds, minutes, hours;
unsigned long startMillis = 0;

void FetchIna() {
  voltage_V = ina.getVoltage();                     // Voltage (V)
  current_A = ina.getCurrent();                     // Current (A)
  power_W = ina.getPower();                         // Power (W)
  shunt_voltage_mV = ina.getShuntVoltage() * 1000;  // Shunt voltage (mV)

  if (voltage_V > voltage_peak_high) voltage_peak_high = voltage_V;
  if (current_A > current_peak_high) current_peak_high = current_A;
  if (power_W > power_peak_high) power_peak_high = power_W;

  if (voltage_V < voltage_peak_low) voltage_peak_low = voltage_V;
  if (current_A < current_peak_low) current_peak_low = current_A;
  if (power_W < power_peak_low) power_peak_low = power_W;
}

void calculateCapacity() {                            //Get last data
  float deltaT = 1.0;                                 // Time step in seconds
  capacity_Ah += (abs(current_A) * deltaT) / 3600.0;  // Update capacity in Ah
  capacity_Wh += (abs(power_W) * deltaT) / 3600.0;    // Update capacity in Wh
}

void FlushCapacity() {
  capacity_Ah = 0;
  capacity_Wh = 0;
  startMillis = millis();
}

void FlushPeakData() {
  voltage_peak_high = 0;
  voltage_peak_low = 0;

  current_peak_high = -999;
  current_peak_low = 999;

  power_peak_high = 0;
  power_peak_low = -1;
}

