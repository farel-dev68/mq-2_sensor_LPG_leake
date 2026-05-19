#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Ganti alamat jika perlu (0x27 atau 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  delay(2000);

  Serial.println("Mulai test LCD...");

  lcd.init();          // Lebih aman dari lcd.begin()
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Hello, Dimas!");

  lcd.setCursor(0, 1);
  lcd.print("LCD OK :)");

  Serial.println("LCD berhasil ditampilkan");
}

void loop() {
  // Tidak perlu apa-apa
}