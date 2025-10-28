//Chad Barrett Oct 2025 - Combined: MQTT messanger to vespera + ENS160 CO2 sensor

// works with MKR1010

//install relevant libraries
#include <SPI.h>
#include <WiFiNINA.h> //allows connectivity to local or internet network
#include <PubSubClient.h> //allows to the device to send and recieve MQTT messages
#include "arduino_secrets.h" //Creates a secret tab to store sensitive gateway access information e.g. Wifi password
#include <utility/wifi_drv.h> // Library to drive to RGB LED on the MKR1010
#include <Wire.h>//allows for communication to I2C devices which send and receive data using a Serial CLock Pin (SCL) and Serial Data Pin where data is sent between devices
#include "ScioSense_ENS160.h" //Library for the ENS160 digital four channel MOX gas sensor with a ScioSense I2C interface

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

// CO2 history tracking
int lastCO2_ppm = -1; // Track the last CO2 reading to detect changes
int currentLEDPosition = 0; // Tracks which set of 3 LEDs to update next (0, 3, 6, 9, etc.)

// Forward declarations
void startWifi();
void reconnectMQTT();
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
    Serial.println("ENS160 sensor failed to initialize!");
  }

  // Connect to WiFi
  startWifi();

  // Connect to MQTT broker
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2000);
  
  Serial.println("Set-up complete");
  Serial.println("Reading CO2 sensor and updating Vespera luminaire");
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

  // Read CO2 sensor and update LEDs
  if (millis() - lastSensorRead >= sensorInterval) {
    lastSensorRead = millis();
    
    if (ens160.available()) {
      ens160.measure(true);
      
      int co2_ppm = ens160.geteCO2();
      int tvoc = ens160.getTVOC();
      int aqi = ens160.getAQI();
      
      // Print sensor readings
      Serial.println("========================================");
      Serial.print("AQI: "); Serial.print(aqi); Serial.print("\t");
      Serial.print("TVOC: "); Serial.print(tvoc); Serial.print("ppb\t");
      Serial.print("eCO2: "); Serial.print(co2_ppm); Serial.println("ppm");
      
      // Only update LEDs if CO2 level has changed significantly (by more than 50 ppm)
      if (abs(co2_ppm - lastCO2_ppm) > 50 || lastCO2_ppm == -1) {
        updateLEDsFromCO2(co2_ppm);
        lastCO2_ppm = co2_ppm;
      } else {
        Serial.println("CO2 level unchanged - not adding to timeline");
      }
      Serial.println("========================================");
    }
  }
}

// Convert CO2 reading to RGB color
void co2ToColor(int co2_ppm, byte &r, byte &g, byte &b) {
  if (co2_ppm < 450) {
    // Good air quality - Green
    r = 0; g = 255; b = 0;
  } else if (co2_ppm < 750) {
    // Moderate - Orange
    float ratio = (co2_ppm - 750) / 450.0;
    r = 255;
    g = 255 * (1 - ratio);
    b = 0;
  } else if (co2_ppm > 751) {
    // Poor - Red
    r = 255;
    g = 0;
    b = 0;
  }  
}

// Update the next 3 LEDs based on CO2 reading (timeline mode)
void updateLEDsFromCO2(int co2_ppm) {
  byte r, g, b;
  co2ToColor(co2_ppm, r, g, b);
  
  // Update the next 3 LEDs in sequence
  for (int i = 0; i < 3; i++) {
    int ledIndex = currentLEDPosition + i;
    if (ledIndex < num_leds) {
      RGBpayload[ledIndex * 3 + 0] = r; // Red
      RGBpayload[ledIndex * 3 + 1] = g; // Green
      RGBpayload[ledIndex * 3 + 2] = b; // Blue
    }
  }
  
  // Publish to MQTT (send to Vespera luminaire)
  if (mqttClient.connected()) {
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.print("✓ Sent to Vespera - LEDs ");
    Serial.print(currentLEDPosition);
    Serial.print("-");
    Serial.print(currentLEDPosition + 2);
    Serial.print(" | RGB(");
    Serial.print(r); Serial.print(",");
    Serial.print(g); Serial.print(",");
    Serial.print(b); Serial.print(") | CO2: ");
    Serial.print(co2_ppm);
    Serial.println(" ppm");
  } else {
    Serial.println("✗ MQTT not connected - cannot send to Vespera");
  }
  
  // Move to the next set of 3 LEDs
  currentLEDPosition += 3;
  
  // If we've filled all 72 LEDs (24 readings), wrap back to the beginning
  if (currentLEDPosition >= num_leds) {
    currentLEDPosition = 0;
    Serial.println("*** Timeline full - wrapping back to start ***");
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
    while (true);
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
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}