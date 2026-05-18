// ============================================================
//  Gas Monitor MQ-6 + Exhaust Fan PWM + LCD 16x2 I2C
//  Pin: MQ-6 = A0 | Fan = D9 (PWM) | LCD = I2C (SDA/SCL)
// ============================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- Konfigurasi Hardware ----------
#define MQ6_PIN     A0
#define FAN_PIN     9       // Harus pin PWM

// ---------- Parameter Sensor ----------
#define RL          10.0    // Resistor beban modul (kΩ) — cek modul Anda
#define SAMPLE_COUNT 60     // Jumlah sampel kalibrasi (1 per detik = 60 detik)

// ---------- Threshold Gas (ppm) ----------
// Sesuaikan setelah kalibrasi jika perlu
#define PPM_SAFE      300   // < 300 ppm  → aman, kipas mati
#define PPM_LOW       700   // 300–699    → waspada, kipas 30%
#define PPM_MED      1500   // 700–1499   → bahaya, kipas 60%
                            // ≥ 1500     → darurat, kipas 100%

// ---------- Nilai PWM Kipas ----------
#define FAN_OFF       0
#define FAN_30PCT    76     // 255 * 0.30 ≈ 76
#define FAN_60PCT   153     // 255 * 0.60 ≈ 153
#define FAN_100PCT  255

// ---------- LCD ----------
// Cek alamat I2C dengan I2C Scanner: biasanya 0x27 atau 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Variabel Global ----------
float R0 = 10.0;        // Akan diisi saat kalibrasi
float ppmValue = 0.0;
int   fanSpeed = 0;
int   condition = 0;

// ============================================================
//  FUNGSI BANTU
// ============================================================

// Hitung Rs dari pembacaan ADC
float getRs(int adcVal) {
  if (adcVal <= 0) adcVal = 1;              // Hindari bagi nol
  float voltage = adcVal * (5.0 / 1023.0);
  return ((5.0 - voltage) / voltage) * RL;
}

// Estimasi ppm dari rasio Rs/R0
// Kurva MQ-6 untuk LPG/Butane (logaritmik)
// ppm = a * (Rs/R0)^b   — konstanta hasil fit datasheet
float calcPPM(float rs_ro_ratio) {
  // Konstanta kurva LPG MQ-6 (approx): a=1000, b=-2.95
  // Semakin kecil rasio → semakin banyak gas
  if (rs_ro_ratio <= 0) return 9999;
  return 1000.0 * pow(rs_ro_ratio, -2.95);
}

// Tampilkan loading bar kalibrasi di LCD
void updateCalibrationLCD(int sample, int total) {
  int progress = map(sample, 0, total, 0, 16);
  lcd.setCursor(0, 0);
  lcd.print("Kalibrasi..    ");
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(i < progress ? '#' : '.');
  }
}

// ============================================================
//  KALIBRASI OTOMATIS — dipanggil sekali di setup()
// ============================================================
void calibrateSensor() {
  Serial.println("==============================");
  Serial.println(" AUTO KALIBRASI MQ-6");
  Serial.println(" Pastikan udara bersih!");
  Serial.println("==============================");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Memanaskan...");
  lcd.setCursor(0, 1);
  lcd.print("Harap tunggu");

  float totalRs = 0.0;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int adcVal = analogRead(MQ6_PIN);
    float rs = getRs(adcVal);
    totalRs += rs;

    // Update LCD tiap 5 sampel
    if (i % 5 == 0) updateCalibrationLCD(i, SAMPLE_COUNT);

    // Log Serial
    Serial.print("Sample ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.print(SAMPLE_COUNT);
    Serial.print(" | ADC: ");
    Serial.print(adcVal);
    Serial.print(" | Rs: ");
    Serial.print(rs, 2);
    Serial.println(" kΩ");

    delay(1000);
  }

  R0 = totalRs / SAMPLE_COUNT;

  Serial.println("------------------------------");
  Serial.print(" R0 terukur = ");
  Serial.print(R0, 2);
  Serial.println(" kΩ");
  Serial.println(" Kalibrasi selesai! Sistem aktif.");
  Serial.println("==============================");

  // Tampilkan hasil di LCD 2 detik
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kalibrasi OK!");
  lcd.setCursor(0, 1);
  lcd.print("R0=");
  lcd.print(R0, 1);
  lcd.print(" kOhm");
  delay(2000);
  lcd.clear();
}

// ============================================================
//  UPDATE LCD — 2 baris: ppm + status kipas
// ============================================================
void updateLCD() {
  // Baris 1: konsentrasi gas
  lcd.setCursor(0, 0);
  lcd.print("Gas:");

  if (ppmValue >= 9999) {
    lcd.print(" ERR   ");           // Sensor error / jenuh
  } else {
    // Format: "Gas:1234.5ppm  "
    lcd.print(ppmValue, 1);
    lcd.print("ppm ");
    // Padding agar bersih
    if (ppmValue < 1000) lcd.print(" ");
    if (ppmValue < 100)  lcd.print(" ");
  }

  // Baris 2: status + kecepatan kipas
  lcd.setCursor(0, 1);
  switch (condition) {
    case 1:
      lcd.print("AMAN   Fan: OFF ");
      break;
    case 2:
      lcd.print("WASPADA Fan:30% ");
      break;
    case 3:
      lcd.print("BAHAYA  Fan:60% ");
      break;
    case 4:
      lcd.print("DARURAT Fan:100%");
      break;
  }
}

// ============================================================
//  SERIAL LOG
// ============================================================
void logSerial() {
  Serial.print("ppm: ");
  Serial.print(ppmValue, 1);
  Serial.print(" | Kondisi: ");
  Serial.print(condition);
  Serial.print(" | Fan PWM: ");
  Serial.print(fanSpeed);
  Serial.print("/255 (");
  Serial.print(map(fanSpeed, 0, 255, 0, 100));
  Serial.println("%)");
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(9600);
  delay(2000); // Waktu untuk membuka Serial Monitor setelah reset

  // Inisialisasi pin
  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, FAN_OFF);

  // Inisialisasi LCD
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" Gas Monitor v1 ");
  lcd.setCursor(0, 1);
  lcd.print("  MQ-6 + Fan    ");
  delay(1500);
  lcd.clear();

  // Jalankan kalibrasi otomatis
  calibrateSensor();
}

// ============================================================
//  LOOP UTAMA
// ============================================================
void loop() {
  // 1. Baca sensor (rata-rata 5 sampel untuk stabilitas)
  long sumADC = 0;
  for (int i = 0; i < 5; i++) {
    sumADC += analogRead(MQ6_PIN);
    delay(10);
  }
  int avgADC = sumADC / 5;

  float rs = getRs(avgADC);
  float ratio = rs / R0;
  ppmValue = calcPPM(ratio);

  // 2. Tentukan kondisi & atur kipas
  if (ppmValue < PPM_SAFE) {
    condition = 1;
    fanSpeed  = FAN_OFF;

  } else if (ppmValue < PPM_LOW) {
    condition = 2;
    fanSpeed  = FAN_30PCT;

  } else if (ppmValue < PPM_MED) {
    condition = 3;
    fanSpeed  = FAN_60PCT;

  } else {
    condition = 4;
    fanSpeed  = FAN_100PCT;
  }

  // 3. Output ke kipas
  analogWrite(FAN_PIN, fanSpeed);

  // 4. Update tampilan
  updateLCD();
  logSerial();

  // 5. Tunggu sebelum pembacaan berikutnya
  delay(1000);
}