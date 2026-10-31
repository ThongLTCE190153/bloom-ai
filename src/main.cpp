#include <Arduino.h>
#include "config.h"
#include "SprayController.h"
#include "DisplayUI.h"

// ==============================================================================
// SETUP
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(400);

    Serial.println("\n========================================================");
    Serial.println("  BLOOM PERFUME - TOUCH TO SPRAY (SERVO D13: 0->ANGLE->0) ");
    Serial.println("========================================================");
    Serial.println("- Phan cung: 1 Servo duy nhat noi chan D13 (GPIO 13)");
    Serial.println("- Co che: Cham tay bat ky dau tren man hinh la Servo quay:");
    Serial.printf ("          0 do ---> %d do (giu %dms) ---> 0 do\n",
                   DEFAULT_PRESS_ANGLE, SERVO_HOLD_PEAK_MS);
    Serial.println("========================================================\n");

    // 1. Khởi tạo Servo trên chân D13 (đưa về 0 độ, ghim task chạy trên Core 0)
    sprayCtrl.begin();

    // 2. Khởi tạo Màn hình & Cảm ứng
    displayUI.begin();
    displayUI.showReadyScreen();

    Serial.println("[HE THONG DA SAN SANG]");
    Serial.println("-> Hay dung ngon tay CHAM VAO MAN HINH de kich hoat Servo!");
    Serial.println("-> Hoac go phim '1' tren Serial Monitor de test quay.");
    Serial.println("--------------------------------------------------------\n");
}

// ==============================================================================
// LOOP (Chạy trên Core 1)
// ==============================================================================
static bool     _showingDone = false;
static uint32_t _doneStartMs = 0;
static const uint32_t DONE_SCREEN_MS = 600; // Hiển thị Done bao lâu trước khi về Ready

void loop() {
    bool isBusy = sprayCtrl.isBusy();
    // Chỉ cho phép nhận lệnh mới khi máy rảnh VÀ không đang trong thời gian hiển thị Done
    bool canTrigger = (!isBusy && !_showingDone);

    // 1. Đọc lệnh từ Serial Monitor (gõ '1', '0', 'spray', 'stop'...)
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input.length() > 0) {
            if (input == "1" || input.equalsIgnoreCase("spray")) {
                if (canTrigger) {
                    displayUI.showSprayingScreen();
                    sprayCtrl.triggerSpray();
                } else {
                    Serial.println("[Serial] He thong dang ban, bo qua lenh!");
                }
            } else {
                sprayCtrl.processSerialCommand(input);
            }
        }
    }

    // 2. Kiểm tra cảm ứng: CHỈ CẦN CHẠM TAY VÀO BẤT KỲ ĐÂU TRÊN MÀN HÌNH
    if (displayUI.pollAnyTouch()) {
        if (canTrigger) {
            displayUI.showSprayingScreen();
            sprayCtrl.triggerSpray();
        } else {
            Serial.println("[Touch] He thong dang trong chu ky, vui long doi...");
        }
    }

    // 3. Phát hiện servo vừa hoàn tất chu kỳ → hiển thị Done NON-BLOCKING
    static bool wasBusy = false;
    if (wasBusy && !isBusy) {
        displayUI.showDoneScreen();
        _showingDone  = true;
        _doneStartMs  = millis();
    }
    wasBusy = isBusy;

    // 4. Sau DONE_SCREEN_MS ms: tự động về lại màn hình Ready
    if (_showingDone && (millis() - _doneStartMs >= DONE_SCREEN_MS)) {
        _showingDone = false;
        if (!sprayCtrl.isBusy()) {
            displayUI.showReadyScreen();
        }
    }

    delay(10);
}
