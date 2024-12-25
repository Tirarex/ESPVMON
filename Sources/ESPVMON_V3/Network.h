//Web server and update server
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

char PrometheusResponse[700];  //uh, it has to be that huge to send all data when power is over 1000W

void InitServer() {
  //Prometheus Api
  server.on("/metrics", []() {
    int length = snprintf(PrometheusResponse, sizeof(PrometheusResponse),
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
    server.send(200, "text/plain; charset=utf-8", PrometheusResponse);
  });

  //Json API
  server.on("/json", HTTP_GET, []() {
    // Use minimal JSON buffer size
    StaticJsonDocument<200> jsonDoc;

    jsonDoc["voltage_V"] = voltage_V;
    jsonDoc["current_A"] = current_A;
    jsonDoc["power_W"] = power_W;
    jsonDoc["shunt_voltage_mV"] = shunt_voltage_mV;
    jsonDoc["capacity_Ah"] = capacity_Ah;
    jsonDoc["capacity_Wh"] = capacity_Wh;

    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);
    ShowConnection = true;
    // Send JSON response
    server.send(200, F("application/json"), jsonResponse);
  });

  server.on("/", []() {
    int rssi = WiFi.RSSI();            // Signal strength
    int freeHeap = ESP.getFreeHeap();  // Free memory

    String html = F("<html><head><title>ESP8266 Metrics</title></head><body>");
    html += F("<h1>ESP8266 Sensor Data</h1>");
    html += F("<table border='1'><tr><th>Parameter</th><th>Value</th></tr>");
    html += "<tr><td>Voltage (V)</td><td>" + String(voltage_V) + "</td></tr>";
    html += "<tr><td>Current (A)</td><td>" + String(current_A) + "</td></tr>";
    html += "<tr><td>Power (W)</td><td>" + String(power_W) + "</td></tr>";
    html += "<tr><td>Shunt Voltage (uV)</td><td>" + String(shunt_voltage_mV) + "</td></tr>";
    html += "<tr><td>Capacity (Ah)</td><td>" + String(capacity_Ah) + "</td></tr>";
    html += "<tr><td>Capacity (Wh)</td><td>" + String(capacity_Wh) + "</td></tr>";
    html += "<tr><td>WiFi Signal Strength (RSSI)</td><td>" + String(rssi) + " dBm</td></tr>";
    html += "<tr><td>Free Heap Memory</td><td>" + String(freeHeap) + " bytes</td></tr>";
    html += F("</table>");
    html += F("<p><a href='/metrics'>Metrics</a></p>");
    html += F("<p><a href='/json'>JSON</a></p>");
    html += F("<p><a href='/update'>Update</a></p>");
    html += F("<p><a href='/clear'>Clear</a></p>");
    html += F("</body></html>");
    ShowConnection = true;
    server.send(200, "text/html", html);
  });
  ;

  //Clear capacuty
  server.on("/clear", []() {
    capacity_Ah = 0.0;
    capacity_Wh = 0.0;
    ShowConnection = true;
    server.send(200, "text/plain; charset=utf-8", "Capacity data cleared.");
  });

  server.begin();

  //Update Server
  httpUpdater.setup(&server);
}

//Ulta simple Serial api, just to get data in some non wireless device
void printSerialData() {
  // Print all values in one line, separated by `/`
  Serial.print(voltage_V);
  Serial.print('/');
  Serial.print(current_A);
  Serial.print('/');
  Serial.print(power_W);
  Serial.print('/');
  Serial.print(shunt_voltage_mV);
  Serial.print('/');
  Serial.print(capacity_Ah);
  Serial.print('/');
  Serial.print(capacity_Wh);
}
