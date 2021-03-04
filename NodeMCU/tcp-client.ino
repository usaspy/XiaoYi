#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

/*
连接wifi，然后与本网内的一台tcp_server建立TCP连接，接受命令，发送数据
*/
const char ssid[] = "HUAWEI";
const char password[] = "12345678";

WiFiClient client;

const char host[] = "192.168.43.115";
const int tcpPort = 8867;

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

}

// the loop function runs over and over again forever
void loop() {
   while (!client.connected()){
      delay(100);
      client.connect(host,tcpPort);
      Serial.println("Client connected!");
   }
   client.print(random(1,20));
   delay(2000);
   Serial.println("send ok");
   client.stop();

 /*
   while(client.available()){
      Serial.println(client.available());
      char re = client.read();
      Serial.println(re);
   }
  */

}