#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

// --- THÔNG TIN WIFI ---
const char* ssid     = "DESKTOP-8CAC5FU 7464";      
const char* password = "^63Le203";

// --- THÔNG TIN THINGSBOARD ---
const char* tb_host = "coap.thingsboard.cloud"; 
const char* access_token = "UHa1nB67dkqNQDRU7JlM"; 

WiFiUDP udp;
Coap coap(udp);
IPAddress tb_ip; 

long lastMsg = 0;
int value = 0;

// Sửa hàm callback để in rõ thông tin phản hồi
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("--------------------------------");
  Serial.println("[CoAP] Nhan duoc phan hoi tu Server!");
  
  // In ra mã phản hồi (Ví dụ: 65 nghĩa là 2.01 Created - Thành công)
  Serial.print("[CoAP] Response Code: ");
  char p[5];
  sprintf(p, "%d.%02d", packet.code >> 5, packet.code & 0x1F);
  Serial.println(p); 
  Serial.println("--------------------------------");
}

void setup() {
  Serial.begin(115200);
  
  Serial.print("Dang ket noi Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi da ket noi!");

  Serial.print("Dang tim IP cua server: ");
  Serial.println(tb_host);
  WiFi.hostByName(tb_host, tb_ip); 
  Serial.print("Da tim thay IP ThingsBoard: ");
  Serial.println(tb_ip);
  
  coap.start();
  coap.response(callback_response);
}

void loop() {
  coap.loop(); // Luôn phải có để xử lý gói tin nhận về

  long now = millis();
  if (now - lastMsg > 5000) { 
    lastMsg = now;
    value++;

    // Tạo payload
    String payload = "{\"count\":" + String(value) + ", \"rssi\":" + String(WiFi.RSSI()) + "}";
    Serial.println("Dang gui len ThingsBoard: " + payload);

    // Tạo path
    String path = "api/v1/" + String(access_token) + "/telemetry";
    
    // GỬI BẰNG HÀM SEND THỦ CÔNG
    coap.send(
        tb_ip, 
        5683, 
        path.c_str(), 
        COAP_CON, 
        COAP_POST, 
        NULL, 
        0, 
        (uint8_t*)payload.c_str(), 
        payload.length()
    );
  }
}