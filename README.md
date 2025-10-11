  # DIBACA JIR..
  ## Transporter robot? kinda.
  ### Note to future or else just in case:
  11/10/2025; Bahasa;  
  program u/ 'robot transporter' 2 driver H L298n, 4 Motor(s) 600rpm, 2 Servo(s) untuk gripper, 1 LCD I2C, dan 1 indikator Buzzer.  
  Dengan konsep mecanum wheel, memungkinkan robot bergerak strafe/crabwalk dan gerakan konvensional dasar.  
  ![Konsep Mecanum Wheel](https://www.roboteq.com/images/article-images/frontpage/wheel-rotations.jpg)
  
  *disesuaikan dengan rulebook Unnes Technoday 2025  

  created 01/10/2025 n izzumhdh, use ESP 32 Dev Module;  
  set pin n mac address  
  94:54:c5:b7:00:9a -> Ps3 stik krtmi  
  b0:a7:32:f2:ff:76 -> Ps3 stik yanto  

  ## Penjelasan file
  | Nama file | Keterangan |
  |:----------|:-----------|
  | RRUMS_rtp1 | tanpa mekanisme mecanum wheel, hanya mekanisme konvensional |
  | RRUMS_rtp2 | dengan mekanisme mecanum wheel, auto gripper dan mode kecepatan(update) |
  | rtp_editServoDeg | untuk mencari sudut servo(s) untuk seting gripper closed-lift dan down-open |
  | rtp_easy | hanya untuk tes driver & motor |
  | rtp_mecanuum | gerakan dasar mecanum (maju/mundur, strafe, dan rotate) |

  ## Library  
  [PS3 Controller](https://github.com/jvpernis/esp32-ps3)   
  [ESP32 Servo](https://github.com/madhephaestus/ESP32Servo)  
  [LCD I2C](https://github.com/johnrickman/LiquidCrystal_I2C)  

  ## Kontrol program utama(RRUMS_rtp2)  
  ### gerak utama 
  * Analog kiri Y   = Maju / mundur  
  * Analog kiri X   = strafe Kanan / kiri  
  * Analog kanan X  = rotate Kanan / kiri   
  * L1              = Clamp mode angkat  
  * R1              = Clamp mode ambil  

  ### tambahan
  * Triangle        = Mode Kecepatan 1, 2, 3  
  * Circle          = Mode Kec rotasi a, b  
  * Cross           = Indikator (Buzzer/LED/etc jika dipasang)  

  ### belum ada fungsi 
  * Square  
  * Dpad Up  
  * Dpad Left  
  * Dpad Right  
  * Ps3 Start  
  * Ps3 Select  
  * Analog kanan Y  
    
  end notes.  
