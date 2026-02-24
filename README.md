# 🌱 Hydroponic Automation System

A modular hydroponic monitoring and control system designed for small-scale smart growing environments. The project integrates multiple environmental sensors with automated control logic and a web-based monitoring interface.

---

## 🚀 Overview

This system continuously reads key water and environmental parameters, adjusts actuators accordingly, and streams real-time data to a web dashboard via an ESP-based gateway.

**Design goals:**

- Scalable  
- Modular  
- Embedded-focused  
- Close to industry practices  

---

## 🧩 Features

- Real-time sensor monitoring  
- Automated motor and actuator control  
- SPI communication between controller and gateway  
- WebSocket-based live dashboard  
- EEPROM/Flash configuration support  
- Modular firmware structure  

---

## 🔧 Hardware

- STM32 / Arduino (sensor & control unit)  
- ESP32 / ESP8266 (IoT gateway)  
- BMP180 (temperature & pressure)  
- Gravity pH sensor  
- TDS Meter v1.0  
- MQ gas sensor  
- Water pumps / motors  

---

## 📡 Communication

- Protocol: SPI (STM32/Arduino → ESP)  
- Frame-based data structure  
- WebSocket streaming to browser  

---

## 🎯 Goals

- Maintain optimal hydroponic conditions  
- Provide real-time remote visibility  
- Enable easy scaling with new sensors  

---
