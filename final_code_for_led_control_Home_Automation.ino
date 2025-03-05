#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>

const char *ssid = "DCSL Lab";
const char *password = "DCSL@444";

// LED configuration
const int ledPin = 27;  // D27 pin for LED
bool ledState = LOW;

// Web server on port 80
WebServer server(80);

// HTML & CSS for the web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 LED Control</title>
  <style>
    html {font-family: Arial, Helvetica, sans-serif; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .button {background-color: #4CAF50; border: none; color: white; padding: 16px 40px;
             text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
    .button2 {background-color: #D11D53;}
  </style>
</head>
<body>
  <h2>ESP32 LED Control</h2>
  <p>LED State: <span id="state">%STATE%</span></p>
  <p>
    <a href="/on"><button class="button">ON</button></a>
    <a href="/off"><button class="button button2">OFF</button></a>
  </p>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  
  // Initialize LED pin
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Set up OTA
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
  ArduinoOTA.begin();

  // Set up web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/on", HTTP_GET, handleOn);
  server.on("/off", HTTP_GET, handleOff);
  server.onNotFound(handleNotFound);
  
  // Start web server
  server.begin();
  
  Serial.println("Web server started");
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("To control the LED, open a browser and navigate to:");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();
  
  // Handle web server clients
  server.handleClient();
}

// Web server route handlers
void handleRoot() {
  String html = FPSTR(index_html);
  html.replace("%STATE%", ledState ? "ON" : "OFF");
  server.send(200, "text/html", html);
}

void handleOn() {
  ledState = HIGH;
  digitalWrite(ledPin, ledState);
  Serial.println("LED turned ON");
  redirectHome();
}

void handleOff() {
  ledState = LOW;
  digitalWrite(ledPin, ledState);
  Serial.println("LED turned OFF");
  redirectHome();
}

void redirectHome() {
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}