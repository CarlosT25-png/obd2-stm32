# Car Telemetric OBD2 Scanner

This is a high performance, dual core automotive diagnostic tool that extracts real-time vehicle telemetry (RPM, Speed, Temperatures) via FDCAN and broadcasts it over Bluetooth Low Energy (BLE) to a custom mobile dashboard.

## Overview

This project combines raw automotive CAN bus data and modern mobile applications. It utilizes an STM32G474RE to handle high speed engine querying via a non blocking FreeRTOS state machine, ensuring precise data acquisition without bus contention. The telemetry is then serialized and transferred to an ESP32 via a packed struct UART bridge for BLE transmission.

In order to have the full project make sure to check the following repositories.
## 🔗 Complementary GitHub Repos
[OBD2 Dashboard PWA](https://github.com/CarlosT25-png/obd2-pwa)

[OBD2 BLE Transmitter](https://github.com/CarlosT25-png/obd2-ble)


## Technical Components

- STM32 Nucleo-G74RE4
- ESP32 (I have an ESP32 to transmit the car data over BLE, but if you have a BLE module, you can also use that)
- CAN transceiver SN65HVD230 (I used a Waveshare SN65HVD230 CAN Board)
- OBD2 Pigtail Cable
- Jumper Wires
## Screenshots

![GIF](https://github.com/CarlosT25-png/obd2-stm32/blob/main/docs/OBD2-MCU.gif?raw=true)
![Screenshot of the app](https://github.com/CarlosT25-png/obd2-stm32/blob/main/docs/screenshot-app.PNG?raw=true)
![Diagram](https://github.com/CarlosT25-png/obd2-stm32/blob/main/docs/diagram.JPEG?raw=true)

## Authors

- [@CarlosT25-png](https://www.github.com/CarlosT25-png)


## License

[MIT](https://choosealicense.com/licenses/mit/)

