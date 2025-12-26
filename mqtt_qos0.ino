#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>



const char* ssid = "DESKTOP-8CAC5FU 7464";       //Wifi connect
const char* password = "^63Le203";   //Password

const char* mqtt_server = "26c64ad9d84c42ee82ab857ffeb93150.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "esp32pro1"; //User
const char* mqtt_password = "Dat123456789"; //Password

WiFiClientSecure espClient;
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
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//Kết nối MQTT Broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientID =  "ESPClient-";
    clientID += String(random(0xffff),HEX);
    if (client.connect(clientID.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("esp32/client");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
//Call back Method for Receiving MQTT massage
void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for(int i=0; i<length;i++) incommingMessage += (char)payload[i];
  Serial.println("Massage arived ["+String(topic)+"]"+incommingMessage);
}
//Method for Publishing MQTT Messages
void publishMessage(const char* topic, String payload, boolean retained){
  if(client.publish(topic,payload.c_str(),true))
    Serial.println("Message published ["+String(topic)+"]: "+payload);
}


void setup() {
  Serial.begin(9600); // Đặt Serial Monitor ở 9600 baud
  while(!Serial) delay(1);



  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

unsigned long timeUpdata=millis();

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Gửi đi sau mỗi 5 giây
  if(millis()-timeUpdata>5000){
    
    //  Tăng giá trị biến đếm lên 1
    value++;

    //  Chuyển số đếm (int) thành chuỗi (String)
    String message_to_send = String(value);

    //  Gửi giá trị đếm lên
    
publishMessage("esp32/counter", message_to_send, true);

    //  Đặt lại mốc thời gian
    timeUpdata=millis();
  }
}
