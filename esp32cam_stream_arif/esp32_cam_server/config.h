#ifndef CONFIG_H
#define CONFIG_H

// Default AP mode settings
#define AP_SSID "mini-cam"
#define AP_PASSWORD ""  // Empty password for open network

// Default streaming settings
#define STREAM_PORT 80
#define STREAM_FRAME_RATE 15

// File system settings
const char* SSID_FILE = "/ssid.txt";
const char* PASSWORD_FILE = "/password.txt";

// Default camera settings
#define DEFAULT_FRAME_SIZE FRAMESIZE_VGA
#define DEFAULT_JPEG_QUALITY 15  // 0-63 lower means higher quality

#endif // CONFIG_H
