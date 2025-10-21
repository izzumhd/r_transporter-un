#include <Ps3Controller.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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
const int ENA1 = 33;
const int IN1 = 25;
const int IN2 = 26;
const int IN3 = 27;
const int IN4 = 14;
const int ENB1 = 12;

// motor depan
const int ENA2 = 0;
const int IN5 = 4;
const int IN6 = 16;
const int IN7 = 17;
const int IN8 = 5;
const int ENB2 = 18;

// -------------- SERVO(Gripper) ---------------
const int unoo_PIN = 13;
const int doss_PIN = 32;
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
const int Buzz1 = 15;
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
unsigned long lastPressSegitiga = 0;
const unsigned long debounceDelay = 300;
unsigned long lastPressCircle = 0;
const unsigned long debounceDelayCircle = 300;

int pwmSpeed = 0;
int turn = 0;
int maxPWM = 255;
int segitiga = 0;
int bundar = 0;
float rotateFactor = 1.0;
