# Main Mecanum Joystick PID - RR ABU HERO 2026

## Deskripsi
Program ini adalah versi upgrade dari `main_mecanum_joystick` dengan tambahan fitur **PID Controller untuk koreksi arah gerak motor** menggunakan sensor **MPU6050**.

## Fitur Utama

### 1. Heading Lock (Penguncian Arah)
- Robot akan mempertahankan arah hadap (heading) saat bergerak translasi (maju/mundur/samping)
- Secara otomatis mengoreksi drift rotasi yang disebabkan oleh:
  - Perbedaan kecepatan motor
  - Gesekan roda yang tidak merata
  - Slip pada permukaan licin

### 2. PID Controller
- **Kp (Proportional)**: 2.0 - Respons terhadap error saat ini
- **Ki (Integral)**: 0.1 - Koreksi error yang terakumulasi
- **Kd (Derivative)**: 0.5 - Prediksi error di masa depan

Nilai ini bisa di-tune via serial command untuk menyesuaikan dengan karakteristik robot.

### 3. Auto-Failsafe
- Timeout 500ms: Robot otomatis berhenti jika koneksi terputus
- MPU6050 error handling: Robot tetap bisa berjalan tanpa heading correction jika MPU gagal

## Command Serial Baru

### Kontrol Heading Lock
```
lock on          - Aktifkan heading lock (default: ON)
lock off         - Matikan heading lock
reset heading    - Reset heading ke 0 derajat
```

### PID Tuning
```
pid <Kp> <Ki> <Kd>   - Set PID gains
Contoh: pid 2.5 0.15 0.6
```

### Status
```
status           - Tampilkan status encoder, MPU, dan heading
```

### Command Lama (Tetap Ada)
```
joy <Vx> <Vy> <Wz>   - Kontrol joystick (dari Python script)
stop                 - Stop motor (active brake)
estop                - Emergency stop (matikan semua EN)
enable               - Aktifkan kembali motor driver
reset                - Reset encoder ke 0
```

## Cara Kerja PID Heading Correction

1. **Saat Translasi Murni** (Vx atau Vy ≠ 0, Wz ≈ 0):
   - PID aktif mengoreksi heading
   - Target heading = heading terakhir saat rotasi manual
   - Output PID ditambahkan ke Wz untuk koreksi otomatis

2. **Saat Rotasi Manual** (Wz ≠ 0):
   - PID di-reset untuk menghindari integral windup
   - Target heading di-update ke heading saat ini
   - Robot mengikuti perintah rotasi manual

3. **Saat Diam** (Vx = Vy = Wz = 0):
   - PID tidak aktif
   - Motor berhenti total

## Tuning PID

### Jika Robot Terlalu Lambat Mengoreksi:
- Naikkan **Kp** (misal dari 2.0 ke 3.0)
- Contoh: `pid 3.0 0.1 0.5`

### Jika Robot Overshoot (Lewat Target):
- Turunkan **Kp** atau naikkan **Kd**
- Contoh: `pid 1.5 0.1 0.8`

### Jika Robot Drift Perlahan:
- Naikkan **Ki** (misal dari 0.1 ke 0.2)
- Contoh: `pid 2.0 0.2 0.5`

### Jika Robot Bergetar/Oscillate:
- Turunkan **Kp** dan **Ki**
- Contoh: `pid 1.5 0.05 0.5`

## Hardware Requirements

- STM32F407VGT6 (Black Pill)
- MPU6050 IMU (I2C: PB6=SCL, PB7=SDA)
- 4x Motor BTS7960
- 2x Encoder 600PPR

## Library Dependencies

- Wire (I2C)
- MPU6050_6Axis_MotionApps20 (DMP library)

## Catatan Penting

1. **Kalibrasi MPU6050**: Offset gyro dan accelerometer sudah di-set di `MPU_handler.h`. Jika robot drift, perlu kalibrasi ulang.

2. **Heading Lock Default ON**: Saat startup, heading lock langsung aktif. Jika tidak diinginkan, kirim command `lock off`.

3. **Error Normalization**: Error heading dinormalisasi ke range -180° sampai 180° untuk menghindari masalah saat melewati 0°/360°.

4. **Anti-Windup**: PID memiliki anti-windup protection untuk mencegah integral term terlalu besar saat output saturated.

## Testing

1. Upload program ke STM32
2. Buka Serial Monitor (115200 baud)
3. Tunggu pesan "MPU6050... OK"
4. Test heading lock:
   ```
   status              # Cek heading awal
   joy 0.5 0 0         # Gerak maju, perhatikan heading tetap
   reset heading       # Reset ke 0
   lock off            # Test tanpa heading lock
   lock on             # Aktifkan kembali
   ```

## Troubleshooting

### MPU6050 tidak terdeteksi
- Cek koneksi I2C (PB6, PB7)
- Cek pull-up resistor 4.7kΩ di SCL dan SDA
- Cek alamat I2C (default 0x68)

### Robot berputar sendiri saat diam
- Turunkan Ki: `pid 2.0 0.0 0.5`
- Atau matikan heading lock: `lock off`

### Heading drift perlahan
- Kalibrasi ulang MPU6050 offset
- Naikkan Ki: `pid 2.0 0.2 0.5`

### Robot tidak stabil
- Turunkan Kp: `pid 1.0 0.1 0.5`
- Naikkan Kd: `pid 2.0 0.1 1.0`

---

**Author**: RR ABU HERO 2026 Team  
**Version**: 1.0 (PID Heading Control)  
**Date**: 2026
