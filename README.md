# ESP32-CAM WiFi Streaming On Any Network

A flexible ESP32-CAM project that streams video over WiFi with a clean web interface. Automatically switches between WiFi client and setup mode as needed.

## Features

- **Smart WiFi Management**: Connects to configured WiFi networks or creates a temporary setup access point
- **Web Interface**: Mobile-friendly UI for both WiFi setup and video streaming
- **Flash Control**: Toggle the onboard LED flash via web interface
- **Persistent Settings**: Saves WiFi credentials for automatic reconnection

## Hardware Requirements

- ESP32-CAM module (AI Thinker model is used here)
- USB-to-Serial adapter (for programming)
- 5V power supply

## Setup Instructions

### 1. Prepare Arduino IDE

- Install [Arduino IDE](https://www.arduino.cc/en/software)
- Add ESP32 board support via Boards Manager ([guide](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/))
- Install required libraries:
  - ESPAsyncWebServer
  - AsyncTCP
  - img_converters
  - SPIFFS

### 2. Connection for Programming

> ⚠️ **Note:**

> In my setup, I used the **ESP32-CAM-MB module** (USB programmer base for the ESP32-CAM), which provides the **most convenient and reliable method** for uploading firmware.

> This module includes a built-in USB-to-Serial converter and handles auto-reset and boot mode selection **automatically**, so there's **no need for manual wiring or pressing buttons** during upload.  

> If you don't have the ESP32-CAM-MB, the following sections describe **alternative programming methods** using an Arduino UNO or an FTDI module.

#### 2.1 Using an Arduino UNO as USB-to-Serial Programmer

*Logic-level note:* The UNO’s TX/RX operate at 5 V while the ESP32‑CAM uses 3.3 V UART, but from the experience of most users, we can flash successfully without level shifting when powering the module at 5 V.

![mini-cam server](Images\esp32_cam_arduino.png)


| ESP32‑CAM Pin    | Arduino UNO (Programmer)              |
|------------------|---------------------------------------|
| **5 V**          | 5 V (UNO 5 V pin)                     |
| **GND**          | GND                                   |
| **U0R (RX)**     | TX (Digital Pin 1)                    |
| **U0T (TX)**     | RX (Digital Pin 0)                    |
| **IO0 (BOOT)**   | GND (only during firmware upload)     |
| **RESET (UNO)**  | GND (to disable UNO MCU during upload)|

> **Note:** Tie RESET and IO0 to GND only while pressing Upload in the IDE; remove both after “Done uploading.”

---

#### 2.2 Using an FTDI FT232RL Module

Set the VCC jumper to 5 V, then wire as follows:

![mini-cam server](Images\esp32_cam_FTDI.png)

| ESP32‑CAM Pin    | FTDI FT232RL Module                      |
|------------------|-------------------------------------------|
| **5 V**          | VCC (set jumper to 5 V) → 5 V             |
| **GND**          | GND                                       |
| **U0R (RX)**     | TX                                        |
| **U0T (TX)**     | RX                                        |
| **IO0 (BOOT)**   | GND (only during firmware upload)         |

> **Tip:** Press IO0 to GND then press Reset when you see the IDE’s “Connecting…” dots.

---

#### 2.3 Using the ESP32‑CAM‑MB USB Adapter Shield

The shield plugs directly onto the ESP32‑CAM and provides built‑in USB conversion (CH340C) plus BOOT and RESET buttons.

![mini-cam server](Images\esp32_cam_mb_driver.png)

| ESP32‑CAM Pin    | ESP32‑CAM‑MB Programmer Shield           |
|------------------|-------------------------------------------|
| **5 V**          | 5 V (powered via Micro‑USB on shield)     |
| **GND**          | GND (common ground on shield)             |
| **U0R (RX)**     | CH340C TX (internally routed to USB)      |
| **U0T (TX)**     | CH340C RX (internally routed to USB)      |
| **IO0 (BOOT)**   | BOOT button (hold during upload)          |
| **RST (RESET)**  | ESP32‑CAM on‑board RST button             |

> **Reminder:** Release BOOT after upload and use the on‑board RST button if the shield’s RESET doesn’t trigger the module correctly.



## Project Components

The project consists of two main parts that need to be uploaded separately:

### Part 1: Filesystem Initialization

This component prepares the ESP32-CAM's internal storage for static files (html, css) for the web interface.

#### Upload Process

1. Open `filesystem_init/SPIFFS_upload/SPIFFS_upload.ino` in Arduino IDE
2. Select board: "AI Thinker ESP32-CAM"
3. Set upload speed: 115200
4. Upload the sketch
5. Open Serial Monitor at 115200 baud to verify successful SPIFFS mounting

#### Customization

- **Web Interface**: Modify files in the `data` folder:
  - `wifisetup.html`: WiFi configuration page
  - `streaming.html`: Video streaming interface
  - `style.css`: Appearance and layout

### Part 2: Camera Server

This is the main application that handles the camera, WiFi connection, and web server.

#### Upload Process

1. Open `esp32_cam_server/esp32_cam_server.ino` in Arduino IDE
2. Verify all header files are in the same folder
3. Upload the sketch
4. Reset the device
5. Check Serial Monitor at 115200 baud for IP address information

#### Customization

- **WiFi Settings**: Modify `config.h`
  - Change setup AP name and password
  - Adjust streaming parameters

- **Camera Configuration**: Modify `camera_config.h`
  - Change resolution (FRAMESIZE_VGA, FRAMESIZE_SVGA, etc.)
  - Adjust JPEG quality (0-63, lower is better quality)

- **Advanced Streaming**: The project includes two streaming implementations:
  - Default: Simple streaming in the main file
  - Alternative: Advanced streaming in `server_handlers.h` (requires additional libraries)

## Usage

### WiFi Operation Modes

The ESP32-CAM operates in two modes:

1. **WiFi Client Mode** (Normal Operation):
   - The device connects to your WiFi network using stored credentials
   - Video streaming is available in this mode
   - Flash LED control, network disconnect and other features are accessible

2. **Setup Access Point Mode** (Configuration Only):
   - Activates only when no stored WiFi credentials exist or connection fails
   - Creates a **`mini-cam`** access point for configuration purposes
   - No video streaming is available in this mode
   - Only used to input WiFi credentials

### First-time Setup

1. Power on the ESP32-CAM module

2. The device will create a **`mini-cam`** WiFi network **(no password)**
![mini-cam server](Images\AP_Mode_Serial_Monitor.png)

3. Connect to this network from your phone or computer
![mini-cam server](Images\AP_Mode_Server.png)

4. Open a web browser and navigate to `192.168.4.1`

5. Enter your WiFi network credentials and connect
![mini-cam server](Images\AP_Mode_UI.png)

6. The device will restart and connect to your WiFi network

### Normal Operation

1. The ESP32-CAM will connect to your configured WiFi network

2. Find the device's IP address (check serial monitor)
![mini-cam server](Images\STA_Mode_Serial_Monitor.png)

3. Navigate to this IP address in a web browser
![mini-cam server](Images\STA_Mode_UI.png)

4. View the live stream and control the **flash** or **disconnect** from current network if necessary
![mini-cam server](Images\STA_Mode_UI_Flash_On.png)

## Project Structure

```
esp32cam_stream
├───esp32_cam_server         # Main application
│       camera_config.h      # Camera pin definitions
│       config.h             # WiFi and server settings
│       esp32_cam_server.ino # Main code
│       server_handlers.h    # Alternative streaming implementation
│
└───filesystem_init          # Filesystem setup
    └───SPIFFS_upload        # SPIFFS initialization utility
        │   SPIFFS_upload.ino # Filesystem formatter
        │
        └───data             # Web interface files
                streaming.html
                style.css
                wifisetup.html
```

## Troubleshooting

- **Cannot upload code**: Ensure IO0 is connected to GND during upload
- **No WiFi AP appears**: Check power supply; ESP32-CAM needs stable 5V
- **Camera fails to initialize**: Verify camera module connections
- **Stream is slow**: Lower resolution in `camera_config.h`
- **SPIFFS errors**: Run the filesystem initialization sketch first
- **Connection issues**: Try resetting the device by pressing the RST button
- **Device stays in AP mode**: Check that you've entered correct WiFi credentials

## License

This project is open-source and available under the MIT License.

## Acknowledgments

- ESP32 Arduino Core developers
- AI Thinker for the ESP32-CAM module
