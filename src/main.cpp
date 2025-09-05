#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <utility>
#include "DHT20.h"

#define PHOTO_RES 37
#define MAX_LIGHT 255
#define MOISTURE_SENSOR 36
#define WHITE_PIN1 25
#define WHITE_PIN2 26
#define WHITE_PIN3 32
#define WHITE_PIN4 33
#define LEFT_BUTTON 0
#define RIGHT_BUTTON 35

#define CHANNEL 0
#define FREQUENCY 5000
#define RESOLUTION 8

#define FF18 &Roboto_Thin_24
TFT_eSPI tft = TFT_eSPI();
DHT20 dht;

int humTarget;
int tempTarget;
int lightTarget; 
int mosTarget;

int currHum;
int currTemp;
int currLight; 
int currMos;

int LEDvalue = 0;

int startHour = 8; 
int startMin = 30;
int endHour = 20; 
int endMin = 15;
bool dayCycle = false;

const int RBUTTON_DELAY = 5000;
const int DEFAULT_DELAY = 1000;
bool rbuttonPressed = false;
int defaultTimer = 0;
int rbuttonTimer = 0;

// Syncing time via ntp server
const char* ntpServer = "pool.ntp.org";
const long offsetToPST = -28800;
const int daylightSavings = 3600;

// Wi-Fi credentials
#define WIFI_SSID "" // NOTE: Please delete value before submitting
#define WIFI_PASSWORD "" // NOTE: Please delete value before submitting

// Azure IoT Hub configuration
#define SAS_TOKEN "SharedAccessSignature sr=group12iothub.azure-devices.net%2Fdevices%2F147esp32&sig=SLyBhvzra0%2FGfw6wrnk03wwspR1r7NfNJRzJV5ugXNM%3D&se=1757615942"
// Root CA certificate for Azure IoT Hub
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n" \
"MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n" \
"UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n" \
"eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n" \
"GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n" \
"mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n" \
"SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n" \
"dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n" \
"AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n" \
"VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n" \
"buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n" \
"AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n" \
"b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n" \
"Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n" \
"oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n" \
"dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n" \
"AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n" \
"klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n" \
"kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n" \
"eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n" \
"penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n" \
"QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n" \
"q/xKzj3O9hFh/g==\n" \
"-----END CERTIFICATE-----\n";

String iothubName = "group12iothub";
String deviceName = "147esp32";
String url = "https://" + iothubName + ".azure-devices.net/devices/" +
deviceName + "/messages/events?api-version=2021-04-12";

// Telemetry interval
#define TELEMETRY_INTERVAL 10000 // Send data every 3 seconds
uint32_t lastTelemetryTime = 0;

// put function declarations here:
void displayState(String, String, String, String);
template <typename T>
String compareCondition(T, T);
template <typename T>
void setLED(T, T);
void setDayCycle();
void setHrMin(int&, int&);
bool activate();

void setup() {
  // Initiate wifi connection
  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  configTime(offsetToPST, daylightSavings, ntpServer);

  Serial.begin(9600);
  Wire.begin(21, 22);
  dht.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  // Connect LED pins to the channel
  ledcSetup(CHANNEL, FREQUENCY, RESOLUTION);

  ledcAttachPin(WHITE_PIN1, CHANNEL);
  ledcAttachPin(WHITE_PIN2, CHANNEL);
  ledcAttachPin(WHITE_PIN3, CHANNEL);
  ledcAttachPin(WHITE_PIN4, CHANNEL);

  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);

  pinMode(MOISTURE_SENSOR, INPUT);

  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillRectHGradient(0, 0, tft.width(), tft.height(), TFT_DARKCYAN, TFT_DARKGREEN);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FF18);

  // Receive plant information from wifi server
  // TEMP VALUES
  humTarget = 65;
  tempTarget = 27;
  lightTarget = 500;
  mosTarget = 2550;
}

void loop() {
  // Read current status
  dht.read();
  currHum = dht.getHumidity();
  currTemp = dht.getTemperature();
  currLight = analogRead(PHOTO_RES);
  currMos = analogRead(MOISTURE_SENSOR);

  // Send plant data to cloud every interval
  if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    // Create JSON payload
    ArduinoJson::JsonDocument doc;
    doc["humidity"] = currHum;
    doc["temperature"] = currTemp;
    doc["light level"] = currLight;
    doc["moisture"] = currMos;
    char buffer[256];
    serializeJson(doc, buffer, sizeof(buffer));

    // Send telemetry via HTTPS
    WiFiClientSecure client;
    client.setCACert(root_ca); // Set root CA certificate
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", SAS_TOKEN);
    int httpCode = http.POST(buffer);
    
    if (httpCode == 204) { // IoT Hub returns 204 No Content for successful telemetry
      Serial.println("Telemetry sent: " + String(buffer));
    } else {
      Serial.println("Failed to send telemetry. HTTP code: " + String(httpCode));
    }

    http.end();
    lastTelemetryTime = millis();
  }

  // Adjust LED lights and display plant status on t-display
  dayCycle = activate();
  if (!dayCycle) ledcWrite(CHANNEL, 0);
  else setLED(currLight, lightTarget);

  // Show status conditions when right button is pressed
  if (digitalRead(RIGHT_BUTTON) == LOW) {
    rbuttonPressed = true;
    rbuttonTimer = millis() + RBUTTON_DELAY;
  }

  // Display condition status when right button is pressed
  if (rbuttonPressed) {
    String hum = compareCondition(currHum, humTarget);
    String temp = compareCondition(currTemp, tempTarget);
    String light = compareCondition(currLight, lightTarget);
    String mos = compareCondition(currMos, mosTarget);

    displayState(String(hum), String(temp),
                 String(light), String(mos));
    
    if (millis() > rbuttonTimer) rbuttonPressed = false;
  }

  // Display condition data by default
  else if (millis() > defaultTimer) {
    displayState(String(currHum), String(currTemp),
               String(currLight), String(currMos));
    defaultTimer = millis() + DEFAULT_DELAY;
  }

  // Set day-night cycle timer for LEDs when left button is pressed
  if (digitalRead(LEFT_BUTTON) == LOW) {
    setDayCycle();
  }
}

