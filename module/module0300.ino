/*
  module0300 智能插座
  功能：控制继电器

*/
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266Ping.h>

//设备码
String DEVICE_UUID = "0b647ddf-6692-405f-92c7-547fbb16ecc1";
String DEVICE_TYPE = "0300";

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
const unsigned int interval = 60;  //心跳间隔，60秒

//继电器控制针脚
const int SOCKET_1_PIN = D1;
const int SOCKET_2_PIN = D2;
const int SOCKET_3_PIN = D3;
const int SOCKET_4_PIN = D4;
//继电器各针脚的当前状态 1  通 0 断
String SOCKET_1_STATUS = "poweron";
String SOCKET_2_STATUS = "poweron";
String SOCKET_3_STATUS = "poweron";
String SOCKET_4_STATUS = "poweron";

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

  //继电器针脚全部设置为常闭，通路状态
  set_socket_status(SOCKET_1_PIN,SOCKET_1_STATUS);//初始设置为通(低电平？高电平？)
  set_socket_status(SOCKET_2_PIN,SOCKET_2_STATUS);
  set_socket_status(SOCKET_3_PIN,SOCKET_3_STATUS);
  set_socket_status(SOCKET_4_PIN,SOCKET_4_STATUS);
}

void loop() {
  /*
   *每60秒发送一次心跳 
   *每1秒轮询一次看中心是否有指令下发
   */
  for (int i = 1; i <= interval; i++) {
    delay(997);
    execute_command();  //执行指令或配置变更，将及时生效

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
        //初始化onoff设置
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

//发送心跳包
void send_heart() {
  String active_packet = "deviceuuid|localip|devicetype|doit|onoff|status|\n";
  active_packet.replace("deviceuuid", DEVICE_UUID);
  active_packet.replace("localip", WiFi.localIP().toString());
  active_packet.replace("devicetype", DEVICE_TYPE);
  active_packet.replace("doit", "ACTIVE");
  active_packet.replace("onoff", String(onoff));
  active_packet.replace("status", SOCKET_1_STATUS + "," + SOCKET_2_STATUS + "," + SOCKET_3_STATUS + "," + SOCKET_4_STATUS);
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

      //配置报文
      if(String(buf).indexOf("SETUP") != -1) {
        //重新设置onoff
        onoff = split(String(buf), '|', 4).toInt();
        //4个插头都设置为常通
        SOCKET_1_STATUS = "poweron";
        SOCKET_2_STATUS = "poweron";
        SOCKET_3_STATUS = "poweron";
        SOCKET_4_STATUS = "poweron";
        set_socket_status(SOCKET_1_PIN,SOCKET_1_STATUS);
        set_socket_status(SOCKET_2_PIN,SOCKET_2_STATUS);
        set_socket_status(SOCKET_3_PIN,SOCKET_3_STATUS);
        set_socket_status(SOCKET_4_PIN,SOCKET_4_STATUS);
        
        Serial.println("SETUP ... OK");
      }
      if(String(buf).indexOf("CMD") != -1 && onoff == 1) {
        String STATUS_STR = split(String(buf), '|', 5);
        
        if(split(String(STATUS_STR), ',', 0) != ""){
            SOCKET_1_STATUS = split(String(STATUS_STR), ',', 0);
            set_socket_status(SOCKET_1_PIN,SOCKET_1_STATUS);
        }
        if(split(String(STATUS_STR), ',', 1) != ""){
            SOCKET_2_STATUS = split(String(STATUS_STR), ',', 1);
            set_socket_status(SOCKET_2_PIN,SOCKET_2_STATUS);
        }
        if(split(String(STATUS_STR), ',', 2) != ""){
            SOCKET_3_STATUS = split(String(STATUS_STR), ',', 2);
            set_socket_status(SOCKET_3_PIN,SOCKET_3_STATUS);
        }
        if(split(String(STATUS_STR), ',', 3) != ""){
            SOCKET_4_STATUS = split(String(STATUS_STR), ',', 3);
            set_socket_status(SOCKET_4_PIN,SOCKET_4_STATUS);
        }
        Serial.println("CMD PROCESS ... OK");
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

//设置插座的状态
void set_socket_status(int pin,String status){
  pinMode(pin, OUTPUT);
  if(status == "poweroff"){
    digitalWrite(pin,LOW); 
  }
  if(status == "poweron"){
    digitalWrite(pin,HIGH); 
  }
}
