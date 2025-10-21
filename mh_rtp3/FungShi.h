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

  if (Ps3.event.button_down.circle && millis() - lastPressCircle > debounceDelayCircle) {
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
