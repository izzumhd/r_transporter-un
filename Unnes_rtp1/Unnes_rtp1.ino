#include <Ps3Controller.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* 
  Note to future just in case: 
  program u/ robot transporter 2 driver H L298n, 4 Motor(s), 2 Servo(s).
  *disesuaikan dengan rulebook Unnes Technoday 2025

  01/10/2025 n izzumhdh,
  set pin dan mac address
  94:54:c5:b7:00:9a -> Ps3 stik krtmi
  b0:a7:32:f2:ff:76 -> Ps3 stik yanto
  
  -------------| utama |------------------
  Analog kiri Y   = Maju / mundur
  Analog kiri X   = Belok Kanan / kiri
  Trigger L2      = Strafe(jalan geser) kiri
  Trigger R2      = Strafe kanan
  L1              = Clamp mode angkat
  R1              = Clamp mode ambil
  Dpad Down       = Indikator (Buzzer/LED/etc jika dipasang)

  ------------| addition |----------------
  Triangle = Servo1 +1 angle
  Cross    = Servo1 -1 angle
  Square   = Servo2 +1 angle
  Circle   = Servo2 -1 angle

  ------------| unmapped |----------------
  Dpad Up
  Dpad Left
  Dpad Right
  Ps3 Start
  Ps3 Select
  Analog kiri X
  Analog kanan Y
  
  end notes.
*/

// ------------------- LCD ------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long prevMillisLCD = 0;

//                   1234567890123456
String teksStatic = "iki jenenge opo ";
String teksWaiting = "Menunggu Stik...";
String teksJalan = "Robot Research UMS            ";
int scrollIndex = 0;
int scrollDelay = 200;

// ------------------ L298N ------------------
#define ENA1 33
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14
#define ENB1 12

#define ENA2 0
#define IN5 4
#define IN6 16
#define IN7 17
#define IN8 5
#define ENB2 18

// ------------------ SERVO ------------------
#define unoo_PIN 13
#define doss_PIN 32
Servo unoo;
Servo doss;

int titikS1 = 90;
int titikS2 = 90;
int unoBuka = 0;
int dosBuka = 0;
int unoTutup = 90;
int dosTutup = 90;

// ---- Buzzer, Led, atau indikator lain -------
#define Buzz1 15
bool beepActive = false;
unsigned long beepPrevMillis = 0;
int beepTotal = 0;
int beepLama = 0;
int beepCount = 0;
bool beepState = false;

void beep(int total, int lama) {
  beepTotal = total;
  beepLama = lama;
  beepCount = 0;
  beepActive = true;
  beepState = false;
  beepPrevMillis = millis();
}

// -------------------- PS3 --------------------
int pwmSpeed = 0;
int turn = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // motor kiri
  pinMode(ENA1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB1, OUTPUT);

  // motor kanan
  pinMode(ENA2, OUTPUT);
  pinMode(IN5, OUTPUT);
  pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT);
  pinMode(IN8, OUTPUT);
  pinMode(ENB2, OUTPUT);

  pinMode(Buzz1, OUTPUT);
  digitalWrite(Buzz1, LOW);

  unoo.attach(unoo_PIN);
  doss.attach(doss_PIN);
  unoo.write(90);
  doss.write(90);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  Ps3.attach(notify);
  Ps3.begin("94:54:c5:b7:00:9a"); // stik krtmi
  Serial.println("Ready.");

  lcd.setCursor(0, 0);
  lcd.print(teksWaiting);
}

void notify() {
  int ly = Ps3.data.analog.stick.ly;  // -128 (atas) s/d 127 (bawah)
  int rx = Ps3.data.analog.stick.rx;  // -128 (kiri) s/d 127 (kanan)

  pwmSpeed = map(-ly, -128, 127, -255, 255);  // maju-mundur
  turn = map(rx, -128, 127, -255, 255);       // belok

  setGripper();
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

  int l2 = Ps3.data.analog.button.l2;  // 0-255
  int r2 = Ps3.data.analog.button.r2;  // 0-255
  int strafe = r2 - l2;                // + ke kanan, - ke kiri

  int leftFront = pwmSpeed + turn + strafe;
  int rightFront = pwmSpeed - turn - strafe;
  int leftBack = pwmSpeed + turn - strafe;
  int rightBack = pwmSpeed - turn + strafe;

  setMotor(ENA1, IN1, IN2, leftFront);
  setMotor(ENB1, IN3, IN4, leftBack);
  setMotor(ENA2, IN5, IN6, rightFront);
  setMotor(ENB2, IN7, IN8, rightBack);
}

