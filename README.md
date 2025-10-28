# Vespera_IoT

# Repository contents
-	Context – Vespera Zine
-	Introduction - Project Vespera
-	System Overview
-	How does it work?
- Template view 
- Component breakdown 
-	Does it work?
-	Test Scripts
-	Arduino Code
-	Final Code
-	Pictures
-	Road Map
-	Evaluation

# Context – Vespera Zine
Struggling to focus in class? You might not be the only one! Recognise poor ventilation instantly, with just one glance at Vespera.

Poor ventilation boosts CO2 concentration and reduces the effectiveness of respiration. This means reduced focus, participation and information retention. 

For an optimal study climate this winter, don’t sacrifice focus for warmth, know exactly when to ventilate the room with Vespera.

![20251025_114548](https://github.com/user-attachments/assets/a6d9c6d9-a619-4ef0-9bcb-a3653b7ba65d)
![20251025_114632](https://github.com/user-attachments/assets/76e40421-c275-4c8b-96b7-e97c9aaabdd7)
![20251025_114642](https://github.com/user-attachments/assets/23a486bb-86ce-4129-8296-8082f53e42e7)
![20251025_114652](https://github.com/user-attachments/assets/a5fe56b1-2f28-4668-b4d9-b1a3afd83401)
![20251025_114659](https://github.com/user-attachments/assets/163dd6b6-6092-4de5-8950-824ca8b8d49e)

# Introduction – Project Vespera
Project Vespera explores developing an IoT system to sense, network and control a Luminaire called Vespera. In this project we will use Vespera - a Wi-Fi enabled luminaire controlled by RGB MQTT messages - to develop an IoT system to flag poor classroom ventilation using CO2 ppm as our key indicator.

“The Internet of Things or IoT is the network of devices such as vehicles and home appliances that contain electronics software sensors actuators and connectivity which allows these things to connect interact and exchange data.” Kevin Ashton.

Simply put:

<p align="center">
<b>Physical object(s) + sensors (or actuators) + network connectivity = IoT</b>
</p>

# System Overview
In the case of Project Vespera, we develop our own IoT system where:

<p align="center">
<b>Vespera Luminaire + CO2 sensor (Hailege ENS160) + Arduino MKR1010 Wi-Fi / MQTT = Vespera IoT</b>
</p>

# How does it work? 
  
  # Architecture
<img width="22830" height="7297" alt="Vespera IoT Systems Architecture XL" src="https://github.com/user-attachments/assets/f94c20a1-6367-41d5-a0e9-b9598b84c498" />


## The Vespera IoT system
<b>1.	Stimulus</b>
•	CO2: The atmosphere contains CO2, its concentration is measured in parts per million

<b>2.	Data collection system (See Breadboard device below)</b>
+	Sensor: CO2 sensor Adafruit ENS160 MOX (metal oxide) Gas Sensor Detects CO2 concentration in the atmosphere by measuring the change in current caused by the atmosphere making contact with the hot plate on the circuit, this is communicated to the Arduino by I2C (the SDA and SCL wires). By default readings are observed at intervals measured in milliseconds.
+	Microcontroller Unit: The Arduino MKR 1010 microcontroller unit has multiple libraries installed enabling it to connect to Wi-Fi using Wi-FiNINA with a secrets file Arduino_secrets.h, PubSubClient To publish and subscribe to specific topic channels

<b>3.	Connectivity</b>
+	Payload: The readings from the sensor are processed by the microcontroller using multiple libraries including ScioSense_ENS160 and utility/wifi_drv. With RGB values linked to CO2 PPM thresholds and corresponding hexadecimal payload values, we can send MQTT messages over Wi-Fi.
+	WIFI Gateway: CE-Wi-Fi located in One Pool Street
+	Tilt Controller: Receives the MQTT payload via WIFI and selects which topic the Vespera luminaires is subscribed. The payload is then published to the coded topic e.g. #6 and sent to the MQTT Broker.
+	QTT Broker: (Mqtt.cetools.org) receives the published payload and if the credentials are correct, it will relay the message to the Vespera Luminare assigning the relevant LED configuration to reflect the CO2 observation.

<b>4.	Vespera Light</b>
+	Physical: Is updated when dialled into the appropriate topic set by the Tilt Controller.
+	Virtual: (IoT.io/projects/lumi/) illustrates the equivalent display shown by the physical luminaire in CASA00014 classroom.
+	Function: 3 LEDs are lit each hour with the colour reflecting the CO2 levels (CO2 <450PPm = Green, CO2 451-750ppm = Orange, CO2 >751 = Red).
+	New observations are appended until all 72 LEDs are showing the past 24 observations (24hrs work of CO2 readings) after which the light is reset.



  # Component Breakdown
- **Physical Object- Vespera:** Vespera is a light or luminaire comprised of 72 NeoPixel LEDs which can be controlled by MQTT messages with RGB payloads.
-	**Breadboard**
  -	**Sensor – ENS160:** Hailege ENS160+AHT21 is a CO2 sensor measuring eCO2 PPM
  -	**Connectivity enabled microcontroller – Arduino:** Arduino MKR1010 is a Wi-Fi enabled micro controller capable of sending MQTT messages.
-	**MQTT Broker** (mqtt.cetools.org)
-	**Web Viewer** (https://www.iot.io/projects/lumi/)

  # Assembly
Follow the steps illustrated by the attached ADAFruit literature.
https://cdn-learn.adafruit.com/downloads/pdf/adafruit-ens160-mox-gas-sensor.pdf

Combined it should look like so:

- 5V to VIN (Voltage In)
- GND to GND (Ground)
- SCL to SCL (Serial Clock Line)
- SDA to SDA (Serial Data Line)

<img width="745" height="564" alt="image" src="https://github.com/user-attachments/assets/b134e1c6-66df-411f-84c4-4ab0b125a0aa" />

# Test Scripts – Does it work?

1. Does the board work? Use the [Blink](https://github.com/chad-casa/Vespera_IoT/blob/8df0a0b58fe31e11d76ad6d3414e8fec8753c69a/Tests/Blink.ino) script to tested the Arduino MKR1000's functionality

2. Can it connect to WI-FI? Use the [SimpleWifi tester](https://github.com/chad-casa/Vespera_IoT/blob/8b2bf5f1b79ee9ebf1dc66fb2b9969085085122d/Tests/SimpleWebServerWiFi-webled-mkr1010.ino) Arduino code to validate that the device can connect to the internet over WI-FI 

3. Can it send MQTT messages?

4. Does the CO2 sensor work? Use [ENS16x code](https://github.com/chad-casa/Vespera_IoT/blob/5368505f9161e4627b4a5e9a68e908a2f0ed466e/Tests/ens160basic_std.ino) to test the sensor takes CO2 readings
<img width="959" height="562" alt="image" src="https://github.com/user-attachments/assets/1e0c6453-26c5-4c25-958d-ce4c45b5d6e4" />

5. Are they recieved by Vespera Luminaire?

# Arduino Final Code

# Pictures

# Road Map
**Stages of development**
1. Zine mindmapping my product thesis
2. Product component selection
3. Product component assembly
4. Functional testing of components using Arduino sketch examples (Blink, WIFININA AP_SimpleWebserver, ScioSense_ENS16X)
5. Wired up a breadboard and CO2 sensor to evaluate whether my chosen sensor works to monitor ppm
6. Test the project [Luminaire script](https://github.com/ucl-casa-ce/casa0014/blob/cc7aed6253ad8d2e7b3fdea0c4e44cc227731e9e/vespera/luminaire_mkr1010_controller/luminaire_mkr1010_controller.ino) from Duncan Wilson, CASA
7. Consolidate ScioSense sketch with Luminaire script
8. Connect my device To MQTT gateway
9. Link sensor readings in serial monitor to RGB LED changes
10. Send RGB chnages to Vespera online tool on topic 6 (my designated channel)
11. Save historic readings on the vespera light to illustrate 1/hour - 3 Neopixel strips coloured represent 1 hour

**Future Plans**
1. Add multiple sensors at different heights and positions into the network to ensure accurate readings
2. Enable Sleep mode to ensure energy efficiency given power intensive nature of Wifi
3. Create wireless version of product for travel

# Assumptions
- Good WI-FI signal
- Access to wired power
- Data privacy is not an important risk

## Why Arduino MKR1010?

  
## Why MQTT?
**Message Queue Telemetry Transport** is a publish and subscribe model protocol where the broker decouples senders and receivers, so they don’t need to know about one another. Benefits for CO2 sensing, small message size and continuous connection. The publish – subscribe model enables the storage of messages which ensures reliability
Low latency is not necessary due to the low frequency of sensing required. Large flows, energy efficiency is not imperative as the device is designed for indoor use so a wireless power source is not necessary and people in the environment are sedentary and in close proximity to the sensor for prolonged periods. 
Data transferred is not highly sensitivity so weak security or no default encryption is not a concern.

# Evaluation
The sensor is working and is selecting RGB values based off of the PPM reading in the serialoutput. The current bounds are 600 - 1000 - 1500 ppm this seems fairly high as the current average is ~400. Readings have been tweaked to run every 10 seconds to prevent spam. In practice readings should be a few times an hour e.g. every 15 minutes.
<img width="954" height="561" alt="image" src="https://github.com/user-attachments/assets/b6771a0a-9750-4189-b10c-3c15db77a52c" />

“Don’t assume your measurements are valid” – Duncan Wilson

- **Reliability and validity of readings:** Multiple sensors, fine tune sensor positioning in room
- **Relevance of readings:** Consider relevant thresholds for CO2 colour change
- **Cost of measurement:** Consider appropriate cadence of measurement

# Sources: 
**Base code** - Arduino template libraries
**Base code** - Luminaire controller code by[Duncan Wilson](https://github.com/ucl-casa-ce/casa0014/blob/cc7aed6253ad8d2e7b3fdea0c4e44cc227731e9e/vespera/luminaire_mkr1010_controller/luminaire_mkr1010_controller.ino) (https://github.com/ucl-casa-ce/casa0014/tree/main/vespera)
**Wiring** - https://cdn-learn.adafruit.com/downloads/pdf/adafruit-ens160-mox-gas-sensor.pdf
**How to merge sketches** - Claude.AI
