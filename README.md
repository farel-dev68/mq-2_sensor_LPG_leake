# MQ-2 Gas Monitor with Automated Ventilation System

An Arduino-based smart gas monitoring system designed to detect LPG and Butane gas concentrations using the **MQ-2 sensor**. The system features real-time data visualization on an **I2C 16x2 LCD display**, automated sensor calibration on startup, and dynamic exhaust fan speed control via **PWM** based on ambient gas levels.

---

## Features

* **Automated Startup Calibration:** Automatically calculates the baseline sensor resistance ($R_0$) in clean air over a 60-second period.
* **Visual Calibration Progress:** Shows a custom loading bar on the LCD screen during pre-heating and calibration.
* **4-Tier Safety Logic:** Dynamically adjusts the fan speed and safety status across four distinct threat levels.
* **Dual Output Logging:** Simultaneously updates the physical LCD interface and sends detailed diagnostic data to the Arduino Serial Monitor.

---

## Hardware Requirements

To build this project, you will need the following components:

* **Arduino Uno / Nano / Mega**
* **MQ-2 Gas Sensor** (LPG/Butane)
* **16x2 LCD Display** with **I2C Interface Module**
* **DC Fan** (5V or 12V with an appropriate external transistor/MOSFET driver circuit)
* **Resistor (10 kΩ)** for the load resistor ($R_L$) configuration (if not using a pre-assembled module)
* **Breadboard and Jumper Wires**

---

## Pin Mapping

| Component | Component Pin | Arduino Pin | Description |
| :--- | :--- | :--- | :--- |
| **MQ-6 Sensor** | VCC | 5V | Power Supply |
| | GND | GND | Ground |
| | AO | A0 | Analog Output Data |
| **16x2 I2C LCD** | VCC | 5V | Power Supply |
| | GND | GND | Ground |
| | SDA | A4 (Uno) | I2C Data Line |
| | SCL | A5 (Uno) | I2C Clock Line |
| **DC Fan Driver** | PWM / Gate | D9 | Fan Speed Control |

---

## System Thresholds & Logic

The system continuously calculates gas concentration in **PPM (Parts Per Million)**. The exhaust ventilation fan reacts dynamically according to the thresholds outlined below:

| PPM Range | Safety Condition | LCD Status Display | Fan Speed (PWM Value) |
| :--- | :--- | :--- | :--- |
| **< 200 PPM** | Condition 1: AMAN (Safe) | `AMAN   Fan: OFF` | 0% (0) |
| **200 - 499 PPM** | Condition 2: WASPADA (Warning) | `WASPADA Fan:30%` | 30% (76) |
| **500 - 1499 PPM** | Condition 3: BAHAYA (Danger) | `BAHAYA  Fan:60%` | 60% (153) |
| **≥ 1500 PPM** | Condition 4: DARURAT (Emergency) | `DARURAT Fan:100%` | 100% (255) |

---

## Software Libraries Required

Ensure you have the following libraries installed in your Arduino IDE before uploading the code:

1. **Wire.h** (Built-in library for I2C communication)
2. **LiquidCrystal_I2C.h** (By Frank de Brabander - available via the Arduino Library Manager)

---

## How It Works

### 1. Calibration Phase
Upon powering up or resetting the Arduino, the system enters a **60-second sensor pre-heating and calibration cycle**. 
> **Important:** Ensure the device is placed in a clean air environment during startup so it can properly read the baseline value ($R_0$). A progress bar will fill up on the LCD screen to indicate completion.

### 2. Main Monitoring Loop
* The system reads the analog voltage from pin `A0` and converts it into the sensor resistance ($R_s$).
* Using the characteristic curve formula derived from the MQ-6 datasheet ($1000.0 \times (R_s/R_0)^{-2.95}$), it maps the resistance ratio to PPM values.
* The system updates the LCD display and outputs data to the Serial Monitor every **2 seconds** while simultaneously adjusting the fan speed.

