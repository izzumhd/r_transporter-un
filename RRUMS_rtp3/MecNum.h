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