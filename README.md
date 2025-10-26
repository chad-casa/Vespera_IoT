# Vespera_IoT

# Repository contents
-	Context – Project Zine
-	Introduction
-	System Overview
-	How does it work?
  o	Template view 
  o	Component breakdown 
-	Does it work?
-	Test Scripts
-	Arduino Code
-	Final Code
-	Pictures
-	Road Map
-	Evaluation

# Context – Vespera Zine
Struggling to focus on class? You might not be the only one! Recognise poor ventilation instantly, with just a glance at Vespera.

Poor ventilation boosts CO2 concentration reducing the effectiveness of respiration. This means reduced focus, participation and information retention. 

For an optimal study climate this winter, don’t sacrifice focus for warmth, know exactly when to ventilate the room with Vespera.

![20251025_114548](https://github.com/user-attachments/assets/a6d9c6d9-a619-4ef0-9bcb-a3653b7ba65d)
![20251025_114632](https://github.com/user-attachments/assets/76e40421-c275-4c8b-96b7-e97c9aaabdd7)
![20251025_114642](https://github.com/user-attachments/assets/23a486bb-86ce-4129-8296-8082f53e42e7)
![20251025_114652](https://github.com/user-attachments/assets/a5fe56b1-2f28-4668-b4d9-b1a3afd83401)
![20251025_114659](https://github.com/user-attachments/assets/163dd6b6-6092-4de5-8950-824ca8b8d49e)

# Introduction – Project Zine
Project Vespera explores developing an IoT system to sense, network and control a Luminaire called Vespera. In this project we will use Vespera - a Wi-Fi enabled luminaire controlled by RGB MQTT messages - to develop an IoT system to flag poor classroom ventilation.

“The Internet of Things or IoT is the network of devices such as vehicles and home appliances that contain electronics software sensors actuators and connectivity which allows these things to connect interact and exchange data.” Kevin Ashton.

Simply put;

**Physical object(s) + sensors (or actuators) + network connectivity = IoT**

# System Overview
In the case of Project Vespera, we develop our own IoT system where; 

**Vespera Luminaire + CO2 sensor (Hailege ENS160) + Arduino MKR1010 Wi-Fi / MQTT = Vespera IoT**

# How does it work? 
  
  # Architecture
<img width="4965" height="1214" alt="image" src="https://github.com/user-attachments/assets/d4d1e2ad-15da-4da7-9f24-a722e4e2ef23" />

Written description of the visual


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
1.Tested the Arduino MKR1000 Functionality by using the Blink script
2. Can it connect to Wifi? Used the SimpleWifi tester Arduino code to validate that the hardware works.
3. Can it send MQTT messages?
4. Does the CO2 sensor work? <img width="959" height="562" alt="image" src="https://github.com/user-attachments/assets/1e0c6453-26c5-4c25-958d-ce4c45b5d6e4" />
5. Are they recieved by Vespera Luminaire?

# Arduino Final Code

# Pictures

# Road Map

# Evaluation

CASA project to control a luminaire LED lamp using an IoT sensor.
Stages of development:
#1 Idea brainstorm: Co2 sensor to visually display Co2 readings of the room to ensure optimal ventilation during the winter season where windows are often kept shut due to the cold, leading to stale air and tired students with lower attention spans.
<img width="3022" height="725" alt="image" src="https://github.com/user-attachments/assets/e2807af8-487e-4e6c-a0ee-0f61469e233f" />


3. Wired up a breadboard and CO2 sensor to evaluate whether my chosen sensor works to monitor ppm
4. Connect my device To MQTT gateway
5. Link sensor readings in serial monitor to RGB LED changes
6. Send RGB chnages to Vespera online tool on topic 6 (my designated channel)
7. Save historic readings on the vespera light to illustrate 1/hour
   
<img width="745" height="564" alt="image" src="https://github.com/user-attachments/assets/b134e1c6-66df-411f-84c4-4ab0b125a0aa" />

<img width="959" height="562" alt="image" src="https://github.com/user-attachments/assets/1e0c6453-26c5-4c25-958d-ce4c45b5d6e4" />

The sensor is working and is selecting RGB values based off of the PPM reading in the serialoutput. The current bounds are 600 - 1000 - 1500 ppm this seems fairly high as the current average is 40. readings have been tweaked to run every 10 seconds to prevent spam.
<img width="954" height="561" alt="image" src="https://github.com/user-attachments/assets/b6771a0a-9750-4189-b10c-3c15db77a52c" />

# Sources: 
Base code - Arduino template libraries
Base code - Duncan Wilson (https://github.com/ucl-casa-ce/casa0014/tree/main/vespera)
Wiring: https://cdn-learn.adafruit.com/downloads/pdf/adafruit-ens160-mox-gas-sensor.pdf
How to merge libraries - Claude.AI
