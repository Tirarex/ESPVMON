
//INA226
INA226 ina(0.0015f, 200.0f);

double Shunt_Value = 0.0015;
double Shunt_Max_Current = 50;
// Data Variables
float current_A = 0.0, voltage_V = 0.0, power_W = 0.0, shunt_voltage_mV = 0.0;

float voltage_peak_high = 0;
float voltage_peak_low = 9999;

float current_peak_high = -999;
float current_peak_low = 999;

float power_peak_high = 0;
float power_peak_low = 9999;

float capacity_Ah = 0.0, capacity_Wh = 0.0;
unsigned long elapsedMillis, seconds, minutes, hours;
unsigned long startMillis = 0;


void InitIna() {
  ina.begin(MaxCurrent, ShuntValue);
  ina.setAveraging(INA226_AVG_X64);
  ina.setSampleTime(INA226_VBUS, INA226_CONV_588US);
  ina.setSampleTime(INA226_VSHUNT, INA226_CONV_1100US);
  updateShunt(MaxCurrent, ShuntValue);
}

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

#ifndef MINIMALMODE
void SetInaSettings() {
  //Я прекрасно знаю что это можно просто перевести число в hex значение, но это для возможной совместимости с другими микросхемами
  switch (deviceSettings.TimeAVG) {
    case 0:
      ina.setAveraging(INA226_AVG_X1);
      break;
    case 1:
      ina.setAveraging(INA226_AVG_X4);
      break;
    case 2:
      ina.setAveraging(INA226_AVG_X16);
      break;
    case 3:
      ina.setAveraging(INA226_AVG_X64);
      break;
    case 4:
      ina.setAveraging(INA226_AVG_X128);
      break;
    case 5:
      ina.setAveraging(INA226_AVG_X256);
      break;
    case 6:
      ina.setAveraging(INA226_AVG_X512);
      break;
    case 7:
      ina.setAveraging(INA226_AVG_X1024);
      break;
    default: 
      ina.setAveraging(INA226_AVG_X1);
      break;
  }

  switch (deviceSettings.VBusAVG) {
    case 0:
      ina.setSampleTime(INA226_VBUS, INA226_CONV_140US);
      break;
    case 1:
      ina.setSampleTime(INA226_VBUS, INA226_CONV_204US);
      break;
    case 2:
      ina.setSampleTime(INA226_VBUS, INA226_CONV_332US);
      break;
    case 3:
      ina.setSampleTime(INA226_VBUS, INA226_CONV_588US);
      break;
    case 4:
      ina.setSampleTime(INA226_VBUS, INA226_CONV_1100US);
      break;
    case 5:
      ina.setSampleTime(INA226_VBUS, INA226_CONV_2116US);
      break;
    case 6:
      ina.setSampleTime(INA226_VBUS, INA226_CONV_4156US);
      break;
    case 7:
      ina.setSampleTime(INA226_VBUS, INA226_CONV_8244US);
      break;
    default: 
      ina.setSampleTime(INA226_VBUS, INA226_CONV_140US);
      break;
  }

  switch (deviceSettings.VShuntAVG) {
    case 0:
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_140US);
      break;
    case 1:
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_204US);
      break;
    case 2:
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_332US);
      break;
    case 3:
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_588US);
      break;
    case 4:
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_1100US);
      break;
    case 5:
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_2116US);
      break;
    case 6:
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_4156US);
      break;
    case 7:
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_8244US);
      break;
    default: 
      ina.setSampleTime(INA226_VSHUNT, INA226_CONV_140US);
      break;
  }
}
#endif

//Can be modified to calculate capacity in Both ways, charge/discharge, to make battery gauge like device
void calculateCapacity() {
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
  voltage_peak_low = 9999;

  current_peak_high = -999;
  current_peak_low = 999;

  power_peak_high = 0;
  power_peak_low = 9999;
}

//Direct code from gyverina lib
void updateShunt(float rShunt, float _i_max) {
  float _current_lsb = _i_max / 32768.0f;
  float _power_lsb = _current_lsb * 25.0f;
  uint16_t _cal_value = trunc(0.00512f / (_current_lsb * rShunt));
  ina.setCalibration(_cal_value);
}
