#include "../shared/AutoReset.h"
/*
 * =========================================================================
 * PROGRAM : MAIN MECANUM JOYSTICK PID - RR ABU HERO 2026
 * =========================================================================
 * Fungsi:
 * Menjalankan robot base Mecanum dengan menerima perintah pergerakan
 * langsung dari PC (via Python). Perintah dikirim via komunikasi Serial (USB).
 * 
 * Fitur Tambahan:
 * - PID Controller untuk koreksi arah gerak motor menggunakan MPU6050
 * - Heading lock: robot mempertahankan arah hadap saat bergerak translasi
 * - Koreksi otomatis drift rotasi
 * 
 * Cara Penggunaan:
 * 1. Upload program ini ke mikrokontroler STM32.
 * 2. Colok kabel USB dari STM32 ke PC.
 * 3. Jalankan script 'joystick_teleop.py' di PC.
 * 
 * Keamanan Tambahan (Failsafe Timeout):
 * Terdapat timer keamanan (500ms). Jika robot sedang bergerak lalu koneksi 
 * kabel tercabut atau PC lag, robot akan otomatis melakukan rem mendadak 
 * (Active Brake) sehingga tidak membahayakan sekitar.
 * =========================================================================
 */

#include "../shared/PinMap.h"
#include "../shared/RobotConfig.h"
#include "MPU_handler.h"
#include "pid_controller.h"

HardwareSerial SerialMonitor(PA10, PA9); // RX, TX

float g_cmPerPulse = CM_PER_PULSE_DEFAULT;

// =========================================================================
// MPU6050 & PID CONTROLLER INIT
// =========================================================================
MPU6050Sensor mpu;
PIDController headingPID(2.0, 0.1, 0.5); // Kp, Ki, Kd untuk heading control

bool mpuReady = false;
float targetHeading = 0.0f;  // Target heading (derajat)
bool headingLockEnabled = true; // Enable/disable heading lock

// =========================================================================
// JOYSTICK TELEOP STATE
// =========================================================================
unsigned long lastJoyTime = 0;
bool isJoyActive = false;

// =========================================================================
// ENCODER & MOTOR INIT (Sama seperti Base)
// =========================================================================

void initEncoders() {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
  GPIOC->MODER  &= ~((3U << (6*2)) | (3U << (7*2)));
  GPIOC->MODER  |=  (2U << (6*2)) | (2U << (7*2));
  GPIOC->OSPEEDR |= (3U << (6*2)) | (3U << (7*2));
  GPIOC->PUPDR  &= ~((3U << (6*2)) | (3U << (7*2)));
  GPIOC->PUPDR  |=  (2U << (6*2)) | (2U << (7*2));
  GPIOC->AFR[0] &= ~((0xFU << (6*4)) | (0xFU << (7*4)));
  GPIOC->AFR[0] |=  (3U << (6*4)) | (3U << (7*4));

  TIM8->PSC   = 0;
  TIM8->ARR   = 0xFFFF;
  TIM8->CCMR1 = 0x0101;
  TIM8->CCER  = 0x0000;
  TIM8->SMCR  = 0x0003;
  TIM8->CNT   = 0;
  TIM8->CR1   = 0x0001;

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  GPIOA->MODER  &= ~((3U << (6*2)) | (3U << (7*2)));
  GPIOA->MODER  |=  (2U << (6*2)) | (2U << (7*2));
  GPIOA->OSPEEDR |= (3U << (6*2)) | (3U << (7*2));
  GPIOA->PUPDR  &= ~((3U << (6*2)) | (3U << (7*2)));
  GPIOA->PUPDR  |=  (2U << (6*2)) | (2U << (7*2));
  GPIOA->AFR[0] &= ~((0xFU << (6*4)) | (0xFU << (7*4)));
  GPIOA->AFR[0] |=  (2U << (6*4)) | (2U << (7*4));

  TIM3->PSC   = 0;
  TIM3->ARR   = 0xFFFF;
  TIM3->CCMR1 = 0x0101;
  TIM3->CCER  = 0x0000;
  TIM3->SMCR  = 0x0003;
  TIM3->CNT   = 0;
  TIM3->CR1   = 0x0001;
}

int32_t getEncoderX() { return (int16_t)TIM8->CNT; }
int32_t getEncoderY() { return (int16_t)TIM3->CNT; }
void    resetEncoders() { TIM8->CNT = 0; TIM3->CNT = 0; }

