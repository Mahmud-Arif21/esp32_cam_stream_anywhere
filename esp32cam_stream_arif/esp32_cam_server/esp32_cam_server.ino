#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "SPIFFS.h"
#include "WebServer.h"
#include "config.h"
#include "camera_config.h"

// Flash LED pin
#define FLASH_LED_PIN 4

// Camera streaming status
bool isStreaming = false;
bool isFlashOn = false;

// Create WebServer object on port 80
WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  // Initialize camera (AI Thinker Model)
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Initialize with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 15;  // 0-63 lower means higher quality
    config.fb_count = 3;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 20;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Setup flash LED
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
  
  // Connect to Wi-Fi or Start AP
  if (!connectToStoredWiFi()) {
    // Start AP mode if no connection
    startAPMode();
  } else {
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    setupStreamingServer();
  }
}

void loop() {
  // Handle client requests
  server.handleClient();
}

bool connectToStoredWiFi() {
  // Load stored credentials
  String ssid = readFile(SPIFFS, "/ssid.txt");
  String password = readFile(SPIFFS, "/password.txt");
  
  if (ssid.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Connecting to WiFi...");
    
    // Wait for connection with timeout
    int connectionAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && connectionAttempts < 30) {
      delay(500);
      Serial.print(".");
      connectionAttempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
  }
  
  return false;
}

void startAPMode() {
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  setupAPServer();
}

void setupAPServer() {
  // Route for root / web page
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loadFromSPIFFS("/wifisetup.html"));
  });
  
  server.on("/style.css", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/css", loadFromSPIFFS("/style.css"));
  });
  
  // Route to handle WiFi credentials
  server.on("/connect", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    if (ssid.length() > 0) {
      // Save credentials to SPIFFS
      writeFile(SPIFFS, "/ssid.txt", ssid.c_str());
      writeFile(SPIFFS, "/password.txt", password.c_str());
      
      server.send(200, "text/plain", "WiFi credentials saved. ESP32 will restart and connect to the network.");
      
      // Restart ESP32
      delay(2000);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "Invalid parameters");
    }
  });
  
  // Start server
  server.begin();
  Serial.println("HTTP server started in AP mode");
}

void setupStreamingServer() {
  // Route for streaming page
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loadFromSPIFFS("/streaming.html"));
  });
  
  server.on("/style.css", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/css", loadFromSPIFFS("/style.css"));
  });
  
  // Route for streaming
  server.on("/stream", HTTP_GET, handleStream);
  
  // Route to toggle flash LED
  server.on("/flash", HTTP_GET, []() {
    isFlashOn = !isFlashOn;
    digitalWrite(FLASH_LED_PIN, isFlashOn ? HIGH : LOW);
    server.send(200, "text/plain", isFlashOn ? "Flash ON" : "Flash OFF");
  });
  
  // Route to disconnect from WiFi
  server.on("/disconnect", HTTP_GET, []() {
    server.send(200, "text/plain", "Disconnecting from WiFi and restarting...");
    
    // Clear stored credentials
    writeFile(SPIFFS, "/ssid.txt", "");
    writeFile(SPIFFS, "/password.txt", "");
    
    // Restart ESP32
    delay(1000);
    ESP.restart();
  });
  
  // Start server
  server.begin();
  Serial.println("HTTP server started in Station mode");
}

void handleStream() {
  WiFiClient client = server.client();
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(response);
  
  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      delay(1000);
      continue;
    }
    
    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    client.print(response);
    
    client.write((char *)fb->buf, fb->len);
    client.print("\r\n");
    
    esp_camera_fb_return(fb);
  }
}

String loadFromSPIFFS(String path) {
  String result = "";
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return result;
  }
  
  while (file.available()) {
    result += (char)file.read();
  }
  
  file.close();
  return result;
}

String readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}
