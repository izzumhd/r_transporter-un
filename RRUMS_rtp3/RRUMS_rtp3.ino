#include "Declarations.h"
#include "MecNum.h"
#include "FungShi.h"

/* 
  Note to future just in case: 
  program u/ 'robot transporter' 2 driver H L298n, 4 Motor(s), 2 Servo(s).
  mecanum wheel movement.
  *disesuaikan dengan rulebook Unnes Technoday 2025

  01/10/2025 n izzumhdh, ESP 32 Dev Module;
  set pin dan mac address
  94:54:c5:b7:00:9a -> Ps3 stik krtmi
  b0:a7:32:f2:ff:76 -> Ps3 stik yanto
  
  -------------| utama |------------------
  Analog kiri Y   = Maju / mundur
  Analog kiri X   = strafe Kanan / kiri
  Analog kanan X  = rotate Kanan / kiri
  L1              = Clamp mode angkat
  R1              = Clamp mode ambil

  ------------| addition |----------------
  Triangle        = Mode Kecepatan 1, 2, 3
  Circle          = Mode Kec rotasi a, b
  Cross           = Indikator (Buzzer/LED/etc jika dipasang)

  ------------| yet to map |--------------
  Square
  Dpad Up
  Dpad Left
  Dpad Right
  Ps3 button
  Ps3 Start
  Ps3 Select
  Analog kanan Y
  
  end notes.
*/

void setup() {
  Serial.begin(115200);
  Wire.begin();

  int pins[] = { IN1, IN2, IN3, IN4, IN5, IN6, IN7, IN8, ENA1, ENB1, ENA2, ENB2 };
  for (int i = 0; i < 12; i++) pinMode(pins[i], OUTPUT);

  pinMode(Buzz1, OUTPUT);
  digitalWrite(Buzz1, LOW);

  unoo.attach(unoo_PIN);
  doss.attach(doss_PIN);
  unoo.write(unoBuka);
  doss.write(dosNaik);

  lcd.init();
  delay(200);
  lcd.backlight();
  lcd.clear();
  beep(3, 200);

  Ps3.attach(notify);
  Ps3.begin("94:54:c5:b7:00:9a");  // stik krtmi
  Serial.println("Ready.");

  lcd.setCursor(0, 0);
  lcd.print(teksWaiting);
}

void loop() {
  bool ps3Connected = Ps3.isConnected();
  tampilLCD(ps3Connected);
  updateBeep();
  indikatorial();

  if (!ps3Connected) {
    delay(50);
    return;
  }

  setGripper();
}
