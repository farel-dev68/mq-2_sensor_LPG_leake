#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//PIN DEFINITIONS
#define MQ6_PIN A0
#define FAN_PIN 9

//Kalibrasi
#define RL 10.0
#define SAMPLE_COUNT 60

//Variabel global
float R0 = 10.0; 
float ppmValue = 0.0;
int fanSpeed = 0;
int condition = 0;

//LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Fungsi kalibarsi
float getRs(int adcVal) {
  if (adcVal <= 0) adcVal = 1; // Hindari pembagian dengan nol
  float voltage = adcVal * (5.0 / 1023.0);
  return ((5.0 - voltage) / voltage) * RL;
}
//Fungsi konversi ppm
float calcPPM(float rs_ro_ratio) {
  if (rs_ro_ratio <= 0) return 9999; // Hindari nilai negatif
  return 1000.0 * pow(rs_ro_ratio, -2.95); // Kurva MQ-6 untuk LPG/Butane
}

//loading bar kalibrasi di LCD
void updateCalibrationLCD(int sample, int total) {
  int progress = map(sample, 0, total, 0, 16);
  lcd.setCursor(0, 0);
  lcd.print("Kalibrasi....");
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(i < progress ? '#' : '.');
  }
}

//kaliobrasi otomatis
void calibrateSensor() {
  Serial.println(" KALIBRASI MQ-6");

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
    if (i % 5 == 0) updateCalibrationLCD(i, SAMPLE_COUNT);

    //Log Serial
    Serial.print("Sample "); Serial.print(i+1);
    Serial.print(" | ADC: "); Serial.print(adcVal);
    Serial.print(" | Rs: "); Serial.println(rs);
    delay(1000); // Delay antar sampel
  }
 
  R0 = totalRs / SAMPLE_COUNT;
  Serial.println(" R0 (udara bersih) =" + String(R0));
  Serial.println("R0 terukur = " + String(R0) + " kΩ"); 
  Serial.println(" Kalibrasi selesai! Sistem aktif");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kalibrasi Selesai");
  lcd.setCursor(0, 1);
  lcd.print("R0: " + String(R0, 2) + " kOhm");
  delay(3000);
  lcd.clear();
}

//update LCD berdasarkan kondisi
void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Gas:");

  if (ppmValue >= 9999) {
    lcd.print(" ERROR");
  } else {
    lcd.print(ppmValue, 1);
    lcd.print("ppm ");
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

void printCentered(String text, int row, int lcdWidth = 16) {
  int spaces = (lcdWidth - text.length()) / 2;
  lcd.setCursor(spaces, row);
  lcd.print(text);
}

void setup() {
  Serial.begin(9600);
  delay(2000); // Waktu untuk membuka Serial Monitor setelah reset

  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, 0); // Matikan kipas saat startup

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Gas Monitor");
  lcd.setCursor(0, 1);
  lcd.print("  MQ-6 + Fan    ");
  delay(1500);
  lcd.clear();

  calibrateSensor();
}