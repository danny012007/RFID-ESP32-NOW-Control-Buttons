#include <esp_now.h>
#include <WiFi.h>

const int lightPin = 27;
const int motorEnablePin = 5;
const int motorPin1 = 18;
const int motorPin2 = 19;
uint64_t startTime = esp_timer_get_time();

bool motorSpinning = false;
bool motorReverse = false;

typedef struct struct_message 
{
  bool status = false; // false = off, true = on
  String objectName = "N/A";
} struct_message;

struct_message data;

void setup() {
  Serial.begin(115200);
  pinMode(lightPin, OUTPUT);
  pinMode(motorEnablePin, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);  

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataReceived));
}
 
void loop() {
  uint64_t currentTime = esp_timer_get_time();

  if (data.objectName == "Light 2")
  {
    if (data.status)
      digitalWrite(lightPin, HIGH);
    else
      digitalWrite(lightPin, LOW);
  }

  if (data.objectName == "Motor")
  {
    if (data.status)
      motorSpinning = true;  
    else
      motorSpinning = false;
  }

  if (data.objectName == "Reverse")
  {
    if (data.status)
      motorReverse = true;  
    else
      motorReverse = false;
  }

  if (currentTime - startTime >= 50)
  {
    SpinMotor();
    startTime = currentTime;
  }

  delay(50);
}

void SpinMotor()
{
  analogWrite(motorEnablePin, 255);   //data.pwm 

  if (motorSpinning)    
  {
    if (motorReverse)
    {
      digitalWrite(motorPin1, LOW);     
      digitalWrite(motorPin2, HIGH);         
    }
    else
    {
      digitalWrite(motorPin1, HIGH);     
      digitalWrite(motorPin2, LOW);         
    }
   
  }
  else
  {
    digitalWrite(motorPin1, LOW);     
    digitalWrite(motorPin2, LOW);        
  }

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