void initMotors() {
  analogWriteResolution(PWM_RESOLUTION_BITS);
  pinMode(PIN_RPWM_1, OUTPUT); pinMode(PIN_LPWM_1, OUTPUT);
  pinMode(PIN_RPWM_2, OUTPUT); pinMode(PIN_LPWM_2, OUTPUT);
  pinMode(PIN_RPWM_3, OUTPUT); pinMode(PIN_LPWM_3, OUTPUT);
  pinMode(PIN_RPWM_4, OUTPUT); pinMode(PIN_LPWM_4, OUTPUT);

  pinMode(PIN_EN_1, OUTPUT); digitalWrite(PIN_EN_1, HIGH);
  pinMode(PIN_EN_2, OUTPUT); digitalWrite(PIN_EN_2, HIGH);
  pinMode(PIN_EN_3, OUTPUT); digitalWrite(PIN_EN_3, HIGH);
  pinMode(PIN_EN_4, OUTPUT); digitalWrite(PIN_EN_4, HIGH);

  analogWriteFrequency(PWM_FREQUENCY_HZ);

  analogWrite(PIN_RPWM_1, 0); analogWrite(PIN_LPWM_1, 0);
  analogWrite(PIN_RPWM_2, 0); analogWrite(PIN_LPWM_2, 0);
  analogWrite(PIN_RPWM_3, 0); analogWrite(PIN_LPWM_3, 0);
  analogWrite(PIN_RPWM_4, 0); analogWrite(PIN_LPWM_4, 0);
}

void setMotorRaw(int pinR, int pinL, int speed) {
  speed = constrain(speed, -PWM_MAX_VALUE, PWM_MAX_VALUE);
  if (speed > 0) {
    analogWrite(pinL, 0);
    analogWrite(pinR, speed);
  } else if (speed < 0) {
    analogWrite(pinR, 0);
    analogWrite(pinL, -speed);
  } else {
    analogWrite(pinR, 0);
    analogWrite(pinL, 0);
  }
}

void stopSemuaMotor() {
  setMotorRaw(PIN_RPWM_1, PIN_LPWM_1, 0);
  setMotorRaw(PIN_RPWM_2, PIN_LPWM_2, 0);
  setMotorRaw(PIN_RPWM_3, PIN_LPWM_3, 0);
  setMotorRaw(PIN_RPWM_4, PIN_LPWM_4, 0);
}

void emergencyStop() {
  stopSemuaMotor();
  digitalWrite(PIN_EN_1, LOW);
  digitalWrite(PIN_EN_2, LOW);
  digitalWrite(PIN_EN_3, LOW);
  digitalWrite(PIN_EN_4, LOW);
  SerialMonitor.println(F("[!!! ESTOP !!!] Semua EN dimatikan. Ketik 'enable' untuk aktifkan kembali."));
}

void enableMotors() {
  digitalWrite(PIN_EN_1, HIGH);
  digitalWrite(PIN_EN_2, HIGH);
  digitalWrite(PIN_EN_3, HIGH);
  digitalWrite(PIN_EN_4, HIGH);
  SerialMonitor.println(F("[OK] Motor driver diaktifkan kembali."));
}

