#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 4     // GPIO4 (SDA / SS pin)
#define RST_PIN 0    // GPIO5 (Reset pin)

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

int blockNum = 2;
byte blockData[16] = { "aidil" };  // Maks 16 byte data
byte readBlockData[18];
byte bufferLen = 18;
MFRC522::StatusCode status;

void setup() {
  Serial.begin(115200);    // Baudrate lebih tinggi untuk ESP
  SPI.begin();             // Gunakan default pin SPI ESP8266 (D5-D6-D7)
  mfrc522.PCD_Init();      // Inisialisasi RC522
  delay(1000);             // Waktu stabilisasi
  Serial.println("Tempelkan kartu RFID...");
  
  // Set default key A (factory setting: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\n== Kartu Terdeteksi ==");
  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print("Jenis kartu: ");
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  Serial.println("\n>> Menulis ke blok...");
  WriteDataToBlock(blockNum, blockData);

  Serial.println("\n>> Membaca dari blok...");
  ReadDataFromBlock(blockNum, readBlockData);

  Serial.print("Isi blok ");
  Serial.print(blockNum);
  Serial.print(" : ");
  for (int j = 0; j < 16; j++) {
    Serial.write(readBlockData[j]);
  }
  Serial.println();

  mfrc522.PICC_HaltA();  // Hentikan komunikasi dengan kartu
  mfrc522.PCD_StopCrypto1();  // Nonaktifkan kripto
  delay(2000);  // Tunggu agar tidak terus-menerus menulis
}

void WriteDataToBlock(int blockNum, byte blockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Gagal autentikasi (tulis): ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Gagal menulis: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("✅ Berhasil menulis ke kartu.");
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Gagal autentikasi (baca): ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Gagal membaca: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("✅ Berhasil membaca dari kartu.");
}
