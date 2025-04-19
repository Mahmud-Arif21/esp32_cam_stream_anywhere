#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H

#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

void streamJpg(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginChunkedResponse(STREAM_CONTENT_TYPE, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
    static size_t currentChunkIndex = 0;
    static size_t jpgLen = 0;
    static uint8_t *jpgBuffer = NULL;
    static bool isNewFrame = true;
    
    if (isNewFrame) {
      // Capture frame
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        return 0;
      }
      
      // Convert to JPEG if not already
      if (fb->format != PIXFORMAT_JPEG) {
        bool jpeg_converted = frame2jpg(fb->buf, fb->len, fb->width, fb->height, PIXFORMAT_RGB888, 80, &jpgBuffer, &jpgLen);
        esp_camera_fb_return(fb);
        if (!jpeg_converted) {
          Serial.println("JPEG conversion failed");
          return 0;
        }
      } else {
        jpgLen = fb->len;
        jpgBuffer = fb->buf;
      }
      
      // Set up header
      size_t headerLen = strlen(STREAM_BOUNDARY) + strlen(STREAM_PART) + 10;
      if (headerLen > maxLen) {
        Serial.println("Buffer too small for headers");
        return 0;
      }
      
      // Write boundary
      size_t len = 0;
      memcpy(buffer, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
      len += strlen(STREAM_BOUNDARY);
      
      // Write header
      size_t contentLen = sprintf((char *)(buffer + len), STREAM_PART, jpgLen);
      len += contentLen;
      
      // Write a chunk of image
      size_t chunkSize = MIN(maxLen - len, jpgLen);
      memcpy(buffer + len, jpgBuffer, chunkSize);
      len += chunkSize;
      
      // Update state
      currentChunkIndex = chunkSize;
      isNewFrame = false;
      
      // Return frame if we're done with it
      if (fb->format != PIXFORMAT_JPEG) {
        free(jpgBuffer);
      } else {
        esp_camera_fb_return(fb);
      }
      
      return len;
    } else {
      // Continue sending chunks of the current frame
      size_t chunkSize = MIN(maxLen, jpgLen - currentChunkIndex);
      if (chunkSize > 0) {
        memcpy(buffer, jpgBuffer + currentChunkIndex, chunkSize);
        currentChunkIndex += chunkSize;
        
        // Check if we've sent the whole frame
        if (currentChunkIndex >= jpgLen) {
          isNewFrame = true;
        }
        
        return chunkSize;
      }
      
      // Finished sending the frame
      isNewFrame = true;
      return 0;
    }
  });
  
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

#endif // SERVER_HANDLER_H
