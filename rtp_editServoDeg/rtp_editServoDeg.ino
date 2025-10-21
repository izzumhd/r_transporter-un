#include <Ps3Controller.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

/* 
  Note to future: 
  program u/ set manual degree servo dengan data tampil di layar/serial monitor.
  05/10/2025 n izzumhdh,
  set pin dan mac address
  94:54:c5:b7:00:9a -> Ps3 stik krtmi
  b0:a7:32:f2:ff:76 -> Ps3 stik yanto

  Triangle = Servo1 +1 angle
  Cross    = Servo1 -1 angle
  Square   = Servo1 +5 angle
  Circle   = Servo1 -5 angle
  
  Up       = Servo2 +1 angle
  Down     = Servo2 -1 angle
  Left     = Servo2 +5 angle
  Right    = Servo2 -5 angle

  end notes.
*/

#define unoPin 13 
#define dosPin 32 

Servo servo1; 
Servo servo2;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int angle1 = 90;
int angle2 = 90;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  
  servo1.attach(unoPin);
  servo2.attach(dosPin);

  lcd.setCursor(0, 0);
  lcd.print("Servo1: ");
  lcd.print(angle1);
  lcd.setCursor(0, 1);
  lcd.print("Servo2: ");
  lcd.print(angle2);

  Ps3.attach(onButtonPress);
  Ps3.begin("94:54:c5:b7:00:9a");

  servo1.write(angle1);
  servo2.write(angle2);
}

void loop() {
}

void onButtonPress() {
  bool updated = false;

  // --- Servo 1: tombol kanan ---
  if (Ps3.event.button_down.triangle) { angle1 += 1; updated = true; }
  if (Ps3.event.button_down.cross)    { angle1 -= 1; updated = true; }
  if (Ps3.event.button_down.square)   { angle1 -= 5; updated = true; }
  if (Ps3.event.button_down.circle)   { angle1 += 5; updated = true; }

  // --- Servo 2: tombol kiri (D-pad) ---
  if (Ps3.event.button_down.up)    { angle2 += 1; updated = true; }
  if (Ps3.event.button_down.down)  { angle2 -= 1; updated = true; }
  if (Ps3.event.button_down.left)  { angle2 -= 5; updated = true; }
  if (Ps3.event.button_down.right) { angle2 += 5; updated = true; }

  angle1 = constrain(angle1, 0, 180);
  angle2 = constrain(angle2, 0, 180);

  if (updated) {
    servo1.write(angle1);
    servo2.write(angle2);
    tampilkanData();
  }
}

void tampilkanData() {
  lcd.setCursor(8, 0);
  lcd.print("    ");
  lcd.setCursor(8, 0);
  lcd.print(angle1);

  lcd.setCursor(8, 1);
  lcd.print("    ");
  lcd.setCursor(8, 1);
  lcd.print(angle2);

  Serial.print("Servo1: ");
  Serial.print(angle1);
  Serial.print("\tServo2: ");
  Serial.println(angle2);
}
