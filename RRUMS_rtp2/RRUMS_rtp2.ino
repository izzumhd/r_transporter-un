#include <Ps3Controller.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

// ------------------- LCD ------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long prevMillisLCD = 0;

//                   1234567890123456    16
String teksStatic = "   RR CIHUYY  +3";
String teksWaiting = "Ngenteni Stik...";
String teksJalan = "Robot Research UMS            ";
int scrollIndex = 0;
int scrollDelay = 200;

// ------------------ L298N ------------------
// motor belakang
#define ENA1 33
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14
#define ENB1 12

// motor depan
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

int titikS1 = 91;
int titikS2 = 71;

const int unoBuka = 114;
const int dosTurun = 70;
const int unoTutup = 138;
const int dosNaik = 145;

bool gripperActive = false;
unsigned long gripperStart = 0;
int gripperStep = 0;

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
unsigned long lastPress = 0;
const unsigned long debounceDelay = 300;
unsigned long lastPressCircle = 0;
const unsigned long debounceDelayCircle = 300;

int pwmSpeed = 0;
int turn = 0;
int maxPWM = 255;
int segitiga = 0;
int bundar = 0;
float rotateFactor = 1.0;

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

void notify() {
  if (!Ps3.isConnected()) return;

  int ly = -Ps3.data.analog.stick.ly;  // maju mundur
  int rx = Ps3.data.analog.stick.rx;   // rotate kiri kanan
  int lx = -Ps3.data.analog.stick.lx;  // strafe kanan kiri

  int fwd = map(ly, -128, 127, -255, 255);
  int str = map(rx, -128, 127, -255, 255);
  int rot = map(lx, -128, 127, -255, 255) * rotateFactor;

  // rumus mecanum wheel
  int pwmFL = fwd + str + rot;  // mtr depan kiri
  int pwmFR = fwd - str - rot;  // mtr depan kanan
  int pwmBL = fwd - str + rot;  // mtr blkg kiri
  int pwmBR = fwd + str - rot;  // mtr blkg kanan

  pwmFL = constrain(pwmFL, -maxPWM, maxPWM);
  pwmFR = constrain(pwmFR, -maxPWM, maxPWM);
  pwmBL = constrain(pwmBL, -maxPWM, maxPWM);
  pwmBR = constrain(pwmBR, -maxPWM, maxPWM);

  // jika kebalik tinggal dibalik. contoh: (IN5, IN6, ..) >> (IN6, IN5, ..) dst.
  setMotor(IN5, IN6, ENA2, pwmFL);  // depan kiri
  setMotor(IN7, IN8, ENB2, pwmFR);  // depan kanan
  setMotor(IN1, IN2, ENA1, pwmBL);  // belakang kiri
  setMotor(IN3, IN4, ENB1, pwmBR);  // belakang kanan

  Serial.printf("FL:%4d  FR:%4d  BL:%4d  BR:%4d\n", pwmFL, pwmFR, pwmBL, pwmBR);
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

void setMotor(int in1, int in2, int en, int pwm) {
  if (pwm > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(en, pwm);
  } else if (pwm < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(en, abs(pwm));
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(en, 0);
  }
}

void setGripper() {

  // ---------------- mod langsung -----------------
  if (Ps3.data.button.r1 && !gripperActive) {
    gripperActive = true;
    gripperStart = millis();
    gripperStep = 0;  // reset state
  }

  if (Ps3.data.button.l1 && !gripperActive) {
    gripperActive = true;
    gripperStart = millis();
    gripperStep = 10;  // step berbeda untuk tutup-naik
  }

  if (!gripperActive) return;

  unsigned long now = millis();

  // Mode R1: Turun → tunggu 0.5s → buka
  if (gripperStep == 0) {
    doss.write(dosTurun);
    gripperStep = 1;
    gripperStart = now;
  } else if (gripperStep == 1 && now - gripperStart >= 500) {
    unoo.write(unoBuka);
    beep(1, 200);
    Serial.printf("| cap open : %d | batang naik : %d |\n", unoBuka, dosTurun);
    gripperActive = false;  // selesai
  }

  // Mode L1: Tutup → tunggu 0.5s → naik
  else if (gripperStep == 10) {
    unoo.write(unoTutup);
    gripperStep = 11;
    gripperStart = now;
  } else if (gripperStep == 11 && now - gripperStart >= 500) {
    doss.write(dosNaik);
    beep(2, 200);
    Serial.printf("| cap tutup: %d | batang turun: %d |\n", unoTutup, dosNaik);
    gripperActive = false;
  }

  // if (Ps3.data.button.r1) {
  //   // turun - jeda 0.5 dtk - grip buka
  //   doss.write(dosTurun);
  //   delay(500);
  //   unoo.write(unoBuka);
  //   beep(2, 200);
  //   Serial.printf("| cap open : %d | batang naik : %d |\n", unoBuka, dosTurun);
  // }
  // if (Ps3.data.button.l1) {
  //   // grip tutup - jeda 0.5 detik - naik
  //   unoo.write(unoTutup);
  //   delay(500);
  //   doss.write(dosNaik);
  //   beep(2, 200);
  //   Serial.printf("| cap tutup: %d | batang turun: %d |\n", unoTutup, dosNaik);
  // }

  // ---------------- Custom Mode ------------------
  if (Ps3.event.button_down.triangle && millis() - lastPressSegitiga > debounceDelay) {
    lastPressSegitiga = millis();

    if (segitiga == 0) {
      maxPWM = 128;
      segitiga = 1;
      teksStatic = "   RR CIHUYY  +1";
      Serial.println("Mode kecepatan minimal");
      beep(1, 200);
    } else if (segitiga == 1) {
      maxPWM = 180;
      segitiga = 2;
      teksStatic = "   RR CIHUYY  +2";
      Serial.println("Mode kecepatan normal");
      beep(2, 200);
    } else if (segitiga == 2) {
      maxPWM = 255;
      segitiga = 0;
      teksStatic = "   RR CIHUYY  +3";
      Serial.println("Mode kecepatan maksimal");
      beep(3, 200);
    }
  }

  if (Ps3.event.button_down.circle && millis() - lastPressCircle > debounceDelay) {
    lastPressCircle = millis();

    if (bundar == 0) {
      rotateFactor = 0.2;
      bundar = 1;
      teksStatic = "   RR CIHUYY  +a";
      beep(1, 150);
      Serial.println("Mode rotate nol koma dua");
    } else if (bundar == 1) {
      rotateFactor = 0.5;
      bundar = 2;
      teksStatic = "   RR CIHUYY  +b";
      beep(2, 150);
      Serial.println("Mode rotate nol koma lima");
    } else if (bundar == 2) {
      rotateFactor = 1.0;
      bundar = 0;
      teksStatic = "   RR CIHUYY  +c";
      beep(3, 150);
      Serial.println("Mode rotate satu koma nol");
    }
  }

  if (Ps3.data.button.square) {
    // yet to map
  }
  // ---------------- manual set(deact) -----------------
  // if (Ps3.data.button.triangle) {
  //   titikS1 = constrain(titikS1 + 1, 90, 150);
  //   unoo.write(titikS1);
  //   Serial.printf("| t buka: %d \n", titikS1);
  // }
  // if (Ps3.data.button.cross) {
  //   titikS1 = constrain(titikS1 - 1, 90, 150);
  //   unoo.write(titikS1);
  //   Serial.printf("| t buka: %d \n", titikS1);
  // }
  // if (Ps3.data.button.circle) {
  //   titikS2 = constrain(titikS2 + 1, 60, 160);
  //   doss.write(titikS2);
  //   Serial.printf("| t tutup: %d \n", titikS2);
  // }
  // if (Ps3.data.button.square) {
  //   titikS2 = constrain(titikS2 - 1, 60, 160);
  //   doss.write(titikS2);
  //   Serial.printf("| t tutup: %d \n", titikS2);
  // }
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
  if (Ps3.data.button.cross) {
    digitalWrite(Buzz1, HIGH);
    Serial.println("Beeeeeeep ");
  } else digitalWrite(Buzz1, LOW);
}
