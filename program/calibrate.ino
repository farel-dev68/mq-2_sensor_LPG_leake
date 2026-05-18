#define MQ6_PIN A0
#define RL 10.0       // kΩ, sesuaikan dengan resistor pada modul
#define SAMPLE_COUNT 60

void setup() {
  Serial.begin(9600);
  Serial.println("=== Kalibrasi MQ-6 ===");
  Serial.println("Pastikan udara bersih. Mengukur 60 detik...");

  float total = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    float adc  = analogRead(MQ6_PIN);
    float volt = adc * (5.0 / 1023.0);
    float rs   = ((5.0 - volt) / volt) * RL;
    total += rs;

    Serial.print("Sample "); Serial.print(i+1);
    Serial.print(" | ADC: "); Serial.print(adc);
    Serial.print(" | Rs: "); Serial.println(rs);
    delay(1000);
  }

  float R0 = total / SAMPLE_COUNT;
  Serial.println("========================");
  Serial.print(">>> R0 (udara bersih) = ");
  Serial.println(R0);
  Serial.println("Catat nilai R0 ini untuk kode utama!");
}

void loop() {}