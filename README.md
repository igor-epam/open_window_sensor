# ESP8266 Wemos D1 Door/Window Monitoring with Telegram Notification

This project aims to create a door and window monitoring system using an ESP8266 based Wemos D1 development board, 433MHz sensors, and EV1527 protocol. When doors or windows are opened for a configured period, users receive real-time status notifications via a Telegram bot. The purpose of this project is to track if any window in a house is opened for longer than 10 minutes (configurable via telegram command), and notify the client about it every 3 minutes (interval is configurable via telegram command as well). 

# Features
- Door/window status monitoring using 433MHz sensors.
- Real-time notifications via Telegram bot.
- Configurable time threshold for notifications.
- Based on EV1527 communication protocol.

# Hardware Components
- ESP8266 Wemos D1 development board.
- 433MHz sensor for door/window status detection. TAIBOAN Wireless 433mhz window door sensor was used.
- 433MHz receiver module for communication. SYN480R was used.
- Connect SYN480R to 3.3v, GND, GPIO4 on a board.
- A 3d printed case https://www.thingiverse.com/thing:3090428
- Antenna for the 433MHz receiver. It could by a DIY one like here or similar https://www.raypcb.com/433mhz-pcb-antenna-design/. 

# Software Requirements
- Arduino IDE for ESP8266 development / Visual studio code with platformio plugin.

# Getting Started

- Clone the GitHub repository to your local machine.
- Set up the hardware components according to the provided documentation and wiring diagram.
- Modify the configuration in the provided Arduino sketch to include your Wi-Fi and Telegram credentials.
- Compile and upload the sketch to your ESP8266 Wemos D1 board using the Arduino IDE.
- On startup, the 'window_sensor' WIFI access point will be established with a password 'password' waiting for a setup. The user should select a WIFI to connect to, provide a password and in addition to the WIFI configuration provide a telegram API key and telegram chat identifiers, separated by a comma.
- Monitor the door/window status and test the notification functionality via Telegram.

For more information on setting up the hardware, required libraries, and detailed usage instructions, please contact the author.

# Contributing

Contributions are welcome! Feel free to create issues, submit pull requests, or discuss improvements in the GitHub repository.