void setMotor(int EN, int INa, int INb, int speed) {
  if (speed > 0) {
    digitalWrite(INa, HIGH);
    digitalWrite(INb, LOW);
    analogWrite(EN, constrain(speed, 0, 255));
  } else if (speed < 0) {
    digitalWrite(INa, LOW);
    digitalWrite(INb, HIGH);
    analogWrite(EN, constrain(-speed, 0, 255));
  } else {
    digitalWrite(INa, LOW);
    digitalWrite(INb, LOW);
    analogWrite(EN, 0);
  }
}

void setGripper() {

  // ---------------- mod langsung -----------------
  if (Ps3.data.button.r1) {
    // turun - jeda 1 dtk - grip buka
    doss.write(dosBuka);
    delay(1000);
    unoo.write(unoBuka);
    beep(2, 200);
    Serial.printf("| cap open : %d | batang naik : %d |\n", unoBuka, dosBuka);
  }
  if (Ps3.data.button.l1) {
    // grip tutup - jeda 0.5 detik - naik
    unoo.write(unoTutup);
    delay(500);
    doss.write(dosTutup);
    beep(2, 200);
    Serial.printf("| cap tutup: %d | batang turun: %d |\n", unoTutup, dosTutup);
  }

  // ---------------- manual set -----------------
  if (Ps3.data.button.triangle) {
    titikS1 = constrain(titikS1 + 1, 0, 180);
    unoo.write(titikS1);
    Serial.printf("| t buka: %d \n", titikS1);
  }
  if (Ps3.data.button.cross) {
    titikS1 = constrain(titikS1 - 1, 0, 180);
    unoo.write(titikS1);
    Serial.printf("| t buka: %d \n", titikS1);
  }
  if (Ps3.data.button.circle) {
    titikS2 = constrain(titikS2 + 1, 0, 180);
    doss.write(titikS2);
    Serial.printf("| t tutup: %d \n", titikS2);
  }
  if (Ps3.data.button.square) {
    titikS2 = constrain(titikS2 - 1, 0, 180);
    doss.write(titikS2);
    Serial.printf("| t tutup: %d \n", titikS2);
  }
}

void tampilLCD(bool ps3Connected) {
  unsigned long currentMillis = millis();
  if (currentMillis - prevMillisLCD >= scrollDelay) {
    prevMillisLCD = currentMillis;

    lcd.setCursor(0, 0);
    if (ps3Connected) lcd.print(teksStatic);
    else lcd.print(teksWaiting + "   ");

    int len = teksJalan.length();
    String tampilan = teksJalan.substring(scrollIndex) + " " + teksJalan.substring(0, scrollIndex);
    lcd.setCursor(0, 1);
    lcd.print(tampilan.substring(0, 16));

    scrollIndex++;
    if (scrollIndex >= len) scrollIndex = 0;
  }
}

void updateBeep() {
  if (!beepActive) return;

  unsigned long currentMillis = millis();
  if (currentMillis - beepPrevMillis >= beepLama) {
    beepPrevMillis = currentMillis;
    beepState = !beepState;
    digitalWrite(Buzz1, beepState ? HIGH : LOW);

    if (!beepState) {
      beepCount++;
      if (beepCount >= beepTotal) {
        beepActive = false;
        digitalWrite(Buzz1, LOW);
      }
    }
  }
}

void indikatorial() {
  if (Ps3.event.button_down.down) {
    digitalWrite(Buzz1, HIGH);
    Serial.print("Beeeeeeep");
  } else digitalWrite(Buzz1, LOW);
}
