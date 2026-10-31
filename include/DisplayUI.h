#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

class DisplayUI {
public:
    DisplayUI();

    void begin();

    // Nhận diện bất kỳ cú chạm nào trên toàn bộ bề mặt màn hình
    bool pollAnyTouch();

    // Các màn hình trạng thái
    void showReadyScreen();
    void showSprayingScreen();
    void showDoneScreen();

private:
    TFT_eSPI _tft;
    bool _wasTouched;
    uint32_t _lastTapTime;
};

extern DisplayUI displayUI;
