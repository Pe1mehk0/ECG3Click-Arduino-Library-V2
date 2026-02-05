# ECG 3 Click (MAX30003) Arduino Library

An open-source Arduino driver for the **Maxim Integrated MAX30003** Biopotential AFE (Analog Front End).

This library serves as a dedicated driver for the **MikroE ECG 3 Click**, enabling seamless communication with the **MAX30003** Biopotential AFE via SPI. 
It was born out of necessity: at the time of its inception, no Arduino library existed for this specific board, requiring a custom solution.

The core of this driver was originally engineered for **Vital**, a wearable health-monitoring system developed for the **Samsung Solve for Tomorrow** competition in Poland. To support the project's goal of assisting emergency departments, the code was optimized to stream continuous raw ECG data for real-time cardiac monitoring.

Following the release of the first version and receiving valuable interest from the community, this library has evolved into **Version 2.0.0**. This major update brings upgraded functionality, cleaner architecture, and improved stability, continuing the mission to help researchers and developers easily integrate professional-grade ECG sensing into the Arduino ecosystem.

> **⚠️ DISCLAIMER: RESEARCH & HOBBY USE ONLY**
> This software and the associated hardware are **NOT** medical devices. They are not intended for the diagnosis, treatment, cure, or prevention of any disease or health condition. This library is provided for educational and research purposes only. **The author assumes no liability for the use of this code.**

## ✨ Features

* **Raw ECG Data:** Reads ECG signals.
* **Heart Rate Detection:** Utilizes the chip's internal R-to-R hardware algorithm for accurate heart rate (BPM) calculation.
* **Hybrid Lead-Off Detection**: The library implements a dual-layer check in `isLeadOff()`. It combines hardware status flags with software-calculated saturation thresholds (±35,000 counts) to distinguish between pathological signals and floating-electrode saturation.

* **Ready-to-use Defaults:**
    * **Sample rate:** 128 Hz
    * **Gain:** 40 V/V
    * **Filters:** 0.5 Hz High Pass / 40 Hz Low Pass
    * **Calibration:** Bipolar, 500 µV amplitude

* **Configuration:**

To ensure maximum reliability and a minimal memory footprint for wearable applications, this library uses a **Static Configuration** architecture. 

** **Manual Customization**: Users requiring different specifications can easily modify the `ecg3_click.cpp` `begin` function. The header file includes a comprehensive register map for:
    *** **Extended Gain Control**: Switch between 20V/V and 40V/V.
    *** **Alternative Bias Resistors**: Options ranging from 50MΩ to 200MΩ.
    *** **Lead-Off Thresholds**: Customizable voltage and current thresholds for electrode detection.

> **Note**: For "out-of-the-box" testing, the provided Python script includes a live ECG graph and BPM/RTOR monitoring with digital filters pre-tuned to these default hardware settings.

## Compatibility & Testing

**Development Setup:**
* **Microcontroller:** Adafruit Feather ESP32 V2
* **Sensor Board:** MikroE ECG 3 Click

**Compatible With:**
* Any Arduino-compatible board (ESP32, STM32, AVR/Uno/Mega, Teensy).

## Wiring (Standard SPI)

Connect the ECG 3 Click (or MAX30003 breakout) to your microcontroller as follows:

| ECG 3 Click Pin | Arduino/ESP32 Pin | Function |
| :--- | :--- | :--- |
| **3.3V** | 3.3V | Power |
| **GND** | GND | Ground |
| **CS** | GPIO 15 (or 10) | Chip Select |
| **SCK** | GPIO 18 (or 13) | SPI Clock |
| **MISO** | GPIO 19 (or 12) | Master In / Slave Out |
| **MOSI** | GPIO 23 (or 11) | Master Out / Slave In |

