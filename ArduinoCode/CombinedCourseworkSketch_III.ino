// Duncan Wilson Oct 2025 - Combined: MQTT messager to vespera + ENS160 CO2 sensor

// works with MKR1010

#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h" 
#include <utility/wifi_drv.h>
#include <Wire.h>
#include "ScioSense_ENS160.h"

/*
**** please enter your sensitive data in the Secret tab/arduino_secrets.h
**** using format below
#define SECRET_SSID "ssid name"
#define SECRET_PASS "ssid password"
#define SECRET_MQTTUSER "user name - eg student"
#define SECRET_MQTTPASS "password";
 */
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = "mqtt.cetools.org";
const int mqtt_port       = 1884;

// create wifi object and mqtt object
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Make sure to update your lightid value below with the one you have been allocated
String lightId = "6"; // the topic id number or user number being used.

// Here we define the MQTT topic we will be publishing data to
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;            
String clientId = ""; // will set once i have mac address so that it is unique

// NeoPixel Configuration - we need to know this to know how to send messages 
// to vespera 
const int num_leds = 72;
const int payload_size = num_leds * 3; // x3 for RGB

// Create the byte array to send in MQTT payload this stores all the colours 
// in memory so that they can be accessed in for example the rainbow function
byte RGBpayload[payload_size];

// ENS160 sensor
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

// Timing variables for sensor
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 10000; // Read sensor every 10 seconds

// Mode control
bool useLocalSensor = true; // true = CO2 sensor controls colors, false = MQTT controls

// CO2 history tracking
int lastCO2_ppm = -1; // Track the last CO2 reading to detect changes
int currentLEDPosition = 0; // Tracks which set of 3 LEDs to update next (0, 3, 6, 9, etc.)

// Forward declarations
void startWifi();
void reconnectMQTT();
void callback(char* topic, byte* payload, unsigned int length);
void updateLEDsFromCO2(int co2_ppm);
void co2ToColor(int co2_ppm, byte &r, byte &g, byte &b);

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Vespera with CO2 Sensor - Timeline Mode");

  // Initialize RGB LED pins
  WiFiDrv::pinMode(25, OUTPUT); // Green LED
  WiFiDrv::pinMode(26, OUTPUT); // Red LED  
  WiFiDrv::pinMode(27, OUTPUT); // Blue LED

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);

  Serial.print("This device is Vespera ");
  Serial.println(lightId);

  // Initialize all LEDs to off
  for (int i = 0; i < payload_size; i++) {
    RGBpayload[i] = 0;
  }

  // Initialize ENS160 sensor
  Serial.print("Initializing ENS160 sensor...");
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

  // Connect to WiFi
  startWifi();

  // Connect to MQTT broker
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2000);
  mqttClient.setCallback(callback);
  
  Serial.println("Set-up complete");
  Serial.println(useLocalSensor ? "Mode: CO2 sensor controlling colors (Timeline)" : "Mode: MQTT only");
  Serial.println("Each reading will update the next 3 LEDs (24 readings max)");
}
 
