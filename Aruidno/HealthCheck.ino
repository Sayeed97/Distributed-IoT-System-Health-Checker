#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <string>
#include <cstring>

// Thresholds for health checking strategy
#define MIN_WORKING_VOLTAGE_VALUE 2.70
#define MAX_HEAP_MEM_USAGE_PERCENTAGE 70.00

#define BUILTIN_LED 2

// WiFi credentials
const char* ssid = "Horizon_404_Mesh";
const char* password = "njit-ss4463-jb768";

// Enabling ADC to read ESP8266 real-time voltage value
ADC_MODE(ADC_VCC);

// Initializing the server to port 80
ESP8266WebServer server(80);

// Intanciating a task to toggle LED
Ticker ledToggleTask;

void toggleLed() {
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
}

// Checking for device supply voltage, heap memory used in % and is the Led toggle task active
String isDeviceHealthy() {
  // JSON conversion related declarations
  DynamicJsonDocument deviceHealthMetricsJson(512);
  String stringBuffer;

  // Read all the important device information for the health check
  float deviceVcc = (ESP.getVcc()/1000.00);
  float deviceHeapMemUsed = (ESP.getFreeHeap()/81920.00)*100;
  bool activeLedToggleTask = ledToggleTask.active();

  // JSON creation
  deviceHealthMetricsJson["Healthy"] = ((deviceVcc > MIN_WORKING_VOLTAGE_VALUE ? true : false) &&
  ((deviceHeapMemUsed <= MAX_HEAP_MEM_USAGE_PERCENTAGE ? true : false) &&
  activeLedToggleTask)) ? std::string("true") : std::string("false");
  deviceHealthMetricsJson["LED Toggle Task Active"] = activeLedToggleTask ? std::string("true") : std::string("false");
  deviceHealthMetricsJson["Heap Memory Used"] = deviceHeapMemUsed;
  deviceHealthMetricsJson["Working Voltage"] = deviceVcc;
  serializeJson(deviceHealthMetricsJson, stringBuffer);

  return stringBuffer;
}

// Server response for a health check
void handleRoot() {
  server.send(200, "application/json", isDeviceHealthy());
}

// Unknown URI request definition for the Server
void handleNotFound() {
  String message = "404: Unknown Request\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  // Waiting for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/healthy", HTTP_GET, handleRoot);
  /* 
  Enabling CORS to prevent the browser from receiving opaque response
  For opaque responses refer: https://tpiros.dev/blog/what-is-an-opaque-response/
  For CORS refer: https://stackoverflow.com/questions/51161837/cors-issue-in-nodemcu-esp8266
  */
  server.enableCORS(true);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // Initializing LED toggle GPIO pin
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  // Initializing a timer to toggle Built-in LED every 1 minute
  ledToggleTask.attach(1, toggleLed);
}

void loop(void) {
  // Server handler instance
  server.handleClient();
}