*(Note: Pin numbers vary by board. Check your specific microcontroller's pinout.)*

## Installation

### Method 1: Global Install
1.  Download this repository.
2.  Copy the entire `ECG3_Click_Library` folder into your Arduino `libraries` folder:
3.  Restart the Arduino IDE.

### Method 2: Local Project
If you only want to use this library for one specific project:
1.  Copy `src/ecg3_click.h` and `src/ecg3_click.cpp` directly into your sketch folder (next to your `.ino` file).

## 🚀 Quick Start

Here is the minimal code to get the ECG 3 Click running.

```.ino
#include <SPI.h>
#include "ecg3_click.h" // Works if installed in libraries OR local folder

// Define Chip Select (CS) Pin - Adjust for your board!
// ESP32 Feather: 15, Arduino Uno: 10
#define CS_PIN 15  

ECG3Click ecg(CS_PIN);

void setup() {
    Serial.begin(115200);
    
    // Initialize SPI (SCK, MISO, MOSI)
    // Example pins for ESP32 Feather: 5, 21, 19
    if (ecg.begin(5, 21, 19)) {
        Serial.println("MAX30003 Initialized");
    } else {
        Serial.println("Initialization Failed");
    }
}

void loop() {
    // Check if new data is available
    if (ecg.getStatus() & _MAX30003_EINT_MASK) {
        
        // Print ECG Data
        Serial.print("ECG:");
        Serial.println(ecg.getECG());

        // Check for Lead-Off
        if (ecg.isLeadOff(ecg.getECG())) {
             Serial.println("Lead Disconnected!");
        }
    }
}
```
## API Reference

| Function | Description |
| :--- | :--- |
| `ECG3Click(int csPin)` | **Constructor.** Creates the ECG object and sets the Chip Select pin. |
| `bool begin(int sck, int miso, int mosi)` | **Initializes** the SPI bus and configures the MAX30003 chip with standard settings (ECG, Filters, R-to-R). Returns `true` if chip ID is verified. |
| `int32_t getECG()` | **Reads** the ECG data. |
| `void getRTOR(uint16_t &hr, uint16_t &rr)` | **Retrieves** the calculated Heart Rate (BPM) and R-to-R interval (ms). Pass two variables by reference to get the values. |
| `bool isRTORReady()` | **Checks** if a new Heart Rate/R-to-R calculation is available to read. |
| `uint32_t getStatus()` | **Reads** the Status Register (STATUS) to check for interrupts. |
| `bool isLeadOff(int32_t currentVal)` | **Checks** if the leads are disconnected. Uses a hybrid method: reads the hardware DC Lead-Off status AND checks if the signal is "railed" (saturated). |
| `uint32_t readRegister(uint8_t regAddr)` | **Low-level Read.** Returns the 24-bit value of a specific register address. |
| `void writeRegister(uint8_t regAddr, uint32_t data)` | **Low-level Write.** Writes a 24-bit value to a specific register address. |

## Python Graph Plotter

This library includes a Python script located in `Examples/Graph_Plotter/` to visualize your ECG signal in real-time on your PC.

**To use it:**
1.  Upload the `ECG_v2` example to your Arduino.
2.  Install Python dependencies: `pip install pyserial matplotlib`
3.  Run the script: `python main.py`

**Digital Signal Processing:**
The Python script implements multiple layers of software filtering to ensure a clean and stable visualization:
* **Moving Average Filter:** Smooths the raw input data to reduce high-frequency noise while preserving the QRS complex.
* **Median Filter (RR Intervals):** Filters heart rate calculations across a 5-beat history to eliminate outliers and provide a stable BPM reading.
* **Dynamic Thresholding:** Automatically adapts to signal amplitude changes, ensuring reliable peak detection even with varying signal-to-noise ratios.

<img width="984" height="622" alt="image_2026-02-03_131521696" src="https://github.com/user-attachments/assets/07a8e6e1-1d4a-4ab6-9093-c130f4f6308d" />

## ❓ Troubleshooting

**1. The ECG signal looks like random noise.**
* Ensure the "Lead-Off" message is not appearing in the Serial Monitor.
* Use a battery or a clean USB power source (laptops on chargers often inject 50Hz/60Hz noise).
* Keep the electrode cables still; movement creates large artifacts.

**2. "Initialization Failed" error.**
* Check your wiring, specifically the **CS (Chip Select)** pin.
* Ensure your `ecg.begin()` call matches your board's SPI pins.

**3. Heart Rate is 0.**
* The MAX30003 takes a few seconds to settle and calculate the first R-to-R interval.
* Ensure the electrodes are placed correctly (Left Arm, Right Arm, Right Leg).

## 🎉 Future Improvements

* **Interrupt Support:** Add support for using the MAX30003 INTB pin to trigger data reads.
* **Dynamic Configuration:** Add functions to change Gain, Sample Rate, and Filters (LPF/HPF) at runtime without editing the library files.
* **Advanced DSP:** Implement software-based 50Hz/60Hz notch filters to remove mains hum.

## License

This project is released under the **MIT License**. See the `LICENSE` file for full details.

## Author

**Aliaksandr Kuryla** 
Developed based on the Maxim Integrated MAX30003 Datasheet. Initialization sequence and logic flow referenced from the MikroE ECG 3 Click driver.

