#include <SPI.h>
#include <LoRa.h>

#define ss 10
#define rst 9
#define dio0 2

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n🌍 YER USTI STANSIYASI ISHGA TUSHMOQDA...");
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) { 
    Serial.println("❌ XATO: LoRa topilmadi!");
    while (1);
  }
  Serial.println("✅ Qabul qiluvchi tayyor! Signal kutilmoqda...\n");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    // Format: Paket,Vaqt,Kenglik,Uzunlik,Balandlik,AccX,AccY,AccZ,Gaz,Holat
    int c1 = incoming.indexOf(',');
    int c2 = incoming.indexOf(',', c1 + 1);
    int c3 = incoming.indexOf(',', c2 + 1);
    int c4 = incoming.indexOf(',', c3 + 1);
    int c5 = incoming.indexOf(',', c4 + 1);
    int c6 = incoming.indexOf(',', c5 + 1);
    int c7 = incoming.indexOf(',', c6 + 1);
    int c8 = incoming.indexOf(',', c7 + 1);
    int c9 = incoming.indexOf(',', c8 + 1); // Gaz uchun yangi vergul!

    if (c9 > 0) {
      String packet = incoming.substring(0, c1);
      String timeStr = incoming.substring(c1 + 1, c2);
      String lat = incoming.substring(c2 + 1, c3);
      String lng = incoming.substring(c3 + 1, c4);
      String alt = incoming.substring(c4 + 1, c5);
      String aX = incoming.substring(c5 + 1, c6);
      String aY = incoming.substring(c6 + 1, c7);
      String aZ = incoming.substring(c7 + 1, c8);
      String gas = incoming.substring(c8 + 1, c9);
      String statusCode = incoming.substring(c9 + 1);

      Serial.print("\n========== [ QABUL QILINDI: PAKET "); Serial.print(packet); Serial.println(" ] ==========");
      
      if (statusCode == "SYS_OK") {
        Serial.println("✅ Kema tizimlari a'lo! (Rasm va CSV saqlandi)");
      } else {
        Serial.print("⚠️ KEMA XATOSI: "); Serial.println(statusCode);
      }

      Serial.println("📊 TELEMETRIYA MA'LUMOTLARI:");
      Serial.print("   Vaqt: "); Serial.println(timeStr);
      Serial.print("   GPS:  Lat: "); Serial.print(lat); Serial.print(" | Lng: "); Serial.println(lng);
      Serial.print("   Bal:  "); Serial.print(alt); Serial.println(" metr");
      Serial.print("   MPU:  X: "); Serial.print(aX); Serial.print("  Y: "); Serial.print(aY); Serial.print("  Z: "); Serial.println(aZ);
      Serial.print("   ☁️ Havo (MQ135): "); Serial.println(gas);
      
      Serial.print("📶 Signal kuchi (RSSI): "); Serial.print(LoRa.packetRssi()); Serial.println(" dBm");
    }
  }
}