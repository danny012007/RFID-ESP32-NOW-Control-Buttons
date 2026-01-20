#include <esp_now.h>
#include <WiFi.h>

const int lightPin = 21;

typedef struct struct_message 
{
  bool status = false; // false = off, true = on
  String objectName = "N/A";
} struct_message;

struct_message data;
 
void setup() {
  Serial.begin(115200);
  pinMode(lightPin, OUTPUT);
  
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataReceived));
}
 
void loop() {
  if (data.objectName == "Light 1")
  {
    if (data.status)
      digitalWrite(lightPin, HIGH);
    else
      digitalWrite(lightPin, LOW);
  }
  delay(50);
}

void OnDataReceived(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&data, incomingData, sizeof(data));
  Serial.println("----------------------------------------------------");
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Status: ");
  Serial.println(data.status);
  Serial.print("Object: ");
  Serial.println(data.objectName);  
}