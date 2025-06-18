#define flamePin    2      // D2 untuk sensor api (flame sensor)
#define buzzerPin   3      // D3 untuk buzzer
#define pinGas      A3     // A3 untuk sensor gas MQ-2
#define pinCahaya   A1     // A1 untuk sensor LDR
#define pinLED      4      // D4 untuk LED indikator

#define AMBANG_GAS      250    // Batas gas bahaya
#define AMBANG_CAHAYA   500    // Batas cahaya gelap

void setup() {
  pinMode(flamePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(pinLED, OUTPUT);
  Serial.begin(9600);
}

void field() {
  tone(buzzerPin, 500);
  delay(250);
  noTone(buzzerPin);
  delay(250);
}

void fieldAPI() {
  Serial.println("🔥 Api terdeteksi!");
  tone(buzzerPin, 1000);
  delay(1000);
  noTone(buzzerPin);
}

void loop() {
  int statusApi = digitalRead(flamePin);         // HIGH = api terdeteksi
  int nilaiGas = analogRead(pinGas);             // Baca nilai gas
  int nilaiCahaya = analogRead(pinCahaya);       // Baca nilai cahaya

  // Cek apakah gas melebihi ambang batas
  bool gasBahaya = (nilaiGas > AMBANG_GAS);

  if (gasBahaya) {
    Serial.println("🚨 Gas terdeteksi!");

    // Cek juga apakah api terdeteksi saat gas bahaya
    if (statusApi == HIGH) {
      fieldAPI(); // Langsung eksekusi kondisi api
    }

    // Bunyi buzzer 5x
    for (int i = 0; i < 5; i++) {
      field();
    }

  } else {
    Serial.print("✅ Aman, tidak ada gas. | ");
    noTone(buzzerPin); // Pastikan buzzer mati
  }

  // Jika tidak sedang kondisi gas bahaya, tapi api sendiri terdeteksi
  if (!gasBahaya && statusApi == HIGH) {
    fieldAPI();
  } else if (!gasBahaya && statusApi == LOW) {
    Serial.println("✅ Aman, tidak ada api.");
  }

  // Kontrol LED berdasarkan cahaya
  if (nilaiCahaya > AMBANG_CAHAYA) {
    digitalWrite(pinLED, HIGH); // Gelap → nyalakan LED
  } else {
    digitalWrite(pinLED, LOW);  // Terang → matikan LED
  }

  // Tampilkan info sensor
  Serial.print("Status API: ");
  Serial.print(statusApi);
  Serial.print(" | Gas: ");
  Serial.print(nilaiGas);
  Serial.print(" | Cahaya: ");
  Serial.println(nilaiCahaya);

  delay(500);
}
