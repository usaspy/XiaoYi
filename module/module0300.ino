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
/*
 * 继电器各针脚的默认状态 poweron 通 poweroff 断
 * 继电器低电触发：
 * 当给低电平时，继电器拨片打到常开端,负载电路断电 poweroff
 * 当无控制信号或给高点平时，负载电路接通，poweron（负载电路接在继电器的常闭端，默认是接通的）
 */

const int POWER_ON = HIGH;
const int POWER_OFF = LOW;

int SOCKET_1_STATUS = POWER_ON;
int SOCKET_2_STATUS = POWER_ON;
int SOCKET_3_STATUS = POWER_ON;
int SOCKET_4_STATUS = POWER_ON;

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

  //继电器不控制状态or低电平信号时，拨片处于常闭端
  set_socket_status(SOCKET_1_PIN,POWER_ON);//初始设置为通(低电平)
  set_socket_status(SOCKET_2_PIN,POWER_ON);
  set_socket_status(SOCKET_3_PIN,POWER_ON);
  set_socket_status(SOCKET_4_PIN,POWER_ON);
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
  active_packet.replace("status", "(" + String(SOCKET_1_STATUS) + "," + String(SOCKET_2_STATUS) + "," + String(SOCKET_3_STATUS) + "," + String(SOCKET_4_STATUS) + ")");
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

      //配置报文SETUP
      if(String(buf).indexOf("SETUP") != -1) {
        //重新设置onoff
        onoff = split(String(buf), '|', 4).toInt();
        //无论是打开还是关闭，4个插头都初始化为poweron
        SOCKET_1_STATUS = POWER_ON;
        SOCKET_2_STATUS = POWER_ON;
        SOCKET_3_STATUS = POWER_ON;
        SOCKET_4_STATUS = POWER_ON;
        set_socket_status(SOCKET_1_PIN,POWER_ON);
        set_socket_status(SOCKET_2_PIN,POWER_ON);
        set_socket_status(SOCKET_3_PIN,POWER_ON);
        set_socket_status(SOCKET_4_PIN,POWER_ON);

        Serial.println("SETUP ... OK");

        return;
      }
      //命令报文CMD
      if(String(buf).indexOf("CMD") != -1 && onoff == 1) {
        String STATUS_ALL = split(String(buf), '|', 5);

        if(split(String(STATUS_ALL), ',', 0) != ""){
            SOCKET_1_STATUS = split(String(STATUS_ALL), ',', 0).toInt();
            set_socket_status(SOCKET_1_PIN,SOCKET_1_STATUS);
        }
        if(split(String(STATUS_ALL), ',', 1) != ""){
            SOCKET_2_STATUS = split(String(STATUS_ALL), ',', 1).toInt();
            set_socket_status(SOCKET_2_PIN,SOCKET_2_STATUS);
        }
        if(split(String(STATUS_ALL), ',', 2) != ""){
            SOCKET_3_STATUS = split(String(STATUS_ALL), ',', 2).toInt();
            set_socket_status(SOCKET_3_PIN,SOCKET_3_STATUS);
        }
        if(split(String(STATUS_ALL), ',', 3) != ""){
            SOCKET_4_STATUS = split(String(STATUS_ALL), ',', 3).toInt();
            set_socket_status(SOCKET_4_PIN,SOCKET_4_STATUS);
        }
        Serial.println("CMD PROCESS ... OK");
        return;
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
void set_socket_status(int pin,int status){
  pinMode(pin, OUTPUT);
  digitalWrite(pin,status);
}