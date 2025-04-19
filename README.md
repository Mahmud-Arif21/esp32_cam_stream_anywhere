# ESP32-CAM WiFi Streaming

A flexible ESP32-CAM project that streams video over WiFi with a clean web interface. Automatically switches between WiFi client and setup mode as needed.

## Features

- **Smart WiFi Management**: Connects to configured WiFi networks or creates a temporary setup access point
- **Web Interface**: Mobile-friendly UI for both WiFi setup and video streaming
- **Flash Control**: Toggle the onboard LED flash via web interface
- **Persistent Settings**: Saves WiFi credentials for automatic reconnection

## Hardware Requirements

- ESP32-CAM module (AI Thinker model recommended)
- USB-to-Serial adapter (for programming)
- 5V power supply

## Setup Instructions

### 1. Prepare Arduino IDE

- Install [Arduino IDE](https://www.arduino.cc/en/software)
- Add ESP32 board support via Boards Manager ([guide](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/))
- Install required libraries:
  - ESP32 Arduino Core
  - SPIFFS filesystem

### 2. Connection for Programming

| ESP32-CAM | USB-to-Serial Adapter |
|-----------|----------------------|
| 5V        | 5V                   |
| GND       | GND                  |
| U0R       | TX                   |
| U0T       | RX                   |
| IO0       | GND (only during upload) |

> **Important**: Connect IO0 to GND only when uploading firmware. Remove this connection for normal operation.

## Project Components

The project consists of two main parts that need to be uploaded separately:

### Part 1: Filesystem Initialization

This component prepares the ESP32-CAM's internal storage for web files.

#### Upload Process

1. Open `filesystem_init/SPIFFS_upload/SPIFFS_upload.ino` in Arduino IDE
2. Select board: "AI Thinker ESP32-CAM" or "ESP32 Wrover Module"
3. Set upload speed: 115200
4. Upload the sketch
5. Open Serial Monitor at 115200 baud to verify successful SPIFFS mounting
6. Use "Tools → ESP32 Sketch Data Upload" to upload web files from the `data` folder

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
4. Remove IO0-GND connection and reset the device
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
   - Flash control and other features are accessible

2. **Setup Access Point Mode** (Configuration Only):
   - Activates only when no stored WiFi credentials exist or connection fails
   - Creates a "mini-cam" access point for configuration purposes
   - No video streaming is available in this mode
   - Only used to input WiFi credentials

### First-time Setup

1. Power on the ESP32-CAM
2. The device will create a "mini-cam" WiFi network (no password)
3. Connect to this network from your phone or computer
4. Open a web browser and navigate to `192.168.4.1`
5. Enter your WiFi network credentials and connect
6. The device will restart and connect to your WiFi network

### Normal Operation

1. The ESP32-CAM will connect to your configured WiFi network
2. Find the device's IP address (check your router or serial monitor)
3. Navigate to this IP address in a web browser
4. View the live stream and control the flash

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