// =========================================================================
// KINEMATIKA INVERSE MECANUM WHEEL (dengan PID Heading Correction)
// =========================================================================
void driveMecanum(float Vx, float Vy, float Wz, float speedScale) {
  speedScale = constrain(speedScale, 0.0f, 1.0f);
  
  // Jika heading lock aktif dan MPU ready, tambahkan koreksi PID
  float correctedWz = Wz;
  if (headingLockEnabled && mpuReady && (abs(Vx) > 0.01f || abs(Vy) > 0.01f)) {
    // Hanya aktifkan heading lock saat ada gerakan translasi (Vx atau Vy)
    // dan tidak ada perintah rotasi manual (Wz ≈ 0)
    if (abs(Wz) < 0.05f) {
      float currentHeading = mpu.getRelativeYaw();
      float headingError = targetHeading - currentHeading;
      
      // Normalisasi error ke range -180 sampai 180 derajat
      while (headingError > 180.0f) headingError -= 360.0f;
      while (headingError < -180.0f) headingError += 360.0f;
      
      // Hitung koreksi PID (output dalam range -1 sampai 1)
      float pidCorrection = headingPID.compute(0, headingError / 180.0f);
      correctedWz = pidCorrection;
      
      // Debug output (opsional, bisa dikomentari untuk performa)
      // SerialMonitor.print("Heading: "); SerialMonitor.print(currentHeading);
      // SerialMonitor.print(" Target: "); SerialMonitor.print(targetHeading);
      // SerialMonitor.print(" Correction: "); SerialMonitor.println(correctedWz);
    } else {
      // Jika ada perintah rotasi manual, update target heading
      targetHeading = mpu.getRelativeYaw();
      headingPID.reset(); // Reset PID untuk menghindari integral windup
    }
  }
  
  // Kinematika mecanum wheel
  float wFL = ( Vx - Vy - LW_SUM * correctedWz);
  float wFR = ( Vx + Vy + LW_SUM * correctedWz);
  float wRL = ( Vx + Vy - LW_SUM * correctedWz);
  float wRR = ( Vx - Vy + LW_SUM * correctedWz);

  float maxVal = max(max(abs(wFL), abs(wFR)), max(abs(wRL), abs(wRR)));
  if (maxVal > 1.0f) {
    wFL /= maxVal; wFR /= maxVal;
    wRL /= maxVal; wRR /= maxVal;
  }

  setMotorRaw(PIN_RPWM_1, PIN_LPWM_1, (int)(wFL * PWM_MAX_VALUE * speedScale));
  setMotorRaw(PIN_RPWM_2, PIN_LPWM_2, (int)(wFR * PWM_MAX_VALUE * speedScale));
  setMotorRaw(PIN_RPWM_3, PIN_LPWM_3, (int)(wRL * PWM_MAX_VALUE * speedScale));
  setMotorRaw(PIN_RPWM_4, PIN_LPWM_4, (int)(wRR * PWM_MAX_VALUE * speedScale));
}

// =========================================================================
// ODOMETRI & STATUS
// =========================================================================
void printStatus() {
  SerialMonitor.println(F("========== STATUS =========="));
  SerialMonitor.print(F("  Enc X (Lateral) : ")); SerialMonitor.println(getEncoderX());
  SerialMonitor.print(F("  Enc Y (Maju)    : ")); SerialMonitor.println(getEncoderY());
  
  if (mpuReady) {
    SerialMonitor.print(F("  MPU Yaw         : ")); SerialMonitor.println(mpu.getRelativeYaw());
    SerialMonitor.print(F("  Target Heading  : ")); SerialMonitor.println(targetHeading);
    SerialMonitor.print(F("  Heading Lock    : ")); SerialMonitor.println(headingLockEnabled ? "ON" : "OFF");
  } else {
    SerialMonitor.println(F("  MPU             : NOT READY"));
  }
  
  SerialMonitor.println(F("============================"));
}

