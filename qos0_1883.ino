#include <WiFi.h>
#include <PubSubClient.h>


// Wifi 
const char* ssid = "DESKTOP-8CAC5FU 7464"; 
const char* password = "^63Le203"; 

// MQTT Broker (HiveMQ Public) 
const char* mqtt_server = "broker.hivemq.com"; 
const int mqtt_port = 1883;                 
const char* mqtt_username = ""; 
const char* mqtt_password = ""; 
//--------------------------------------------------

WiFiClient espClient;           // Client thường
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Tạo ID ngẫu nhiên để server không đá kết nối cũ
    String clientID = "ESP32Client-";
    clientID += String(random(0xffff), HEX);
    
    // Kết nối không cần user/pass
    if (client.connect(clientID.c_str())) { 
      Serial.println("connected");
      // Đăng ký nhận tin nhắn (nếu cần test nhận)
      client.subscribe("esp32/client");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage += (char)payload[i];
  Serial.println("Message arrived [" + String(topic) + "]: " + incommingMessage);
}

void publishMessage(const char* topic, String payload, boolean retained) {
  if (client.publish(topic, payload.c_str(), true))
    Serial.println("Message published [" + String(topic) + "]: " + payload);
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  
  
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

unsigned long timeUpdata = millis();

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Gửi đi sau mỗi 5 giây
  if (millis() - timeUpdata > 5000) {
    value++;
    String message_to_send = String(value);
    
    // Gửi lên topic
    publishMessage("esp32/counter", message_to_send, true);
    
    timeUpdata = millis();
  }
}
