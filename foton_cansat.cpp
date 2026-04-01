#include <Wire.h>
#include <TinyGPSPlus.h>
#include <SPI.h>
#include <LoRa.h>
#include "FS.h"
#include "SD_MMC.h"
#include "esp_camera.h"

// ================= 1. PINLAR SOZLAMASI =================
#define RXD1 41  
#define TXD1 2   
#define MQ135_PIN 1
#define SERVO_PIN 3
#define I2C_SDA 47
#define I2C_SCL 48
const int MPU_ADDR = 0x68;
#define SCK 35
#define MISO 36
#define MOSI 37
#define SS 14
#define RST 42
#define DIO0 21

// Kamera (Freenove)
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  15
#define SIOD_GPIO_NUM  4
#define SIOC_GPIO_NUM  5
#define Y9_GPIO_NUM    16
#define Y8_GPIO_NUM    17
#define Y7_GPIO_NUM    18
#define Y6_GPIO_NUM    12
#define Y5_GPIO_NUM    10
#define Y4_GPIO_NUM    8
#define Y3_GPIO_NUM    9
#define Y2_GPIO_NUM    11
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM  7
#define PCLK_GPIO_NUM  13

TinyGPSPlus gps;
unsigned long lastSendTime = 0;
int packetCount = 1;

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM; config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM; config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM; config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM; config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;         
  config.frame_size = FRAMESIZE_VGA;      
  config.pixel_format = PIXFORMAT_JPEG;   
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_DRAM; 
  config.jpeg_quality = 12;               
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ XATO: Kamera ishga tushmadi!");
  } else {
    Serial.println("📸 Kamera FAOL!");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n🚀 Koinot Kemasi - TO'LIQ EKOTIZIM ISHGA TUSHMOQDA...");

  pinMode(MQ135_PIN, INPUT); 

  pinMode(38, PULLUP); pinMode(39, PULLUP); pinMode(40, PULLUP);
  SD_MMC.setPins(39, 38, 40);
  if (!SD_MMC.begin("/sdcard", true, true)) {
    Serial.println("❌ SD Karta xatosi!");
  } else {
    Serial.println("✅ SD Karta FAOL!");
    SD_MMC.mkdir("/parvoz_data"); 
    File file = SD_MMC.open("/parvoz_data/telemetry.csv", FILE_WRITE);
    if (file) { 
      // 💡 XARITA LINKI UCHUN USTUN QO'SHILDI
      file.println("Paket,Vaqt,Kenglik,Uzunlik,Balandlik,AccX,AccY,AccZ,Gaz_PPM,Xarita_Link,Holat"); 
      file.close(); 
    }
  }

  delay(1000); 
  initCamera();
  
  delay(1000);
  Serial1.setRxBufferSize(1024); 
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1); 
  Serial.println("🛰️ GPS FAOL!");
  
  delay(1000);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x6B); Wire.write(0); Wire.endTransmission(true);
  Serial.println("⚖️ MPU Muvozanat FAOL!");
  
  delay(1000);
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("❌ LoRa xatosi!");
  } else {
    LoRa.setTxPower(20); 
    Serial.println("📡 LoRa FAOL!");
  }
  
  Serial.println("\n✅ TIZIM TAYYOR! PARVOZ BOSHLANMOQDA!\n");
}

void loop() {
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  if (millis() - lastSendTime > 1000) {
    lastSendTime = millis();
    String sysStatus = "SYS_OK"; 
    String camStatus = "Saqlandi";
    String sdStatus = "Yozildi";

    int mq135_val = analogRead(MQ135_PIN);

    Wire.beginTransmission(MPU_ADDR); Wire.write(0x3B); Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6, true);
    float aX = 0, aY = 0, aZ = 0;
    if (Wire.available() >= 6) {
      aX = (Wire.read()<<8 | Wire.read()) / 16384.0;
      aY = (Wire.read()<<8 | Wire.read()) / 16384.0;
      aZ = (Wire.read()<<8 | Wire.read()) / 16384.0;
    }

    camera_fb_t * fb = esp_camera_fb_get();










    
// ----------------------------------------------------------------------------------
String imgPath = "/parvoz_data/img_" + String(packetCount) + ".jpg"; 
    
    while (Serial1.available() > 0) {
      gps.encode(Serial1.read());
    }

    if (fb) {
      File file = SD_MMC.open(imgPath.c_str(), FILE_WRITE);
      if (file) {
        file.write(fb->buf, fb->len);
        file.close();
      } else {
        sysStatus = "SD_ERR"; camStatus = "XATO";
      }
      esp_camera_fb_return(fb); 
    } else {
      sysStatus = "CAM_ERR"; camStatus = "XATO";
    }

    while (Serial1.available() > 0) {
      gps.encode(Serial1.read());
    }

    // MANZIL VA XARITA LINKINI OLISH
    String timeStr = "Kutilyapti";
    if (gps.time.isValid()) {
      char tBuffer[10];
      sprintf(tBuffer, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
      timeStr = String(tBuffer);
    }
    String lat = gps.location.isValid() ? String(gps.location.lat(), 6) : "0.0";
    String lng = gps.location.isValid() ? String(gps.location.lng(), 6) : "0.0";
    String alt = gps.altitude.isValid() ? String(gps.altitude.meters(), 1) : "0.0";

    // 💡 GOOGLE MAPS LINKINI YARATISH
    String mapLink = "https://www.google.com/maps?q=" + lat + "," + lng;

    // 💡 TELEMETRIYAGA LINK QO'SHILDI
    String telemetry = String(packetCount) + "," + timeStr + "," + lat + "," + lng + "," + alt + "," + String(aX, 2) + "," + String(aY, 2) + "," + String(aZ, 2) + "," + String(mq135_val) + "," + mapLink + "," + sysStatus;
    
    File csvFile = SD_MMC.open("/parvoz_data/telemetry.csv", FILE_APPEND);
    if (csvFile) { 
      csvFile.println(telemetry); 
      csvFile.close(); 
    } else {
      sysStatus = "SD_ERR"; sdStatus = "XATO";
    }

    LoRa.beginPacket();
    LoRa.print(telemetry);
    LoRa.endPacket();

    // 🖥️ BOSH QARUV PANELI (DASHBOARD)
    Serial.println("\n==========================================");
    Serial.print("🚀 UZCANSAT - PAKET "); Serial.println(packetCount);
    Serial.println("==========================================");
    Serial.print("⏱️ Vaqt:       "); Serial.println(timeStr);
    Serial.print("📍 GPS:        Lat: "); Serial.print(lat); Serial.print(" | Lng: "); Serial.println(lng);
    Serial.print("   Balandlik:  "); Serial.print(alt); Serial.println(" m");
    Serial.print("🗺️ Xarita:     "); Serial.println(mapLink); // Xarita linki terminalga chiqadi!
    Serial.print("⚖️ MPU:        X: "); Serial.print(aX, 2); Serial.print(" | Y: "); Serial.print(aY, 2); Serial.print(" | Z: "); Serial.println(aZ, 2);
    Serial.print("☁️ Havo(Gaz):  "); Serial.print(mq135_val); Serial.println(" PPM");
    Serial.println("------------------------------------------");
    Serial.print("📸 Kamera:     "); Serial.print(imgPath); Serial.print(" ("); Serial.print(camStatus); Serial.println(")");
    Serial.print("💾 Qora Quti:  telemetry.csv ("); Serial.print(sdStatus); Serial.println(")");
    Serial.print("📡 LoRa:       Uzatildi ("); Serial.print(sysStatus); Serial.println(")");
    Serial.println("==========================================");

    packetCount++;
  }
}