// put function definitions here:
void displayState(String hum, String temp, 
                  String light, String mos) {
  tft.fillRectHGradient(0, 0, tft.width(), tft.height(), TFT_DARKCYAN, TFT_DARKGREEN);
  tft.setCursor(0, tft.fontHeight());

  tft.print("Humidity: "); tft.println(hum);
  tft.print("Temperature: "); tft.println(temp);
  if (dayCycle) {
    tft.print("Light Level: "); tft.println(light);
  }
  tft.print("Moisture: "); tft.println(mos);
}

template <typename T>
String compareCondition(T curr, T ideal) {
  double percentDiff = abs(curr - ideal) / 
                       ((curr + ideal) / 2.0);
  if (percentDiff < 0.05) {
    return "Good";
  }
  else if (curr > ideal) {
    return "Too high";
  }
  else return "Too low";
}

template <typename T>
void setLED(T curr, T ideal) {
  // Incrementally raise or lower light level to match ideal
  double percentDiff = abs(curr - ideal) / 
                       ((curr + ideal) / 2.0);
  if (percentDiff < 0.05) return;
  else if (curr < ideal) ++LEDvalue;
  else if (curr > ideal) --LEDvalue;
  constrain(LEDvalue, 0, MAX_LIGHT);
  ledcWrite(CHANNEL, LEDvalue);
}

bool activate() {
  struct tm currTime;

  if (!getLocalTime(&currTime)) {
    Serial.println("Failed to obtain time");
    return dayCycle;
  }

  int hr = currTime.tm_hour;
  int min = currTime.tm_min;

  // Manually check whether current time is within day cycle hours
  if (startHour == endHour) {
    if (startMin == endMin) return false;
    if (startMin < endMin) {
      if (hr == startHour && min >= startMin && min < endMin) return true;
    }
    else {
      if (hr > startHour || hr < endHour || min >= startMin || min < endMin)
          return true;
    }
  }
  else if (startHour < endHour) {
    if (startHour <= hr && hr < endHour && min >= startMin) return true;
    else if (hr == endHour && min < endMin) return true;
  }
  else if (startHour > endHour) {
    if ((hr >= startHour && min >= startMin) || hr < endHour) return true;
    else if (hr == endHour && min < endMin) return true;
  }
  return false;
}

void setDayCycle() {
  tft.fillRectHGradient(0, 0, tft.width(), tft.height(), TFT_DARKCYAN, TFT_DARKGREEN);
  tft.setCursor(0, tft.fontHeight());
  tft.print("LED activation time:"); delay(1000);
  setHrMin(startHour, startMin);

  tft.fillRectHGradient(0, 0, tft.width(), tft.height(), TFT_DARKCYAN, TFT_DARKGREEN);  
  tft.setCursor(0, tft.fontHeight());
  tft.print("LED deactivation time:"); delay(1000);
  setHrMin(endHour, endMin);
}

void setHrMin(int &hr, int &min) {
  // Until right button is pressed, each left button press will increment time
  while (digitalRead(RIGHT_BUTTON) != LOW) {
    if (digitalRead(LEFT_BUTTON) == LOW) {
      if (hr < 23) ++hr;
      else hr = 0;
    }
    // Display new time
    tft.fillRectHGradient(0, 0, tft.width(), tft.height(), TFT_DARKCYAN, TFT_DARKGREEN);
    tft.setCursor(0, tft.fontHeight());
    tft.print(hr); tft.print(":"); tft.print(min);
    delay(200);
  }

  delay(1000);

  while (digitalRead(RIGHT_BUTTON) != LOW) {
    if (digitalRead(LEFT_BUTTON) == LOW) {
      if (min < 59) ++min;
      else min = 0;
    }
    tft.fillRectHGradient(0, 0, tft.width(), tft.height(), TFT_DARKCYAN, TFT_DARKGREEN);
    tft.setCursor(0, tft.fontHeight());
    tft.print(hr); tft.print(":"); tft.print(min);
    delay(200);
  }
}