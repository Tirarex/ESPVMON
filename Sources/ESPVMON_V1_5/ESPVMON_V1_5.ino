#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <GyverINA.h>
#include <ESP8266HTTPUpdateServer.h>

// WiFi Credentials
const char* ssid = "";
const char* password = "";

INA226 ina(0.0015f, 50.0f);

#define debug


// Web Server
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// Data Variables
float current_A = 0.0, voltage_V = 0.0, power_W = 0.0, shunt_voltage_mV = 0.0;
float capacity_Ah = 0.0;
float capacity_Wh = 0.0;  
unsigned long lastUpdateTime = 0;  

void setup() {

#ifdef debug
  Serial.begin(115200);
  Serial.print("Connecting to WiFi");
#endif


  // Initialize WiFi
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);

#ifdef debug
    Serial.print(".");
#endif
  }

#ifdef debug
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
#endif

  // Initialize INA226
  Serial.print(F("INA226..."));
  if (ina.begin()) {

#ifdef debug
    Serial.println(F("connected!"));
#endif
  } else {

#ifdef debug
    Serial.println(F("not found!"));
#endif
    while (1)
      ;
  }

  // Configure INA226
  ina.setSampleTime(INA226_VBUS, INA226_CONV_8244US);   
  ina.setSampleTime(INA226_VSHUNT, INA226_CONV_8244US);  
  ina.setAveraging(INA226_AVG_X128);                    

#ifdef debug
  Serial.print(F("Calibration value: "));
  Serial.println(ina.getCalibration());
#endif

  // Define Prometheus API endpoint
  server.on("/metrics", []() {
    // Read INA226 sensor data
    FetchIna();

    String response = "";
    response += "# HELP esp8266_voltage Voltage in volts\n";
    response += "# TYPE esp8266_voltage gauge\n";
    response += "esp8266_voltage " + String(voltage_V, 3) + "\n";

    response += "# HELP esp8266_current Current in amps\n";
    response += "# TYPE esp8266_current gauge\n";
    response += "esp8266_current " + String(current_A, 3) + "\n";

    response += "# HELP esp8266_power Power in watts\n";
    response += "# TYPE esp8266_power gauge\n";
    response += "esp8266_power " + String(power_W, 3) + "\n";

    response += "# HELP esp8266_shunt_voltage Shunt voltage in millivolts\n";
    response += "# TYPE esp8266_shunt_voltage gauge\n";
    response += "esp8266_shunt_voltage " + String(shunt_voltage_mV, 3) + "\n";

    response += "# HELP esp8266_capacity_ah Accumulated capacity in ampere-hours (Ah).\n";
    response += "# TYPE esp8266_capacity_ah gauge\n";
    response += "esp8266_capacity_ah " + String(capacity_Ah, 6) + "\n";

    response += "# HELP esp8266_capacity_wh Accumulated capacity in watt-hours (Wh).\n";
    response += "# TYPE esp8266_capacity_wh gauge\n";
    response += "esp8266_capacity_wh " + String(capacity_Wh, 6) + "\n";

    server.send(200, "text/plain; charset=utf-8", response);
  });

  server.on("/clear", []() {
    capacity_Ah = 0.0;
    capacity_Wh = 0.0;
    server.send(200, "text/plain; charset=utf-8", "Capacity data cleared.");
    Serial.println("Capacity data cleared.");
  });


 httpUpdater.setup(&server);
  server.begin();

#ifdef debug
  Serial.println("Server started");
#endif
}

void FetchIna() {
  voltage_V = ina.getVoltage();                     // Voltage (V)
  current_A = ina.getCurrent();                     // Current (A)
  power_W = ina.getPower();                         // Power (W)
  shunt_voltage_mV = ina.getShuntVoltage() * 1000;  // Shunt voltage (mV)
}

void calculateCapacity() {
  FetchIna();                                    //Get last data
  float deltaT = 1.0;                            // Time step in seconds
  capacity_Ah += (abs(current_A) * deltaT) / 3600.0;  // Update capacity in Ah
  capacity_Wh += (abs(power_W) * deltaT) / 3600.0;    // Update capacity in Wh
}

void loop() {

  if (millis() - lastUpdateTime >= 1000) {
    calculateCapacity();
    lastUpdateTime = millis();
  }


#ifdef debug
  // Debug output to Serial
  Serial.print(F("Voltage: "));
  Serial.print(voltage_V, 3);
  Serial.println(F(" V"));
  Serial.print(F("Current: "));
  Serial.print(current_A, 3);
  Serial.println(F(" A"));
  Serial.print(F("Power: "));
  Serial.print(power_W, 3);
  Serial.println(F(" W"));
  Serial.print(F("Shunt voltage: "));
  Serial.print(shunt_voltage_mV, 3);
  Serial.println(F(" mV"));
  Serial.println();
#endif

  // Handle web server
  server.handleClient();
  delay(100);  // Data update interval (100ms)
}
