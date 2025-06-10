#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Servo.h>

// RFID RC522
#define RST_PIN 0  // GPIO0 (D3)
#define SS_PIN  4  // GPIO4 (D2)

// Gabungan LED + Buzzer
#define ALERT_PIN 15  // GPIO15 (D8)

// Servo
#define SERVO_PIN 5  // GPIO5 (D1)

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

int blockNum = 2;
byte bufferLen = 18;
byte readBlockData[18];
String card_holder_name;

const String sheet_url = "https://script.google.com/macros/s/AKfycbxvmzn4bMEyU0A-K0U_vzjrU2zQjxvL1pvAqL2zqe0_MmJpzk2RfZBMRBW51atm6mnW/exec?name=";

#define WIFI_SSID     "Sidiq"
#define WIFI_PASSWORD ""

Servo myServo;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("\nConnecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(ALERT_PIN, OUTPUT);
  digitalWrite(ALERT_PIN, LOW); // pastikan mati di awal

  // Servo setup
  myServo.attach(SERVO_PIN);
  myServo.write(0); // posisi awal

  // RFID setup
  SPI.begin();  // D5=SCK, D6=MISO, D7=MOSI
  mfrc522.PCD_Init();
  Serial.println("Scan kartu RFID...");
}

void loop() {
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("\n**Kartu Terdeteksi**");

  if (!ReadDataFromBlock(blockNum, readBlockData)) {
    Serial.println("Gagal membaca data.");
    return;
  }

  String nama = "";
  for (int j = 0; j < 16; j++) {
    if (readBlockData[j] != 0) {
      nama += (char)readBlockData[j];
    }
  }
  nama.trim();

  Serial.print("Data terbaca dari kartu: ");
  Serial.println(nama);

  // Nyalakan buzzer dan LED bareng
  digitalWrite(ALERT_PIN, HIGH);
  delay(300);
  digitalWrite(ALERT_PIN, LOW);

  myServo.write(180);
  delay(5000);
  myServo.write(0);

  // Kirim ke Google Sheet
  if (WiFi.status() == WL_CONNECTED && nama.length() > 0) {
    WiFiClientSecure client;
    client.setInsecure();

    String full_url = sheet_url + nama;
    HTTPClient https;

    Serial.println("Mengirim ke Google Sheet: " + full_url);

    if (https.begin(client, full_url)) {
      int httpCode = https.GET();
      if (httpCode > 0) {
        Serial.printf("Response Code: %d\n", httpCode);
      } else {
        Serial.printf("Gagal kirim: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.println("Koneksi HTTPS gagal.");
    }
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

bool ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Autentikasi gagal: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Gagal membaca blok: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  Serial.println("Blok berhasil dibaca"); 
  return true;
}