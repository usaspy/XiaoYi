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
#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdio.h>

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

//设备码
char* DEVICE_UUID = "98176c7a-c8f4-5656-bfac-b9813fdec4ef";
char* DEVICE_TYPE = "0400";

//WIFI配置
const char* SSID = "HUAWEI";
const char* PASSWORD = "12345678";

//中心UDP服务器配置
const String remoteHost = "LULU";
const unsigned int remoteUDPPort = 9527;
//本地UDP监听端口
const unsigned int localUDPPort = 13130;

//其他配置
int onoff = 1;  //1：启用(默认值，不使用该配置)
WiFiUDP udp;
WiFiClient client;

//中心服务器文件传输配置
const unsigned int remoteTCPPort = 9528;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  //Serial.println();
  Serial.printf("connecting to WIFI...%s", SSID);

  WiFi.mode(WIFI_STA);  //设置WIFI模式为STA
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("connecting...");
  }

  Serial.println("WiFi connected!");
  Serial.printf("Device's Local IP -> %s", WiFi.localIP().toString());

  //拍照
  take_pictures();

  //发送照片
  //send_pictures();

  delay(2000);
  Serial.println("Into Deep-Sleep mode now...");
  Serial.println("...zZZZ");
  esp_deep_sleep_start();
}

void loop() {

}

boolean send_pictures(){
   client.connect(remoteHost.c_str(),remoteTCPPort);
   while (!client.connected()){
      delay(100);
      Serial.println("Client connecting...");
   }

   Serial.println("Client connected!");

  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return false;
  }

  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return false;
  }

   for(int i=1;i<=3;i++){
      String path = "/picture" + String(i) +".jpg";
      fs::FS &fs = SD_MMC;
      Serial.printf("Picture file name: %s\n", path.c_str());

      File file = fs.open(path.c_str(), FILE_READ);
      if(!file){
        Serial.println("Failed to open file in read mode");
      }
      else {
          client.write(file);
      }
      file.close();
   }

   client.print(random(1,20));
   delay(2000);
   Serial.println("send ok");
   client.stop();

   return true;
}

//向远端中心发送上线SYN数据包
void communication_init() {
  udp.begin(localUDPPort);

  String local_IP = WiFi.localIP().toString();
  int str_len = local_IP.length() + 1;
  char ip_array[str_len];
  local_IP.toCharArray(ip_array,str_len);

  char* syn_packet = "deviceuuid|localip|devicetype|doit|onoff|period|\n";
  str_replace(syn_packet,strlen("deviceuuid"),DEVICE_UUID);
  str_replace(syn_packet,strlen("localip"),ip_array);
  str_replace(syn_packet,strlen("devicetype"),DEVICE_TYPE);
  str_replace(syn_packet,strlen("doit"),"SYN");
  str_replace(syn_packet,strlen("onoff"),"1");
  str_replace(syn_packet,strlen("period"),"0");
  Serial.print(syn_packet);

  //发送SYN包 -> 中心
  udp.beginPacket(remoteHost.c_str(), remoteUDPPort);
  udp.printf(syn_packet);
  udp.endPacket();
}

boolean take_pictures(){
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
    return false;
  }

  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return false;
  }

  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return false;
  }
//连续拍摄三张照片
   client.connect(remoteHost.c_str(),remoteTCPPort);
   while (!client.connected()){
      delay(100);
      Serial.println("Client connecting...");
   }
   Serial.println("Client connected!");

  for(int i = 1; i <= 3; i++){
      delay(1000);
      camera_fb_t * fb = NULL;
     // Take Picture with Camera
      fb = esp_camera_fb_get();
      if(!fb) {
        Serial.println("Camera capture failed");
        return false;
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
        client.write(fb->buf, fb->len);
        Serial.printf("Saved file to path: %s\n", path.c_str());
      }
      file.close();
      esp_camera_fb_return(fb);
  }

  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4 关闭板载闪光灯
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);

  return true;
}

void str_replace(char * cp, int n, char * str)
{
  int lenofstr;
  int i;
  char * tmp;
  lenofstr = strlen(str);
  //str3比str2短，往前移动
  if(lenofstr < n)
  {
    tmp = cp+n;
    while(*tmp)
    {
      *(tmp-(n-lenofstr)) = *tmp; //n-lenofstr是移动的距离
      tmp++;
    }
    *(tmp-(n-lenofstr)) = *tmp; //move '\0'
  }
  else
          //str3比str2长，往后移动
    if(lenofstr > n)
    {
      tmp = cp;
      while(*tmp) tmp++;
      while(tmp>=cp+n)
      {
        *(tmp+(lenofstr-n)) = *tmp;
        tmp--;
      }
    }
  strncpy(cp,str,lenofstr);
}