#include <Ps3Controller.h>
#include <ESP32Servo.h>

// -------------------- L298N --------------------
// ===== PIN SETUP DRIVER 1 (BELAKANG) =====
#define ENA1 33
#define IN1 253
#define IN2 26
#define IN3 27
#define IN4 14
#define ENB1 12

// ===== PIN SETUP DRIVER 2 (DEPAN) =====
#define ENA2 0     // ganti dari 0 biar aman boot
#define IN5 4
#define IN6 16
#define IN7 17
#define IN8 5
#define ENB2 18

int speed = 255;

void setup() {
  Serial.begin(115200);

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

}

void loop() {
    digitalWrite(IN5, HIGH);
    digitalWrite(IN6, LOW);
    analogWrite(ENA2, speed);

    digitalWrite(IN7, HIGH);
    digitalWrite(IN8, LOW);
    analogWrite(ENB2, speed);
}