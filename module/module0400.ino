/*
 * 智能门禁模块module0400
 * 通过ESP32-CAM摄像头进行人脸拍照
 * 1.ESP32-CAM长期处于低功耗状态
 * 2.当访客到来时按动按钮，触发GPIOx针脚得LOW低电平信号唤醒ESP32-CAM
 * 3.初始化完之后按1张/秒得速度连续拍摄3张高清照片
 * 4.照片存储在SD卡中
 * 5.ESP32-CAM 通过TCP与树莓派建立连接上传照片，（接下来由中心端完成后续工作）
 * 6.照片上传完毕后，关闭TCP连接
 * 7.模块重新进入低功耗状态等待唤醒
 */
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  //Serial.println();

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

  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
//连续拍摄三张照片
  for(int i = 1; i <= 3; i++){
      delay(1000);
      camera_fb_t * fb = NULL;
     // Take Picture with Camera
      fb = esp_camera_fb_get();
      if(!fb) {
        Serial.println("Camera capture failed");
        return;
      }

      // Path where new picture will be saved in SD Card
      String path = "/picture" + String(i) +".jpg";

      fs::FS &fs = SD_MMC;
      Serial.printf("Picture file name: %s\n", path.c_str());

      File file = fs.open(path.c_str(), FILE_WRITE);
      if(!file){
        Serial.println("Failed to open file in writing mode");
      }
      else {
        file.write(fb->buf, fb->len); // payload (image), payload length
        Serial.printf("Saved file to path: %s\n", path.c_str());
      }
      file.close();
      esp_camera_fb_return(fb);
  }

  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4 关闭板载闪光灯
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);

  delay(2000);
  Serial.println("Into Deep-Sleep mode now...");
  delay(2000);
  Serial.println("...zzZZ");
  esp_deep_sleep_start();
}

void loop() {

}