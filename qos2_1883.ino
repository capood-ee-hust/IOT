#include <WiFi.h>
#include <AsyncMQTT_ESP32.h> 

//Wifi-
const char* ssid = "DESKTOP-8CAC5FU 7464"; 
const char* password = "^63Le203"; 

//MQTT Broker (HiveMQ Public)
const char* mqtt_server = "broker.hivemq.com"; 
const int mqtt_port = 1883;

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

void onWifiConnect(WiFiEvent_t event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(WiFiEvent_t event) {
  Serial.println("Disconnected from Wi-Fi.");
  xTimerStart(wifiReconnectTimer, 0);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  
  // Đăng ký nhận tin 
  mqttClient.subscribe("esp32/client", 2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.print("Message arrived (QoS ");
  Serial.print(properties.qos); // In ra QoS của tin nhắn nhận được
  Serial.print(") [");
  Serial.print(topic);
  Serial.print("]: ");
  
  for (int i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void onMqttPublish(uint16_t packetId) {
  // Hàm này chạy khi Server đã nhận xong - Hoàn tất QoS2
  Serial.println("Publish acknowledged (QoS 2 success). PacketId: " + String(packetId));
}

void setup() {
  Serial.begin(9600);
  
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, [](TimerHandle_t) {
    connectToMqtt();
  });
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, [](TimerHandle_t) {
    connectToWifi();
  });

  WiFi.onEvent(onWifiConnect, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(onWifiDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  mqttClient.setServer(mqtt_server, mqtt_port);
  
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish); 

  connectToWifi();
}

void loop() {
  if (millis() - lastMsg > 5000) {
    if (mqttClient.connected()) {
      value++;
      String message_to_send = String(value);
      
      
      // gửi với cú pháp: publish(topic, qos, retain, payload)
      //"2" là QoS 2
      uint16_t packetIdPub2 = mqttClient.publish("esp32/counter", 2, true, message_to_send.c_str());
      
      Serial.printf("Publishing at QoS 2, packetId: %d \n", packetIdPub2);
    }
    lastMsg = millis();
  }
}
