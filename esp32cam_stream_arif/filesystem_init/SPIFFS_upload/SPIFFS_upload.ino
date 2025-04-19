/**
 * @file SPIFFS_upload.ino
 * @brief Mounts (and formats on first run) the SPIFFS filesystem on ESP32
 *        so that static files in data/ are available to your camera server.
 *
 * Drop your HTML/CSS/JS into data/, then use "Tools → ESP32 Sketch Data Upload"
 * in the Arduino IDE to flash them into SPIFFS. This sketch itself only mounts
 * the filesystem and reports status.
 */

#include <SPIFFS.h>

void setup() {
  Serial.begin(115200);
  delay(100);

  // Mount SPIFFS, formatting if it was never formatted before
  if (!SPIFFS.begin(/*formatOnFail=*/true)) {
    Serial.println("❌ SPIFFS mount failed! Halting.");
    while (true) { delay(1000); }
  }
  Serial.println("✅ SPIFFS mounted successfully.");
  
  // Uncomment the following lines to inspect usage:
  //   FSInfo info;
  //   SPIFFS.info(info);
  //   Serial.printf("Total: %u, Used: %u, Free: %u\n",
  //                 info.totalBytes, info.usedBytes, info.totalBytes - info.usedBytes);
}

void loop() {

}
