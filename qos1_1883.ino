#include <WiFi.h>
#include <AsyncMQTT_ESP32.h> 

// Wifi
const char* ssid = "DESKTOP-8CAC5FU 7464"; 
const char* password = "^63Le203"; 

// MQTT Broker (HiveMQ Public)
const char* mqtt_server = "broker.hivemq.com"; 
const int mqtt_port = 1883;

// Tạo đối tượng AsyncMqttClient
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

int value = 0;
unsigned long lastMsg = 0;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

// Wifi đã kết nối
void onWifiConnect(WiFiEvent_t event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

// Wifi bị mất
void onWifiDisconnect(WiFiEvent_t event) {
  Serial.println("Disconnected from Wi-Fi.");
  xTimerStart(wifiReconnectTimer, 0); // Hẹn giờ kết nối lại
}

// MQTT đã kết nối thành công
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.println("Session present: " + String(sessionPresent));
  
  // Đăng ký nhận tin 
  mqttClient.subscribe("esp32/client", 1);
}

// Khi MQTT bị mất kết nối
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0); // Hẹn giờ kết nối lại
  }
}

// Khi nhận được tin nhắn
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String message = "";
  for (int i = 0; i < len; i++) {
    message += (char)payload[i];
  }
  Serial.println("Message arrived [" + String(topic) + "]: " + message);
}

//  Server xác nhận đã nhận được gói tin(PUBACK)
void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged (QoS 1). PacketId: " + String(packetId));
}

void setup() {
  Serial.begin(9600);
  
  // Tạo timer để tự động kết nối lại nếu mất mạng
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, [](TimerHandle_t) {
    connectToMqtt();
  });
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, [](TimerHandle_t) {
    connectToWifi();
  });

  // Đăng ký các sự kiện Wifi
  WiFi.onEvent(onWifiConnect, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(onWifiDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // Cấu hình MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);

  
  // Hàm callback (sự kiện)
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish); // Hàm này sẽ chạy khi server gửi xác nhận OK

  connectToWifi();
}

void loop() {
 
  // Gửi đi sau mỗi 5 giây
  if (millis() - lastMsg > 5000) {
    if (mqttClient.connected()) {
      value++;
      String message_to_send = String(value);
      
      // Gửi với cú pháp: publish(topic, qos, retain, payload)
      uint16_t packetIdPub1 = mqttClient.publish("esp32/counter", 1, true, message_to_send.c_str());
      
      Serial.printf("Publishing at QoS 1, packetId: %d \n", packetIdPub1);
    }
    lastMsg = millis();
  }
}
