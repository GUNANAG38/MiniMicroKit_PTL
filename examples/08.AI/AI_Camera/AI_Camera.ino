#include <WiFi.h>
#include <PubSubClient.h>
#include "MiniMicroKit_PTL.h"

MiniMicroKit_PTL kit;

//รองรับ WiFi 2.4 GHz (มาตรฐาน IEEE 802.11 b/g/n)
const char* ssid     = "xxxxxxx";
const char* password = "0000000";

const char* mqtt_server = "broker.emqx.io";
const int   mqtt_port   = 1883;

// แก้ไข Topic ให้ตรงกับใน Webapp AI Camera ในส่วน(School ID / Name ID)
const char* mqtt_topic  = "gelectronlab/PTL-School/group1/status"; 

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();

  kit.clearDisplay();
  kit.printText("AI Status:", 0, 0, 1);

  if (message == "Hongthai") {                // เงื่อนไขที่ 1 
      kit.setLED(false, false, true);         // ไฟแสดงสถานะสีเขียว
      kit.printText("Hongthai", 0, 16, 2);    // แสดงข้อความ Hongthai บนจอ OLED
      kit.printText("Status : OK", 0, 50, 1); // แสดงข้อความ Status : OK บนจอ OLED
  } 
  else if (message == "Red Marker") {             // เงื่อนไขที่ 2
      kit.playSystemSound(SOUND_ERROR);           // เสียงเตือน
      kit.setLED(true, false, false);             // ไฟแสดงสถานะสีแดง
      kit.printText("Red Marker", 0, 16, 2);      // แสดงข้อความ Red Marker บนจอ OLED
      kit.printText("Status : Alert", 0, 50, 1);  // แสดงข้อความ Status : Alert บนจอ OLED
  } 
  else if (message == "None" || message == "Unknown" || message == "Uncertain...") { // เงื่อนไขที่ 3
      kit.setLED(false, true, false);                                                // ไฟแสดงสถานะสีเหลือง
      kit.printText("No Object", 0, 16, 2);                                          // แสดงข้อความ No Object บนจอ OLED  
      kit.printText("Please Check", 0, 50, 1);                                       // แสดงข้อความ Please Check บนจอ OLED  
  }
  else if (message == "Test_Class") {
      kit.playSystemSound(SOUND_CONNECT);
      kit.setLED(true, true, true);
      kit.printText("Test Pass", 0, 16, 2);
      kit.printText("Connected!", 0, 50, 1);
  }
  else {
      kit.setLED(true, true, false); 
      kit.printText("Class Name:", 0, 10, 1);
      kit.printText(message, 0, 16, 2);
      kit.printText("Active", 0, 50, 1);
  }
  kit.displayUpdate();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    String clientId = "ESP32C3-G2-" + String(random(0xffff), HEX);
    
    // เชื่อมต่อพร้อมกำหนด KeepAlive นานขึ้นเพื่อไม่ให้ตัดการเชื่อมต่อไวเกินไป (60 วินาที)
    if (client.connect(clientId.c_str(), NULL, NULL, 0, 0, 0, 0, true)) {
      Serial.println("connected!");
      client.subscribe(mqtt_topic);
      Serial.print("Subscribed to topic: ");
      Serial.println(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  kit.begin();
  kit.showDialog("AI System", "Connecting Wi-Fi", 1000);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // ปิดโหมดประหยัดพลังงาน Wi-Fi ป้องกันอาการหลุด
  WiFi.begin(ssid, password);
  
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    if (millis() - wifiStart > 20000) {
      Serial.println("\nWiFi Timeout, resetting WiFi stack...");
      WiFi.disconnect(true);
      delay(1000);
      WiFi.mode(WIFI_STA);
      WiFi.setSleep(false);
      WiFi.begin(ssid, password);
      wifiStart = millis();
    }
  }

  Serial.println("\nWiFi Connected successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(512);

  kit.clearDisplay();
  kit.printText("Ready for AI", 0, 0, 1);
  kit.printText("Topic Ready", 0, 20, 1);
  kit.displayUpdate();
}

void loop() {
  kit.update();
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
