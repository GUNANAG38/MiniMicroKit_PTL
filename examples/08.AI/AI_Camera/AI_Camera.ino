#include <WiFi.h>
#include <PubSubClient.h>
#include "MiniMicroKit_PTL.h"

MiniMicroKit_PTL kit;

//รองรับ WiFi 2.4 GHz (มาตรฐาน IEEE 802.11 b/g/n)
const char* ssid     = "WiFi";
const char* password = "0000";

const char* mqtt_server = "broker.emqx.io";
const int   mqtt_port   = 1883;

// กำหนด ID ตามกล่อง
// แก้ไข Topic ให้ตรงกับใน Webapp AI Camera ในส่วน(School ID / Name ID)
const char* mqtt_topic  = "gelectronlab/KS-PT/Group01/status"; 

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReconnectAttempt = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();

  kit.clearDisplay();
  kit.printText("AI Status:", 0, 0, 1);

  // เงื่อนไขที่ 1 (โมเดล 1)
  if (message == "Hongthai") {
      kit.setLED(false, false, true);
      kit.printText("Hongthai", 0, 16, 2);
      kit.printText("Status : OK", 0, 50, 1);
  } 
  // เงื่อนไขที่ 2 (โมเดล 2)
  else if (message == "Red Marker") {
      kit.playSystemSound(SOUND_ERROR);
      kit.setLED(true, false, false);
      kit.printText("Red Marker", 0, 16, 2); 
      kit.printText("Status : Alert", 0, 50, 1);
  }
  // เงื่อนไขที่ 3 (โมเดล 3 : พื้นหลัง)
  else if (message == "None" || message == "Unknown" || message == "Uncertain...") {
      kit.setLED(false, true, false);
      kit.printText("No Object", 0, 16, 2);    
      kit.printText("Please Check", 0, 50, 1);
  }
  // เงื่อนไขที่ 4 (ปุ่ม Test)
  else if (message == "Test_Class") {
      kit.playSystemSound(SOUND_CONNECT);
      kit.setLED(true, true, true);
      kit.printText("Test Pass", 0, 16, 2);
      kit.printText("Connected!", 0, 50, 1);
  }
  // เงื่อนไขที่ 5 กรณีไม่เข้าเงื่อนไขใด(ชื่อไม่ตรงกับโมเดลที่เทรน)
  else {
      kit.setLED(true, true, false); 
      kit.printText("Class Name:", 0, 10, 1);
      kit.printText(message, 0, 16, 2);
      kit.printText("Active", 0, 50, 1);
  }
  kit.displayUpdate();
}

void reconnect() {
  String clientId = "ESP32C3-" + String(random(0xffff), HEX);
  
  Serial.print("Attempting MQTT connection...");
  if (client.connect(clientId.c_str())) {
    Serial.println("connected!");
    client.subscribe(mqtt_topic);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" will retry later.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  kit.begin();
  kit.showDialog("AI System", "Connecting Wi-Fi", 1000);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // ปิดโหมดประหยัดพลังงาน Wi-Fi
  
  // ตั้งค่าระดับพลังงานตามที่ SDK รองรับ
  WiFi.setTxPower(WIFI_POWER_15dBm); 
  
  WiFi.begin(ssid, password);
  
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    if (millis() - wifiStart > 15000) {
      Serial.println("\nWiFi Timeout, resetting stack...");
      WiFi.disconnect(true);
      delay(1000 + random(500, 2000)); // สุ่มหน่วงเวลาไม่ให้ชนกับบอร์ดอื่น
      WiFi.mode(WIFI_STA);
      WiFi.setSleep(false);
      WiFi.setTxPower(WIFI_POWER_15dBm);
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
  
  lastReconnectAttempt = millis();
}

void loop() {
  kit.update();
  
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      reconnect();
    }
  } else {
    client.loop();
  }
}