void loop() {
  // Reconnect if necessary
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  
  // keep mqtt alive
  mqttClient.loop();

  // Read CO2 sensor and update LEDs if in local sensor mode
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
      
      // Only update LEDs if CO2 level has changed significantly (by more than 50 ppm)
      if (abs(co2_ppm - lastCO2_ppm) > 50 || lastCO2_ppm == -1) {
        updateLEDsFromCO2(co2_ppm);
        lastCO2_ppm = co2_ppm; // Remember this CO2 level
      } else {
        Serial.println("CO2 level unchanged - not adding to timeline");
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

// Update the next 3 LEDs based on CO2 reading (timeline mode)
void updateLEDsFromCO2(int co2_ppm) {
  byte r, g, b;
  co2ToColor(co2_ppm, r, g, b);
  
  // Update the next 3 LEDs in sequence
  for (int i = 0; i < 3; i++) {
    int ledIndex = currentLEDPosition + i;
    if (ledIndex < num_leds) { // Make sure we don't go past 72 LEDs
      RGBpayload[ledIndex * 3 + 0] = r; // Red
      RGBpayload[ledIndex * 3 + 1] = g; // Green
      RGBpayload[ledIndex * 3 + 2] = b; // Blue
    }
  }
  
  // Publish to MQTT
  if (mqttClient.connected()) {
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.print("Updated LEDs ");
    Serial.print(currentLEDPosition);
    Serial.print("-");
    Serial.print(currentLEDPosition + 2);
    Serial.print(" to RGB(");
    Serial.print(r); Serial.print(",");
    Serial.print(g); Serial.print(",");
    Serial.print(b); Serial.print(") - CO2: ");
    Serial.print(co2_ppm);
    Serial.println(" ppm");
  }
  
  // Move to the next set of 3 LEDs
  currentLEDPosition += 3;
  
  // If we've filled all 72 LEDs (24 readings), wrap back to the beginning
  if (currentLEDPosition >= num_leds) {
    currentLEDPosition = 0;
    Serial.println("*** LED strip full - wrapping back to start ***");
  }
}

// Function to update the R, G, B values of a single LED pixel
// RGB can a value between 0-254, pixel is 0-71 for a 72 neopixel strip
void send_RGB_to_pixel(int r, int g, int b, int pixel) {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    // Update the byte array with the specified RGB color pattern
    RGBpayload[pixel * 3 + 0] = (byte)r; // Red
    RGBpayload[pixel * 3 + 1] = (byte)g; // Green
    RGBpayload[pixel * 3 + 2] = (byte)b; // Blue

    // Publish the byte array
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    Serial.println("Published whole byte array after updating a single pixel.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_RGB_to_pixel*.");
  }
}

void send_all_off() {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    // Fill the byte array with the specified RGB color pattern
    for(int pixel=0; pixel < num_leds; pixel++){
      RGBpayload[pixel * 3 + 0] = (byte)0; // Red
      RGBpayload[pixel * 3 + 1] = (byte)0; // Green
      RGBpayload[pixel * 3 + 2] = (byte)0; // Blue
    }
    // Publish the byte array
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    // Reset timeline position
    currentLEDPosition = 0;
    
    Serial.println("Published an all zero (off) byte array. Timeline reset.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_off*.");
  }
}

void send_all_random() {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    // Fill the byte array with the specified RGB color pattern
    for(int pixel=0; pixel < num_leds; pixel++){
      RGBpayload[pixel * 3 + 0] = (byte)random(50,256); // Red - 256 is exclusive, so it goes up to 255
      RGBpayload[pixel * 3 + 1] = (byte)random(50,256); // Green
      RGBpayload[pixel * 3 + 2] = (byte)random(50,256); // Blue
    }
    // Publish the byte array
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    Serial.println("Published an all random byte array.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_random*.");
  }
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

// ===== WIFI AND MQTT FUNCTIONS =====

void startWifi() {
  // Check if the WiFi module is present
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true); // Stop execution
  }

  // Set RGB LED to red while connecting
  WiFiDrv::digitalWrite(25, LOW);  // Green off
  WiFiDrv::digitalWrite(26, HIGH); // Red on
  WiFiDrv::digitalWrite(27, LOW);  // Blue off

  // Attempt to connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set RGB LED to green when connected
  WiFiDrv::digitalWrite(25, HIGH); // Green on
  WiFiDrv::digitalWrite(26, LOW);  // Red off
  WiFiDrv::digitalWrite(27, LOW);  // Blue off
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Create a unique client ID using the MAC address
    byte mac[6];
    WiFi.macAddress(mac);
    clientId = "MKR1010-";
    for (int i = 0; i < 6; i++) {
      clientId += String(mac[i], HEX);
    }
    
    Serial.print("Client ID: ");
    Serial.println(clientId);
    
    // Attempt to connect with username and password
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
      
      // Subscribe to the topic if you want to receive messages
      mqttClient.subscribe(mqtt_topic.c_str());
      Serial.print("Subscribed to: ");
      Serial.println(mqtt_topic);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("Length: ");
  Serial.println(length);
  
  // If we receive an MQTT message with the correct payload length, switch to MQTT mode
  if (length == payload_size) {
    Serial.println("Received LED data from MQTT - switching to MQTT mode");
    useLocalSensor = false; // Disable CO2 sensor mode
    
    // Update RGBpayload with received data
    memcpy(RGBpayload, payload, payload_size);
    
    // Note: The actual LED strip is controlled elsewhere via MQTT
    // This just updates our local buffer
  }
  
  // To switch back to CO2 sensor mode, send a message with text "CO2_MODE"
  if (length == 8) {
    char message[9];
    memcpy(message, payload, length);
    message[length] = '\0';
    if (strcmp(message, "CO2_MODE") == 0) {
      Serial.println("Switching back to CO2 sensor mode");
      useLocalSensor = true;
    }
  }
  
  // Command to reset timeline: "RESET"
  if (length == 5) {
    char message[6];
    memcpy(message, payload, length);
    message[length] = '\0';
    if (strcmp(message, "RESET") == 0) {
      Serial.println("Resetting LED timeline position");
      currentLEDPosition = 0;
      // Optionally clear all LEDs
      send_all_off();
    }
  }
}