#include "DisplayUI.h"
#include "config.h"

DisplayUI displayUI;

DisplayUI::DisplayUI() : _wasTouched(false), _lastTapTime(0) {}

void DisplayUI::begin() {
    _tft.init();
    _tft.setRotation(1); // 320x240 ngang (Landscape)
    _tft.fillScreen(TFT_BLACK);

    // Nạp sẵn thông số Calib chuẩn cho màn hình 2.4" XPT2046 để getTouch luôn sẵn sàng
    uint16_t calData[5] = { 286, 3534, 283, 3600, 6 };
    _tft.setTouch(calData);

    Serial.println("[DisplayUI] Khoi tao man hinh TFT + Cam ung toan man hinh (Touch Anywhere).");
}

bool DisplayUI::pollAnyTouch() {
    uint16_t x = 0, y = 0;

    // CHỈ dùng getTouch() với threshold 200 — đủ nhạy cho ngón tay
    // KHÔNG dùng getTouchRawZ(): chip XPT2046 luôn báo noise ~80-200 dù không ai chạm!
    bool touched = _tft.getTouch(&x, &y, 200);

    // Lọc bỏ tọa độ (0, 0): đây là "ghost touch" / nhiễu điện — không phải chạm thật
    if (x == 0 && y == 0) {
        touched = false;
    }

    uint32_t now = millis();
    bool isTapTriggered = false;

    if (touched) {
        // Chống dội: 600ms giữa các lần chạm (đủ rộng để tay nhấc ra khỏi kính)
        if (!_wasTouched && (now - _lastTapTime > 600)) {
            isTapTriggered = true;
            _lastTapTime = now;
            Serial.printf("[Touch] >>> CHAM TAY THAT: X=%d, Y=%d <<<\n", x, y);
        }
    }

    _wasTouched = touched;
    return isTapTriggered;
}

void DisplayUI::showReadyScreen() {
    _tft.fillScreen(TFT_BLACK);

    // Thanh tiêu đề phía trên
    _tft.fillRect(0, 0, 320, 26, TFT_NAVY);
    _tft.drawFastHLine(0, 26, 320, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, TFT_NAVY);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString("BLOOM - AUTOMATIC DISPENSER", 160, 13, 2);

    // Khối chạm lớn bao trọn toàn bộ bề mặt màn hình
    _tft.fillRoundRect(10, 34, 300, 196, 12, TFT_DARKGREEN);
    _tft.drawRoundRect(10, 34, 300, 196, 12, TFT_WHITE);
    _tft.drawRoundRect(12, 36, 296, 192, 10, TFT_GREEN);

    _tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    _tft.drawString("TOUCH SCREEN", 160, 75, 4);
    _tft.drawString("TO SPRAY", 160, 110, 4);

    _tft.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
    char buf[50];
    snprintf(buf, sizeof(buf), "Servo D13: 0 deg -> %d deg -> 0 deg", DEFAULT_PRESS_ANGLE);
    _tft.drawString(buf, 160, 155, 2);

    _tft.setTextColor(TFT_CYAN, TFT_DARKGREEN);
    _tft.drawString("Cham bat ky dau de xit nuoc hoa", 160, 190, 2);
}

void DisplayUI::showSprayingScreen() {
    // Chuyển toàn bộ khung sang màu Cam rực rỡ báo hiệu đang quay
    _tft.fillRoundRect(10, 34, 300, 196, 12, TFT_ORANGE);
    _tft.drawRoundRect(10, 34, 300, 196, 12, TFT_WHITE);

    _tft.setTextColor(TFT_WHITE, TFT_ORANGE);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString("SPRAYING...", 160, 80, 4);

    _tft.setTextColor(TFT_YELLOW, TFT_ORANGE);
    char buf[40];
    snprintf(buf, sizeof(buf), "0 -> %d -> 0 DEG", DEFAULT_PRESS_ANGLE);
    _tft.drawString(buf, 160, 120, 4);

    _tft.setTextColor(TFT_WHITE, TFT_ORANGE);
    _tft.drawString("Dang quay servo...", 160, 165, 2);
}

void DisplayUI::showDoneScreen() {
    // Chuyển khung sang màu Xanh Dương hoàn tất
    _tft.fillRoundRect(10, 34, 300, 196, 12, TFT_BLUE);
    _tft.drawRoundRect(10, 34, 300, 196, 12, TFT_WHITE);

    _tft.setTextColor(TFT_WHITE, TFT_BLUE);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString("COMPLETED!", 160, 90, 4);

    _tft.setTextColor(TFT_GREENYELLOW, TFT_BLUE);
    _tft.drawString("SERVO DA VE 0 DO", 160, 135, 4);
}
