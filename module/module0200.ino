/*
  module0200 入侵监测模块
  功能：当传感器监测到入侵时，通过UDP传送给中心端，中心端下发指令给模块
  只有当两个传感器都触发时才触发入侵事件
  SR501 红外温度传感器
  RCWL-0516 微波雷达
*/
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266Ping.h>

//设备码
String DEVICE_UUID = "003c48c0-e3f6-471a-b698-7506f68455cb";
String DEVICE_TYPE = "0200";

//WIFI配置
const String SSID = "HUAWEI";
const String PASSWORD = "12345678";

//中心UDP服务器配置
const String remoteHost = "LULU";
const unsigned int remoteUDPPort = 9527;
//本地UDP监听端口
const unsigned int localUDPPort = 11677;

//其他配置
int onoff = 0;  //0:停用（默认不发送传感器数据）  1：启用
const unsigned int interval = 300;  //心跳间隔，300秒

//SR501针脚
const int SR501_PIN = D0;

//RCWL_0516微波雷达连接nodeMCU的针脚
const int RCWL_0516_PIN = D1;
const int RCWL_0516_CDS_PIN = D2;
const int CDS_ON = HIGH;
const int CDS_OFF = LOW;

//LED针脚
const int LED_PIN = D3;

WiFiUDP udp;

void setup() {
  Serial.begin(9600);
  //pinMode(LED_BUILTIN, OUTPUT);
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

  //微波雷达初始化-使能
  pinMode(RCWL_0516_CDS_PIN, OUTPUT);
  digitalWrite(RCWL_0516_CDS_PIN,CDS_ON);
}

void loop() {
  /*
   *每300秒发送一次心跳 
   *每1秒轮询一次两个传感器，如果都为1时，则发送告警事件
   */
  for (int i = 1; i <= interval; i++) {
    delay(1000);
    execute_command();  //配置指令将及时生效
    
    if (onoff == 1) {
      int sr501_status = digitalRead(SR501_PIN);
      int rcwl_0516_status = digitalRead(RCWL_0516_PIN);
      
      //如果两个传感器都触发，则发送入侵告警事件
      if(sr501_status && rcwl_0516_status){
        send_alarm();
      }
    }
    if (i == interval) {
      send_heart();  //发送心跳
    }
  }
}


//与远端中心进行两次握手，发送上线SYN数据包，然后接收中心ACK响应，如果没收到ACK响应，则持续发送SYN（每隔30秒）
boolean communication_init() {
  udp.begin(localUDPPort);

  String syn_packet = "deviceuuid|localip|devicetype|doit|onoff|\n";
  syn_packet.replace("deviceuuid", DEVICE_UUID);
  syn_packet.replace("localip", WiFi.localIP().toString());
  syn_packet.replace("devicetype", DEVICE_TYPE);
  syn_packet.replace("doit", "SYN");
  syn_packet.replace("onoff", "0");
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

        Serial.println("UDP_SYN ... OK");
        return true;
      }
    }
  }
  udp.stop();
  Serial.println("UDP_SYN ... TIMEOUT");
  return false;
}

//发送入侵告警事件
void send_alarm() {
  String active_packet = "deviceuuid|localip|devicetype|doit|onoff|alarm|\n";
  active_packet.replace("deviceuuid", DEVICE_UUID);
  active_packet.replace("localip", WiFi.localIP().toString());
  active_packet.replace("devicetype", DEVICE_TYPE);
  active_packet.replace("doit", "ACTIVE");
  active_packet.replace("onoff", String(onoff));
  active_packet.replace("alarm", "1");

  Serial.print(active_packet);

  //发送ACTIVE心跳包 -> 中心
  udp.beginPacket(remoteHost.c_str(), remoteUDPPort);
  udp.write(active_packet.c_str(), active_packet.length());
  udp.endPacket();
}

//发送心跳包
void send_heart() {
  String active_packet = "deviceuuid|localip|devicetype|doit|onoff|\n";
  active_packet.replace("deviceuuid", DEVICE_UUID);
  active_packet.replace("localip", WiFi.localIP().toString());
  active_packet.replace("devicetype", DEVICE_TYPE);
  active_packet.replace("doit", "ACTIVE");
  active_packet.replace("onoff", String(onoff));
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
        //重新设置onoff
        onoff = split(String(buf), '|', 4).toInt();

        if(onoff == 0){
          digitalWrite(RCWL_0516_CDS_PIN,CDS_OFF); //如果onoff=0，则微博雷达失能
        }else{
          digitalWrite(RCWL_0516_CDS_PIN,CDS_ON);  //如果onoff=1，则微博雷达使能
        }
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
