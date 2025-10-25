# Vespera_IoT
CASA project to control a luminaire LED lamp using an IoT sensor.
Stages of development:
#1 Idea brainstorm: Co2 sensor to visually display Co2 readings of the room to ensure optimal ventilation during the winter season where windows are often kept shut due to the cold, leading to stale air and tired students with lower attention spans.
<img width="3022" height="725" alt="image" src="https://github.com/user-attachments/assets/e2807af8-487e-4e6c-a0ee-0f61469e233f" />

Does by hardware work?
1.Tested the Arduino MKR1000 Functionality by using the Blink script
2. Can it connect to Wifi? Used the SimpleWifi tester Arduino code to validate that the hardware works.

How to choose and wire the CO2 sensor?
https://cdn-learn.adafruit.com/downloads/pdf/adafruit-ens160-mox-gas-sensor.pdf 
3. Wired up a breadboard and CO2 sensor to evaluate whether my chosen sensor works to monitor ppm
4. Connect my device To MQTT gateway
5. Link sensor readings in serial monitor to RGB LED changes
6. Send RGB chnages to Vespera online tool on topic 6 (my designated channel)
7. Save historic readings on the vespera light to illustrate 1/hour
8. 

<img width="745" height="564" alt="image" src="https://github.com/user-attachments/assets/b134e1c6-66df-411f-84c4-4ab0b125a0aa" />

<img width="959" height="562" alt="image" src="https://github.com/user-attachments/assets/1e0c6453-26c5-4c25-958d-ce4c45b5d6e4" />

The sensor is working and is selecting RGB values based off of the PPM reading in the serialoutput. The current bounds are 600 - 1000 - 1500 ppm this seems fairly high as the current average is 40. readings have been tweaked to run every 10 seconds to prevent spam.
<img width="954" height="561" alt="image" src="https://github.com/user-attachments/assets/b6771a0a-9750-4189-b10c-3c15db77a52c" />

Sources: 
Base code - Arduino template libraries
Base code - Duncan Wilson (https://github.com/ucl-casa-ce/casa0014/tree/main/vespera)
Wiring: https://cdn-learn.adafruit.com/downloads/pdf/adafruit-ens160-mox-gas-sensor.pdf
How to merge libraries - Claude.AI
