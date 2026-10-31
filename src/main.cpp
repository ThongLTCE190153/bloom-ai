#include <Arduino.h>
#include <ESP32Servo.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ==============================================================================
// CẤU HÌNH PHẦN CỨNG SERVO
// ==============================================================================
// Nếu bạn chỉ cắm 1 servo, hãy đổi NUM_SERVOS = 1 để tránh tiêu hao dòng thừa
const int NUM_SERVOS = 2; 
const int SERVO_PINS[NUM_SERVOS] = {13, 14};

// Dải xung điều khiển MG996R (500us - 2400us hoặc 544us - 2400us)
const int SERVO_MIN_US = 500;
const int SERVO_MAX_US = 2400;

// ==============================================================================
// CẤU HÌNH TỐC ĐỘ VÀ THỜI GIAN
// ==============================================================================
// Thời gian chờ để servo hoàn thành hành trình vật lý với tốc độ tối đa của motor:
// MG996R mất khoảng 400ms - 500ms để quay hết hành trình 180 độ (tùy nguồn 5V hay 6V).
const int MOVE_TIME_MS       = 600; // Thời gian quay đến góc đích (ms)
const int HOLD_PEAK_DELAY_MS = 100; // Thời gian giữ ở đỉnh 180 độ (ms) trước khi quay về

Servo servos[NUM_SERVOS];
int currentAngle = 0;
bool isRunningCycle = false;

// ==============================================================================
// HÀM ĐIỀU KHIỂN GÓC CHO TẤT CẢ SERVO
// ==============================================================================
void writeServoAngle(int angle) {
  angle = constrain(angle, 0, 180);
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].write(angle);
  }
  currentAngle = angle;
}

// ==============================================================================
// CHU KỲ QUÉT TỰ ĐỘNG: 0° -> 180° -> 0° (TỐC ĐỘ TỐI ĐA - MAXIMUM SPEED)
// Gọi trực tiếp write() không qua bước đệm delay nhỏ để motor đạt vận tốc cơ khí cực đại
// ==============================================================================
void runFullCycle() {
  if (isRunningCycle) return;
  isRunningCycle = true;

  Serial.println("\n>>> [BAT DAU CHU KY] Quay toc do toi da: 0 do -> 180 do -> 0 do <<<");

  // Giai doan 1: Bung het toc do motor tu vi tri hien tai len 180 do
  Serial.println("[-] Quay MAX SPEED toi 180 do...");
  writeServoAngle(180);
  delay(MOVE_TIME_MS); // Cho servo hoan thanh hanh trinh co khi 180 do

  if (HOLD_PEAK_DELAY_MS > 0) {
    delay(HOLD_PEAK_DELAY_MS);
  }

  // Giai doan 2: Bung het toc do motor tu 180 do ve 0 do
  Serial.println("[-] Quay MAX SPEED ve 0 do...");
  writeServoAngle(0);
  delay(MOVE_TIME_MS); // Cho servo hoan thanh hanh trinh co khi ve 0 do

  Serial.println("[V] Da hoan tat chu ky ve 0 do!\n");
  Serial.println("----------------------------------------------");
  Serial.println("Nhan '1' de tiep tuc chay chu ky moi.");
  Serial.println("----------------------------------------------");

  isRunningCycle = false;
}

// ==============================================================================
// SETUP
// ==============================================================================
void setup() {
  // 1. VO HIEU HOA BROWNOUT DETECTOR TAM THOI TRONG CODE
  // Giup ESP32 khong bi reset tuc thi neu dien ap bi tut nhe trong vai mili-giay.
  // LUU Y: Ban van can dam bao nguon ngoai 5V du dong (2A-3A) cho servo MG996R!
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==============================================");
  Serial.println("    ESP32 - DIEU KHIEN SERVO MG996R (SERIAL)  ");
  Serial.println("==============================================");
  Serial.printf("- So luong servo: %d (Pins: ", NUM_SERVOS);
  for (int i = 0; i < NUM_SERVOS; i++) {
    Serial.printf("%d%s", SERVO_PINS[i], (i == NUM_SERVOS - 1) ? "" : ", ");
  }
  Serial.println(")");
  Serial.println("- Da go bo man hinh cam ung de tap trung test servo.");
  Serial.println("- Che do: Toc do quay MAX SPEED (0 -> 180 -> 0).");
  Serial.println("==============================================");

  // 2. KHOI TAO TIMER VA GAN CHAN SERVO
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].setPeriodHertz(50); // Chuan 50Hz cho RC servo
    servos[i].attach(SERVO_PINS[i], SERVO_MIN_US, SERVO_MAX_US);
  }

  // Dua servo ve vi tri 0 do ban dau
  writeServoAngle(0);
  delay(300);

  Serial.println("\n[HUONG DAN DIEU KHIEN]");
  Serial.println("-> Go '1' roi Enter: Servo chay chu ky 0* -> 180* -> 0* (MAX SPEED)");
  Serial.println("-> Go '0' roi Enter: Dua servo ve ngay 0*");
  Serial.println("-> Go mot goc (vd '90', '45', '120'): Quay toi goc do ngay lap tuc");
  Serial.println("\nSan sang! Hay nhap '1' vao Serial de test:\n");
}

// ==============================================================================
// LOOP
// ==============================================================================
void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Loai bo khoang trang va ky tu xuong dong

    if (input.length() == 0) return;

    if (input == "1") {
      runFullCycle();
    } 
    else if (input == "0") {
      Serial.println("\n[LENH '0'] Dua servo ve ngay 0 do...");
      writeServoAngle(0);
      Serial.println("[V] Da o vi tri 0 do.");
    } 
    else {
      // Kiem tra xem nguoi dung co nhap mot so goc cu the khong (0 - 180)
      int targetAngle = input.toInt();
      if (targetAngle > 0 || input == "0") {
        if (targetAngle >= 0 && targetAngle <= 180) {
          Serial.printf("\n[LENH GOC] Quay ngay den %d do (MAX SPEED)...\n", targetAngle);
          writeServoAngle(targetAngle);
          delay(MOVE_TIME_MS);
          Serial.printf("[V] Da toi %d do.\n", targetAngle);
        } else {
          Serial.println("[!] Goc khong hop le! Vui long nhap tu 0 den 180 do.");
        }
      } else {
        Serial.printf("[!] Ky tu khong xac dinh: '%s'. Hay nhan '1' de chay chu ky 0* -> 180* -> 0*.\n", input.c_str());
      }
    }
  }

  delay(10);
}