// =========================================================================
// SERIAL COMMAND PARSER
// =========================================================================
void parseSerialCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  // ── Joy Teleop ──
  // Format: joy <Vx> <Vy> <Wz>
  if (cmd.startsWith(F("joy "))) {
    lastJoyTime = millis();
    isJoyActive = true;
    
    // Parse values
    int firstSpace = cmd.indexOf(' ');
    int secondSpace = cmd.indexOf(' ', firstSpace + 1);
    int thirdSpace = cmd.indexOf(' ', secondSpace + 1);
    
    if (secondSpace > 0 && thirdSpace > 0) {
      float vx = cmd.substring(firstSpace + 1, secondSpace).toFloat();
      float vy = cmd.substring(secondSpace + 1, thirdSpace).toFloat();
      float wz = cmd.substring(thirdSpace + 1).toFloat();
      
      // Use max speed for joystick
      driveMecanum(vx, vy, wz, 1.0f);
    }
  }
  // ── PID Tuning Commands ──
  else if (cmd.startsWith(F("pid "))) {
    // Format: pid <Kp> <Ki> <Kd>
    int firstSpace = cmd.indexOf(' ');
    int secondSpace = cmd.indexOf(' ', firstSpace + 1);
    int thirdSpace = cmd.indexOf(' ', secondSpace + 1);
    
    if (secondSpace > 0 && thirdSpace > 0) {
      float kp = cmd.substring(firstSpace + 1, secondSpace).toFloat();
      float ki = cmd.substring(secondSpace + 1, thirdSpace).toFloat();
      float kd = cmd.substring(thirdSpace + 1).toFloat();
      
      headingPID.setGains(kp, ki, kd);
      SerialMonitor.print(F("[PID] Gains updated: Kp=")); SerialMonitor.print(kp);
      SerialMonitor.print(F(" Ki=")); SerialMonitor.print(ki);
      SerialMonitor.print(F(" Kd=")); SerialMonitor.println(kd);
    }
  }
  else if (cmd == F("lock on")) {
    headingLockEnabled = true;
    if (mpuReady) targetHeading = mpu.getRelativeYaw();
    headingPID.reset();
    SerialMonitor.println(F("[OK] Heading lock ENABLED"));
  }
  else if (cmd == F("lock off")) {
    headingLockEnabled = false;
    headingPID.reset();
    SerialMonitor.println(F("[OK] Heading lock DISABLED"));
  }
  else if (cmd == F("reset heading")) {
    if (mpuReady) {
      mpu.resetYaw();
      targetHeading = 0.0f;
      headingPID.reset();
      SerialMonitor.println(F("[OK] Heading direset ke 0"));
    } else {
      SerialMonitor.println(F("[ERROR] MPU tidak ready"));
    }
  }
  // ── Kontrol ──
  else if (cmd == F("stop"))   { stopSemuaMotor(); SerialMonitor.println(F("[STOP] Active brake.")); }
  else if (cmd == F("estop"))  { emergencyStop(); }
  else if (cmd == F("enable")) { enableMotors(); }
  else if (cmd == F("status")) { printStatus(); }
  else if (cmd == F("reset"))  { resetEncoders(); SerialMonitor.println(F("[OK] Encoder direset ke 0.")); }
}

// =========================================================================
// SETUP & LOOP
// =========================================================================
void setup() {
  SerialMonitor.setTx(PIN_SERIAL_TX);
  SerialMonitor.setRx(PIN_SERIAL_RX);
  SerialMonitor.begin(115200);
  delay(1500);

  SerialMonitor.println(F("\n===================================="));
  SerialMonitor.println(F("  RR ABU HERO 2026 — JOYSTICK PID  "));
  SerialMonitor.println(F("  STM32F407VGT6 + MPU6050          "));
  SerialMonitor.println(F("===================================="));

  initMotors();
  initEncoders();
  
  // Inisialisasi MPU6050
  SerialMonitor.print(F("[INIT] Menginisialisasi MPU6050... "));
  if (mpu.begin()) {
    mpuReady = true;
    SerialMonitor.println(F("OK"));
    
    // Set PID output limits (-1.0 sampai 1.0 untuk Wz)
    headingPID.setOutputLimits(-1.0f, 1.0f);
    
    // Reset heading ke 0
    delay(100);
    mpu.update();
    mpu.resetYaw();
    targetHeading = 0.0f;
    
    SerialMonitor.println(F("[OK] Heading PID Controller ready"));
    SerialMonitor.println(F("[INFO] Commands:"));
    SerialMonitor.println(F("  - 'lock on/off'     : Enable/disable heading lock"));
    SerialMonitor.println(F("  - 'reset heading'   : Reset heading ke 0"));
    SerialMonitor.println(F("  - 'pid Kp Ki Kd'    : Tune PID gains"));
  } else {
    SerialMonitor.println(F("FAILED"));
    SerialMonitor.println(F("[WARNING] MPU6050 tidak terdeteksi!"));
    SerialMonitor.println(F("[WARNING] Robot akan berjalan tanpa heading correction"));
    mpuReady = false;
  }
  
  SerialMonitor.println(F("[OK] Menunggu perintah Joystick..."));
}

void loop() {
  checkAutoReset();
  // Update MPU6050 data
  if (mpuReady) {
    mpu.update();
  }
  
  if (SerialMonitor.available() > 0) {
    String cmd = SerialMonitor.readStringUntil('\n');
    parseSerialCommand(cmd);
  }
  
  // Timeout Joystick (Keamanan)
  // Jika joystick aktif, tapi tidak ada perintah lebih dari 500ms -> STOP
  if (isJoyActive && (millis() - lastJoyTime > 500)) {
    stopSemuaMotor();
    isJoyActive = false;
    SerialMonitor.println(F("[WARNING] Koneksi joystick terputus/timeout! Robot dihentikan."));
  }
}
