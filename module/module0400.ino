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
#include <stdio.h>
#include <WiFiUDP.h>

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

//中心TCP服务器配置
const char* remoteHost = "LULU";
const unsigned int remoteUDPPort = 9527;
const unsigned int remoteTCPPort = 9528;
//本地UDP监听端口
const unsigned int localUDPPort = 13130;

WiFiUDP udp;
WiFiClient client;
int photos_Num = 3;

const char* PHOTO_FLAG="===PHOTO===";
const char* OVER_FLAG="===OVER===";

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(9600);
  Serial.printf("connecting to WIFI...%s", SSID);
  //连接WIFI
  WiFi.mode(WIFI_STA);  //设置WIFI模式为STA
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("connecting...");
  }
  Serial.println("WiFi connected!");

  //建立UDP连接，发送SYN报文
  do {
    Serial.println("udp communication initialize...");
  } while (!communication_init());

  //建立TCP连接拍照并上传
  take_photos();

  delay(2000);
  Serial.println("Into Deep-Sleep mode now...zZZZ");
  esp_deep_sleep_start();
}

void loop() {

}

//向远端中心发送上线SYN数据包
boolean communication_init() {
  udp.begin(localUDPPort);

  String local_IP = WiFi.localIP().toString();
  int str_len = local_IP.length() + 1;
  char ip_array[str_len];
  local_IP.toCharArray(ip_array,str_len);

  std::string const& cc = std::string(DEVICE_UUID) + "|" + std::string(ip_array) + "|" + std::string(DEVICE_TYPE) + "|SYN|1|3|\n";
  char const *syn_packet = cc.c_str();
  //Serial.println(syn_packet);

  //发送SYN包 -> 中心
  udp.beginPacket(remoteHost, remoteUDPPort);
  udp.printf(syn_packet);
  udp.endPacket();

    //循环等待32秒 等待接收ACK响应 <- 中心
  for (int i = 0; i < 32; i++) {
    delay(500);
    int packetSize = udp.parsePacket();

    if (packetSize) {
      char buf[packetSize];
      udp.read(buf, packetSize);
      //Serial.println(buf);

      if (String(buf).indexOf("ACK") != -1) {
        //初始化onoff/interval设置
        photos_Num = split(String(buf), '|', 5).toInt();
        Serial.println("UDP_SYN ... OK");
        return true;
      }
    }
  }
  udp.stop();
  Serial.println("UDP_SYN ... TIMEOUT");
  return false;
}

//抓拍照片并上传
boolean take_photos(){
  //连接TCP-Server
  client.connect(remoteHost,remoteTCPPort);
  while (!client.connected()){
    delay(1000);
    Serial.println("connecting...");
  }
  Serial.println("TCP-Server connected!");

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
  Serial.printf("count:%d",photos_Num);
//连续拍摄三张照片
  for(int i = 1; i <= photos_Num; i++){
      camera_fb_t * fb = NULL;
     // Take Picture with Camera
      fb = esp_camera_fb_get();
      if(!fb) {
        Serial.println("Camera capture failed");
        return false;
      }else{
        client.write(fb->buf, fb->len);
        client.write(PHOTO_FLAG, strlen(PHOTO_FLAG));  //第N张照片发送完毕标记
        if(i == photos_Num){
            client.write(OVER_FLAG, strlen(OVER_FLAG)); //所有照片发送完毕标记
        }
        Serial.printf("send file: %d\n", i);
      }
      esp_camera_fb_return(fb);
  }
  if (client.available()){
    client.stop();
  }

  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4 关闭板载闪光灯
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);

  return true;
}


//自定义函数  截取字串
String split(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}