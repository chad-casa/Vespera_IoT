// Combined sketch: ENS160 sensor + NeoPixel control via MQTT

// --- Libraries ---
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <utility/wifi_drv.h>
#include <Wire.h>
#include "ScioSense_ENS160.h"
#include "arduino_secrets.h"

// WiFi credentials
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

// MQTT Configuration
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1883;
const char* mqtt_client_id = "MKR1010_NeoPixel_Luminaire_CHAD";
const char* mqtt_subscribe_topic = "student/CASA0014/luminaire/6";//subscribe to my topic - 6
const char* user_update_topic = "student/CASA0014/luminaire/user";
const char* brightness_update_topic = "student/CASA0014/luminaire/brightness";

int LUMINAIRE_USER = 0;
int LUMINAIRE_BRIGHTNESS = 150;

// NeoPixel Configuration
#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 72
#define NEOPIXEL_DATA_LENGTH (NEOPIXEL_COUNT * 3)

// Global Objects
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ENS160 sensor
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

// Timing variables
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 10000; // Read sensor every 10 second

// Mode selection
bool useLocalSensor = true; // Set to true to use ENS160, false to use MQTT only

// Function Prototypes
void setup_wifi();
void reconnect_mqtt();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void updateLEDsFromCO2(int co2_ppm);
void co2ToColor(int co2_ppm, byte &r, byte &g, byte &b);
void publishLEDsToMQTT(byte r, byte g, byte b);

// LED Control Functions
void LedRed() {
  WiFiDrv::digitalWrite(25, LOW);
  WiFiDrv::digitalWrite(26, HIGH);
  WiFiDrv::digitalWrite(27, LOW);
}

void LedGreen() {
  WiFiDrv::digitalWrite(25, HIGH);
  WiFiDrv::digitalWrite(26, LOW);
  WiFiDrv::digitalWrite(27, LOW);
}

void LedBlue() {
  WiFiDrv::digitalWrite(25, LOW);
  WiFiDrv::digitalWrite(26, LOW);
  WiFiDrv::digitalWrite(27, HIGH);
}

void setup() {
  Serial.begin(115200);
  
  // RGB LED's
  WiFiDrv::pinMode(25, OUTPUT);
  WiFiDrv::pinMode(26, OUTPUT);
  WiFiDrv::pinMode(27, OUTPUT);
  
  LedRed();
  
  Serial.println("------------------------------------------------------------");
  Serial.println("Starting MKR1010 NeoPixel + ENS160 Controller...");
  Serial.println("------------------------------------------------------------");
  
  // Initialize NeoPixels
  pixels.begin();
  pixels.show();
  pixels.setBrightness(LUMINAIRE_BRIGHTNESS);
  
  // Initialize ENS160 sensor
  if (useLocalSensor) {
    Serial.print("Initializing ENS160...");
    ens160.begin();
    if (ens160.available()) {
      Serial.println("done!");
      Serial.print("\tRev: "); Serial.print(ens160.getMajorRev());
      Serial.print("."); Serial.print(ens160.getMinorRev());
      Serial.print("."); Serial.println(ens160.getBuild());
      
      Serial.print("\tStandard mode ");
      Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");
    } else {
      Serial.println("failed!");
      useLocalSensor = false; // Fall back to MQTT mode
    }
  }
  
  // Set up MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqtt_callback);
  
  // Connect to Wi-Fi
  setup_wifi();
  
  Serial.println("Setup complete!");
  Serial.println(useLocalSensor ? "Mode: Local ENS160 sensor" : "Mode: MQTT only");
}

void loop() {
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    reconnect_mqtt();
  }
  mqttClient.loop();
  
  // Read sensor and update LEDs if using local sensor
  if (useLocalSensor && (millis() - lastSensorRead >= sensorInterval)) {
    lastSensorRead = millis();
    
    if (ens160.available()) {
      ens160.measure(true);
      
      int co2_ppm = ens160.geteCO2();
      int tvoc = ens160.getTVOC();
      int aqi = ens160.getAQI();
      
      // Print sensor readings
      Serial.print("AQI: "); Serial.print(aqi); Serial.print("\t");
      Serial.print("TVOC: "); Serial.print(tvoc); Serial.print("ppb\t");
      Serial.print("eCO2: "); Serial.print(co2_ppm); Serial.println("ppm");
      
      // Update LEDs based on CO2 reading
      updateLEDsFromCO2(co2_ppm);
    }
  }
}

