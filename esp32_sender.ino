#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData1"

// Set REPEAT_CAL to true instead of false to run calibration
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// MUST CHANGE: MAC address of receivers ------------------------------------------------------------------------------------------------------------
uint8_t broadcastAddressOne[] = {0x00, 0x4b, 0x12, 0x3c, 0x31, 0xc8};
uint8_t broadcastAddressTwo[] = {0x00, 0x4b, 0x12, 0x3a, 0xbb, 0x50};

// MUST CHANGE: RFID Card UID -------------------------------------------------------------------------------------------------------------------------
const byte allowedUid[] = {0x33, 0xD9, 0x8E, 0x36};

// Present RFID key to unlock remote
bool RFID_LOCKED = true; 
bool lockScreenDrawn = false;

// RFID
MFRC522DriverPinSimple ss_pin(22);
MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};

// ESP-NOW
typedef struct struct_message 
{
  bool status = false; // false = off, true = on
  String objectName = "N/A";
} struct_message;

esp_now_peer_info_t peerInfo;
 
// TFT
TFT_eSPI tft = TFT_eSPI();

struct ComponentButton
{
  TFT_eSPI_Button btn;
  int16_t x, y, w, h;
  uint16_t outline, fill, textcolor;
  char* label;
  uint8_t textsize;
  char* displayName;
  uint8_t* receiverMac;
  struct_message data; // optional parameter, as it will be generated
};

ComponentButton lightOneButton = {TFT_eSPI_Button(), 110, 70, 80, 40, TFT_BLACK, TFT_RED, TFT_WHITE, "OFF", 2, "Light 1",  broadcastAddressOne};
ComponentButton lightTwoButton = {TFT_eSPI_Button(), 110, 160, 80, 40, TFT_BLACK, TFT_RED, TFT_WHITE, "OFF", 2, "Light 2", broadcastAddressTwo};
ComponentButton motorButton = {TFT_eSPI_Button(), 210, 70, 80, 40, TFT_BLACK, TFT_RED, TFT_WHITE, "OFF", 2, "Motor", broadcastAddressTwo};
ComponentButton motorDirectionButton = {TFT_eSPI_Button(), 210, 160, 80, 40, TFT_BLACK, TFT_RED, TFT_WHITE, "OFF", 2, "Reverse", broadcastAddressTwo};

static bool wasPressed = false;
bool drawnUI = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // RFID
  mfrc522.PCD_Init();

  // ESP-NOW
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));

  peerInfo.channel = 0;  
  peerInfo.encrypt = false;  

    // Peer 1
  memcpy(peerInfo.peer_addr, broadcastAddressOne, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

    // Peer 2
  memcpy(peerInfo.peer_addr, broadcastAddressTwo, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  // TFT
  tft.init();
  tft.setRotation(3);
  touch_calibrate();
  tft.fillScreen(TFT_LIGHTGREY);
}
 
void loop() 
{
  if (RFID_LOCKED)
  {
    if (!lockScreenDrawn)
      drawLockScreen();

    if (!mfrc522.PICC_IsNewCardPresent())
      return;

    if (!mfrc522.PICC_ReadCardSerial())
      return;

    if (mfrc522.uid.size != 4) return;
    for (byte i=0;i<4;i++) 
    {
      if (mfrc522.uid.uidByte[i] != allowedUid[i]) 
        return;  
    }

    RFID_LOCKED = false;
    tft.fillScreen(TFT_LIGHTGREY);

    // deselects MFRC522
    pinMode(22, OUTPUT);
    digitalWrite(22, HIGH);
  }
  else
  {
    if (!drawnUI)
    {
      ComponentDrawButton(lightOneButton);
      ComponentDrawButton(lightTwoButton);
      ComponentDrawButton(motorButton);
      ComponentDrawButton(motorDirectionButton);
      drawnUI = true;
    }
    uint16_t tx = 0, ty = 0; // To store the touch coordinates
    
    bool pressed = tft.getTouch(&tx, &ty);

    if (pressed && !wasPressed)
    {
      if (lightOneButton.btn.contains(tx, ty))
        ComponentUpdateButton(lightOneButton);      

      if (lightTwoButton.btn.contains(tx, ty))
        ComponentUpdateButton(lightTwoButton);   

      if (motorButton.btn.contains(tx, ty))
        ComponentUpdateButton(motorButton); 
      
      if (motorDirectionButton.btn.contains(tx, ty))
        ComponentUpdateButton(motorDirectionButton);
    }
    wasPressed = pressed;
  }

  delay(50);
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  char macStr[18];
  Serial.print("Packet from: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void drawLockScreen()
{
    tft.fillScreen(TFT_LIGHTGREY); 
    tft.setTextColor(TFT_RED, TFT_RED);
    tft.setTextFont(4);
    tft.setTextSize(2);  

    tft.setCursor(tft.width()/7, tft.height()/3);
    tft.print("PRESENT");
    tft.setCursor(tft.width()/7, tft.height()/2);
    tft.print("KEYCHAIN");
    lockScreenDrawn = true;
}

void ComponentDrawButton(ComponentButton &b)
{
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setTextFont(2);
  tft.setTextSize(1);

  tft.setCursor(b.x - (b.w/4), b.y - b.h);
  tft.print(b.displayName);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);      

  b.btn.initButton(&tft, b.x, b.y, b.w, b.h, b.outline, b.fill, b.textcolor, b.label, b.textsize);
  b.btn.drawButton();

  b.data.objectName = String(b.displayName);
}

void ComponentUpdateButton(ComponentButton &b)
{
  if (b.data.status)
  {
    b.btn.initButton(&tft, b.x, b.y, b.w, b.h, b.outline, TFT_RED, b.textcolor, "OFF", b.textsize);
    b.data.status = false;
  }
  else
  {
    b.btn.initButton(&tft, b.x, b.y, b.w, b.h, b.outline, TFT_GREEN, b.textcolor, "ON", b.textsize);
    b.data.status = true;
  }
  
  esp_now_send(b.receiverMac, (uint8_t *) &b.data, sizeof(b.data));
  Serial.println("----------------------------------------------------");
  Serial.print(b.displayName);
  Serial.println(" data sent.");
  
  b.btn.drawButton();
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) 
  {
    if (REPEAT_CAL)
      SPIFFS.remove(CALIBRATION_FILE);
    else
    {
      fs::File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) 
      {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) 
    tft.setTouch(calData); // calibration data valid
  else 
  {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) 
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    fs::File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) 
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}