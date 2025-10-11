#include <Ps3Controller.h>

// ===== PIN SETUP DRIVER 1 (BELAKANG) =====
#define ENA1 33
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14
#define ENB1 12

// ===== PIN SETUP DRIVER 2 (DEPAN) =====
#define ENA2 0
#define IN5 4
#define IN6 16
#define IN7 17
#define IN8 5
#define ENB2 18

// ===== SETUP CONSTANT =====
int maxPWM = 255;

// ======= FUNGSI MOTOR =======
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

// ======= CALLBACK PS3 =======
void notify() {
  if (!Ps3.isConnected()) return;

  // baca analog stick
  int ly = -Ps3.data.analog.stick.ly;  // maju mundur
  int lx = Ps3.data.analog.stick.lx;   // geser kiri kanan
  int rx = Ps3.data.analog.stick.rx;   // rotasi kanan kiri

  // ubah skala dari -128..127 ke -255..255
  int vy = map(ly, -128, 127, -255, 255);
  int vx = map(lx, -128, 127, -255, 255);
  int rot = map(rx, -128, 127, -255, 255);

  // rumus mecanum wheel
  int pwmFL = vy + vx + rot;
  int pwmFR = vy - vx - rot;
  int pwmBL = vy - vx + rot;
  int pwmBR = vy + vx - rot;

  // batasi nilai pwm
  pwmFL = constrain(pwmFL, -maxPWM, maxPWM);
  pwmFR = constrain(pwmFR, -maxPWM, maxPWM);
  pwmBL = constrain(pwmBL, -maxPWM, maxPWM);
  pwmBR = constrain(pwmBR, -maxPWM, maxPWM);

  // kirim ke motor
  setMotor(IN5, IN6, ENA2, pwmFL);  // depan kiri
  setMotor(IN7, IN8, ENB2, pwmFR);  // depan kanan
  setMotor(IN1, IN2, ENA1, pwmBL);  // belakang kiri
  setMotor(IN3, IN4, ENB1, pwmBR);  // belakang kanan

  // debug
  Serial.printf("FL:%4d  FR:%4d  BL:%4d  BR:%4d\n", pwmFL, pwmFR, pwmBL, pwmBR);
}

void onConnect() {
  Serial.println(" PS3 Controller Connected!");
}

void setup() {
  Serial.begin(115200);

  // atur semua pin
  int pins[] = {IN1, IN2, IN3, IN4, IN5, IN6, IN7, IN8, ENA1, ENB1, ENA2, ENB2};
  for (int i = 0; i < 12; i++) pinMode(pins[i], OUTPUT);

  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.begin("94:54:c5:b7:00:9a");

  Serial.println("Menunggu koneksi PS3...");
}

void loop() {
  if (!Ps3.isConnected()) return;
}
