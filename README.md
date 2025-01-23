This project implements an automatic system to monitor the electrical consumption of a single-phase load. This system is designed to monitor the electrical consumption of a single-phase load, measuring key electrical parameters such as:

- RMS voltage and frequency of the power supply
- RMS current and frequency through the load
- Active power
- Apparent power
- Power factor
- Energy consumed over time

Using an ESP32 microcontroller, the system calculates and processes the measurements, which are then sent to the ThingSpeak platform for remote monitoring. The data is updated every minute.

Features:

- Real-time measurement of electrical parameters
- Integration with ThingSpeak for remote monitoring
- Accurate tracking of energy consumption

Components:

- ESP32 Microcontroller: A low-cost microcontroller with integrated Wi-Fi, widely used in IoT and robotics projects due to its ease of programming and wireless capabilities.
- Signal Conditioning: The ESP32 can only read voltages between 0 and 3.3V, requiring signal conditioning to safely measure high-voltage signals from the electrical grid. This was achieved using:
    - LEM LV 25-P: Voltage transducer for signal attenuation.
    - YHDC Current Transformer SCT-013 20A/1V: Current transducer used for current signal attenuation.
- Signal Offset Adjustment: To handle the signal range properly, two different summing amplifier circuits were designed to adjust the DC offset of the signals.

This setup allows for accurate and safe monitoring of electrical consumption with real-time data visualization on ThingSpeak.
