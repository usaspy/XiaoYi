/*
  module0100 环境监测模块
  功能：定时将传感器数据通过UDP传送给中心端，中心端下发指令给模块
  DHT22 温湿度传感器
  MP-503 空气质量传感器
*/
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266Ping.h>
#define DHTTYPE DHT22

//设备码
String DEVICE_UUID = "66776c7a-c8f4-4143-bfac-b9813fdec4ef";
String DEVICE_TYPE = "0100";

//WIFI配置
const String SSID = "HUAWEI";
const String PASSWORD = "12345678";

//中心UDP服务器配置
const String remoteHost = "LULU";
const unsigned int remoteUDPPort = 9527;
//本地UDP监听端口
const unsigned int localUDPPort = 13130;

//其他配置
int onoff = 0;  //0:停用（默认不发送传感器数据）  1：启用
unsigned int interval = 300;  //udp上报时间间隔（同时也是心跳间隔），默认300秒  0 ~ 65535

//DHT22传感器连接nodeMCU的针脚
const int DHT22_PIN = D2;

//MP503传感器连接nodeMCU的针脚
const int MP503_A_PIN = D0;
const int MP503_B_PIN = D1;

DHT dht(DHT22_PIN, DHTTYPE);
WiFiUDP udp;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(3000);  //等待3秒
  Serial.println("connecting to WIFI..." + SSID);

  WiFi.mode(WIFI_STA);  //设置WIFI模式为STA
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("connecting...");
  }

  Serial.println("WiFi connected!");
  Serial.println("Device's Local IP -> " + WiFi.localIP().toString());

  //Ping远端服务器测试
  if (Ping.ping(remoteHost.c_str())) {
    Serial.println("ping server...SUCCESS");
  } else {
    Serial.println("ping server...FAILED");
  }

  //通信初始化
  do {
    Serial.println("udp communication initialize...");
  } while (!communication_init());

  //dht22传感器初始化
  dht.begin();

  //mp503传感器初始化
  pinMode(MP503_A_PIN,INPUT);
  pinMode(MP503_B_PIN,INPUT);
}

void loop() {
  //上报.....
  //每秒检查一下中心有没有下发配置指令
  //间隔到期时开始发送数据或心跳
  for (int i = 1; i <= interval; i++) {
    delay(1000);
    execute_command();  //配置指令将及时生效

    if (i == interval) {
      if (onoff == 1) { //间隔时间满足，开始发送数据或者发送心跳
        send_data();
      } else {
        send_heart();
      }
    }
  }
}


//与远端中心进行两次握手，发送上线SYN数据包，然后接收中心ACK响应，如果没收到ACK响应，则持续发送SYN（每隔32秒）
boolean communication_init() {
  udp.begin(localUDPPort);

  String syn_packet = "deviceuuid|localip|devicetype|doit|onoff|period|\n";
  syn_packet.replace("deviceuuid", DEVICE_UUID);
  syn_packet.replace("localip", WiFi.localIP().toString());
  syn_packet.replace("devicetype", DEVICE_TYPE);
  syn_packet.replace("doit", "SYN");
  syn_packet.replace("onoff", "0");
  syn_packet.replace("period", "300");
  //Serial.print(packet);

  //发送SYN包 -> 中心
  udp.beginPacket(remoteHost.c_str(), remoteUDPPort);
  udp.write(syn_packet.c_str(), syn_packet.length());
  udp.endPacket();

  //循环等待32秒 等待接收ACK响应 <- 中心
  for (int i = 0; i < 32; i++) {
    delay(1000);
    int packetSize = udp.parsePacket();

    if (packetSize) {
      char buf[packetSize];
      udp.read(buf, packetSize);
      //Serial.println(buf);

      if (String(buf).indexOf("ACK") != -1) {
        //初始化onoff/interval设置
        onoff = split(String(buf), '|', 4).toInt();
        interval = split(String(buf), '|', 5).toInt();

        Serial.println("UDP_SYN ... OK");
        return true;
      }
    }
  }
  udp.stop();
  Serial.println("UDP_SYN ... TIMEOUT");
  return false;
}

//发送数据包
void send_data() {
  String active_packet = "deviceuuid|localip|devicetype|doit|onoff|period|dht22_data|mp503_data|\n";
  active_packet.replace("deviceuuid", DEVICE_UUID);
  active_packet.replace("localip", WiFi.localIP().toString());
  active_packet.replace("devicetype", DEVICE_TYPE);
  active_packet.replace("doit", "ACTIVE");
  active_packet.replace("onoff", String(onoff));
  active_packet.replace("period", String(interval));

//采集DHT22传感器数据
  float h = dht.readHumidity();  //湿度
  float t = dht.readTemperature();  //温度 （摄氏度）
  float f = dht.readTemperature(true);  //温度 (华氏度)

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
  }else{
    float hi = dht.computeHeatIndex(f, h); //温度（体感温度）
    String dht22_data = "(" + String(h) + "," + String(t) + "," + String(f) + "," + String(hi) + ")";
    active_packet.replace("dht22_data", dht22_data);
  }

//采集MP503传感器数据   00:优  01:良  10:中  11:差
  int A = digitalRead(MP503_A_PIN);
  int B = digitalRead(MP503_B_PIN);

  active_packet.replace("mp503_data", "(" + String(A) + "," + String(B) + ")");

  Serial.print(active_packet);

  //发送ACTIVE心跳包 -> 中心
  udp.beginPacket(remoteHost.c_str(), remoteUDPPort);
  udp.write(active_packet.c_str(), active_packet.length());
  udp.endPacket();
}

//发送心跳包
void send_heart() {
  String active_packet = "deviceuuid|localip|devicetype|doit|onoff|period|\n";
  active_packet.replace("deviceuuid", DEVICE_UUID);
  active_packet.replace("localip", WiFi.localIP().toString());
  active_packet.replace("devicetype", DEVICE_TYPE);
  active_packet.replace("doit", "ACTIVE");
  active_packet.replace("onoff", String(onoff));
  active_packet.replace("period", String(interval));
  Serial.print(active_packet);

  //发送ACTIVE心跳包 -> 中心
  udp.beginPacket(remoteHost.c_str(), remoteUDPPort);
  udp.write(active_packet.c_str(), active_packet.length());
  udp.endPacket();
}

//接收并执行指令
void execute_command() {
    //接收中心下发的控制指令 <- 中心
    int packetSize = udp.parsePacket();

    if (packetSize) {
      char buf[packetSize];
      udp.read(buf, packetSize);
      //Serial.println(buf);

      if (String(buf).indexOf("SETUP") != -1) {
        //重新设置onoff和interval
        onoff = split(String(buf), '|', 4).toInt();
        interval = split(String(buf), '|', 5).toInt();

        Serial.println("SETUP ... OK");
      }
    }
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
