#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266Ping.h>

/*
连接wifi，然后与本网内的一台udp_server发送UDP数据，接受命令
*/
const char ssid[] = "HUAWEI";
const char password[] = "12345678";

WiFiUDP udp;

const char hostname[] = "lulu";
const int remoteUDPPort = 6688;
unsigned int localUDPPort = 2333;

void setup() {
   Serial.begin(115200);
   pinMode(LED_BUILTIN,OUTPUT);
   delay(100);
   Serial.print("connecting to WIFI...");
   Serial.println(ssid);

   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid,password);

   while(WiFi.status() != WL_CONNECTED){
      delay(1000);
      Serial.println("connecting...");
   }

   Serial.println("WiFi connected!");
   Serial.print("Client IP:");
   Serial.print(WiFi.localIP());

   udp.begin(localUDPPort);
}

// the loop function runs over and over again forever
void loop() {
  Serial.println(Ping.ping(hostname));
  if(Ping.ping(hostname)){
    Serial.println("ping success");
   }
   delay(1000);
   int packetSize = udp.parsePacket();

   if(packetSize){
      char buf[packetSize];
      udp.read(buf,packetSize);
      Serial.println(buf);

      udp.beginPacket(hostname,udp.remotePort());
      udp.write(buf,packetSize);
      udp.endPacket();  

   }

}
