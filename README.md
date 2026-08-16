# Smart Lecture Hall Energy Saving System

## 📌 Project Overview

The Smart Lecture Hall Energy Saving System is an Arduino-based automation system designed to reduce unnecessary electricity consumption in university lecture halls.

The system detects people entering and leaving the lecture hall, maintains a real-time occupancy count, detects human presence using a PIR sensor, and automatically controls electrical devices based on the occupancy status.

The main purpose of the project is to prevent lights and other connected electrical devices from remaining ON when the lecture hall is empty.

## 🎯 Objectives

- Automatically detect people entering and leaving the lecture hall
- Maintain a real-time people count
- Detect human presence using a PIR sensor
- Automatically control electrical devices
- Reduce unnecessary electricity consumption
- Provide real-time system feedback through an OLED display
- Reduce the need for manual switching

## ⚙️ How the System Works

The system uses two ultrasonic sensors positioned at the entrance to identify the direction of movement.

### 👤 Person Entry

When a person enters the lecture hall:

1. The first ultrasonic sensor detects the person.
2. The second ultrasonic sensor detects the person.
3. The system identifies the movement as an entry.
4. The people count is increased.
5. The OLED display updates the current count.

### 🚪 Person Exit

When a person leaves:

1. The ultrasonic sensors detect the movement sequence.
2. The system identifies the movement as an exit.
3. The people count is decreased.
4. The OLED display updates the count.

### 💡 Automatic Device Control

The Arduino uses the occupancy count and PIR sensor to determine whether the lecture hall is occupied.

When the hall is occupied, the connected electrical device can be switched ON.

When the hall becomes empty, the system automatically switches the connected light OFF, helping to prevent unnecessary energy consumption.

## 🧩 System Components

- Arduino Uno
- 2 × HC-SR04 Ultrasonic Sensors
- PIR Motion Sensor
- 2-Channel Relay Module
- 0.96-inch OLED Display
- LED Bulb
- Breadboard
- Jumper Wires
- Power Supply

## 🔌 System Architecture

![System_Architecture](Media/System-Architecture.png)
![video](Media/intro.mp4)