// WiFi Setup Function
void setup_wifi() {
  delay(10);
  LedBlue();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  LedGreen();
}

// MQTT Reconnection Function
void reconnect_mqtt() {
  LedBlue();
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(mqtt_client_id)) {
      Serial.println("connected!");
      if (mqttClient.subscribe(mqtt_subscribe_topic)) {
        Serial.print("Subscribed to topic: ");
        Serial.println(mqtt_subscribe_topic);
      } else {
        Serial.println("Failed to subscribe to topic!");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
  LedGreen();
}

// MQTT Message Callback Function
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char payload_str[length + 1];
  memcpy(payload_str, payload, length);
  payload_str[length] = '\0';
  
  if (strcmp(topic, user_update_topic) == 0) {
    int new_user_id = atoi(payload_str);
    LUMINAIRE_USER = new_user_id;
    Serial.print("LUMINAIRE_USER updated to: ");
    Serial.println(LUMINAIRE_USER);
    
  } else if (strcmp(topic, brightness_update_topic) == 0) {
    int new_brightness_id = atoi(payload_str);
    LUMINAIRE_BRIGHTNESS = new_brightness_id;
    Serial.print("LUMINAIRE_BRIGHTNESS updated to: ");
    Serial.println(LUMINAIRE_BRIGHTNESS);
    pixels.setBrightness(LUMINAIRE_BRIGHTNESS);
    
  } else {
    char* last_slash = strrchr(topic, '/');
    if (last_slash != NULL) {
      char* number_str = last_slash + 1;
      int topic_user_id = atoi(number_str);
      
      if (topic_user_id == LUMINAIRE_USER) {
        if (length == NEOPIXEL_DATA_LENGTH) {
          // Disable local sensor mode when receiving MQTT commands
          useLocalSensor = false;
          
          for (int i = 0; i < NEOPIXEL_COUNT; i++) {
            byte r = payload[i * 3];
            byte g = payload[i * 3 + 1];
            byte b = payload[i * 3 + 2];
            pixels.setPixelColor(i, r, g, b);
          }
          pixels.show();
          Serial.println("LEDs updated from MQTT");
        }
      }
    }
  }
}

// Convert CO2 reading to RGB color
void co2ToColor(int co2_ppm, byte &r, byte &g, byte &b) {
  if (co2_ppm < 600) {
    // Good air quality - Green
    r = 0; g = 255; b = 0;
  } else if (co2_ppm < 1000) {
    // Moderate - Yellow/Green gradient
    float ratio = (co2_ppm - 600) / 400.0;
    r = 255 * ratio;
    g = 255;
    b = 0;
  } else if (co2_ppm < 1500) {
    // Poor - Orange
    float ratio = (co2_ppm - 1000) / 500.0;
    r = 255;
    g = 255 * (1 - ratio);
    b = 0;
  } else {
    // Very poor - Red
    r = 255; g = 0; b = 0;
  }
}

// Update all LEDs based on CO2 reading
void updateLEDsFromCO2(int co2_ppm) {
  byte r, g, b;
  co2ToColor(co2_ppm, r, g, b);
  
  // Set all LEDs to the same color
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, r, g, b);
  }
  pixels.show();
  
  // PUBLISH TO MQTT so website can see it
  publishLEDsToMQTT(r, g, b);
  
  Serial.print("LEDs updated: RGB(");
  Serial.print(r); Serial.print(",");
  Serial.print(g); Serial.print(",");
  Serial.print(b); Serial.println(")");
}

// Publish LED colors to MQTT so the website can see them
void publishLEDsToMQTT(byte r, byte g, byte b) {
  // Create payload for all 72 LEDs
  byte payload[NEOPIXEL_DATA_LENGTH];
  
  // Fill payload with same color for all LEDs
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    payload[i * 3] = r;       // Red
    payload[i * 3 + 1] = g;   // Green
    payload[i * 3 + 2] = b;   // Blue
  }
  
  // Publish to MQTT
  mqttClient.publish(mqtt_subscribe_topic, payload, NEOPIXEL_DATA_LENGTH);
  Serial.println(mqtt_subscribe_topic);
  //Serial.println(payload);
  Serial.println(NEOPIXEL_DATA_LENGTH);
  Serial.println("Published to MQTT");
}
