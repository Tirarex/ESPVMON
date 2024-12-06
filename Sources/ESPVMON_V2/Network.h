

//Web server and update
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

char response[650];

void InitServer() {
  server.on("/metrics", []() {
    int length = snprintf(response, sizeof(response),
                          "# HELP esp8266_voltage Voltage in volts\n"
                          "# TYPE esp8266_voltage gauge\n"
                          "esp8266_voltage %.3f\n"
                          "# HELP esp8266_current Current in amps\n"
                          "# TYPE esp8266_current gauge\n"
                          "esp8266_current %.3f\n"
                          "# HELP esp8266_power Power in watts\n"
                          "# TYPE esp8266_power gauge\n"
                          "esp8266_power %.3f\n"
                          "# HELP esp8266_shunt_voltage Shunt voltage in millivolts\n"
                          "# TYPE esp8266_shunt_voltage gauge\n"
                          "esp8266_shunt_voltage %.3f\n"
                          "# HELP esp8266_capacity_ah Accumulated capacity in ampere-hours (Ah).\n"
                          "# TYPE esp8266_capacity_ah gauge\n"
                          "esp8266_capacity_ah %.6f\n"
                          "# HELP esp8266_capacity_wh Accumulated capacity in watt-hours (Wh).\n"
                          "# TYPE esp8266_capacity_wh gauge\n"
                          "esp8266_capacity_wh %.6f\n",
                          voltage_V, current_A, power_W, shunt_voltage_mV, capacity_Ah, capacity_Wh);
    ShowConnection = true;
    server.send(200, "text/plain; charset=utf-8", response);
  });

  server.on("/clear", []() {
    capacity_Ah = 0.0;
    capacity_Wh = 0.0;
    server.send(200, "text/plain; charset=utf-8", "Capacity data cleared.");
    Serial.println(F("Capacity data cleared."));
  });

  server.begin();

  //Update Server
  httpUpdater.setup(&server);
}