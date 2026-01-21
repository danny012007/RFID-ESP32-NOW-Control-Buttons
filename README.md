# RFID-ESP32-NOW-Control-Buttons

This repository contains the code and diagram wiring of my ESP-NOW control buttons project. Sender has to be unlocked with a keychain and has four buttons on an LCD screen that have an "ON" and "OFF" state. When pressed, these buttons send signals to the ESP32 receivers to change a state. Receiver One controls an LED while Receiver Two controls an LED and a DC motor.

[Video demonstration](https://www.youtube.com/watch?v=hILfgU4563I)

Why ESP-NOW over Bluetooth or Wifi?  
ESP-NOW has the range of Wifi, but uses the ESP32 microcontrollers as stations, so does not require a dedicated router. It has low latency and power usage.

# Sender
<img width="1500" height="1259" alt="sender_circuit" src="https://github.com/user-attachments/assets/7b585fdc-8325-4914-8a53-693b6403049e" />

# Receiver 1
<img width="1500" height="1782" alt="receiver_1_circuit" src="https://github.com/user-attachments/assets/33c14e42-af52-4eb4-a173-19a9ad88159b" />

# Receiver 2
<img width="1500" height="1117" alt="receiver_2_circuit" src="https://github.com/user-attachments/assets/71273571-931a-402e-ac32-f37a690cabd6" />
