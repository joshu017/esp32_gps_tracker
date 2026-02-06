# esp32_gps_tracker
A GPS tracker and environmental sensor data logger for ESP32, released under the MIT License.

# Hardware Bill of Materials (with example sources):
- Lolin32 dev board clone
  - https://www.aliexpress.us/item/3256807098225693.html
- AHT20 Temperature and Humidity sensor (I2C) plus BMP280 Temperature and Pressure sensor (I2C) module
  - https://www.aliexpress.us/item/3256807516159141.html
- U-blox NEO-6 GPS module (serial)
  - https://www.aliexpress.us/item/3256802687680892.html
- SD card reader (SPI)
  - https://www.aliexpress.us/item/3256810561916891.html
- DX-LR02 LORA module (serial)
  - https://www.amazon.com/LR02-Development-868-915Mhz-Set/dp/B0DS1TJ9W5

# Software Library Dependencies (install and manage via Arduino library manager)
- AHT20 by dvarrel https://github.com/dvarrel/AHT20
- BMP280 by dvarrel https://github.com/dvarrel/BMP280
- TinyGPSPlus by Mikal Hart (ESP32 fork) https://github.com/TinyuZhao/TinyGPSPlus-ESP32

# Todo list:
- Add support for the LORA module DX-LR02
- Create the receiver side Arduino sketch
- Add diagrams
- Complete documentation
