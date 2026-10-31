#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include <atomic>        // std::atomic — an toàn tuyệt đối đa luồng giữa Core 0 và Core 1
#include "config.h"

class SprayController {
public:
    SprayController();
    ~SprayController();

    // Khởi tạo phần cứng PWM trên chân GPIO 13, tạo Task Core 0
    bool begin();

    // Kích hoạt chu kỳ xịt: 0° -> 170° -> tự quay về 0° (Bất đồng bộ)
    bool triggerSpray();

    // Đưa servo về 0 độ (khi hệ thống rảnh)
    void returnToRest();

    // Dừng khẩn cấp (ra hiệu Core 0 dừng an toàn, tránh xung đột ghi đa lõi)
    void emergencyStop();

    // Kiểm tra servo có đang trong chu kỳ quay không (thread-safe)
    bool isBusy() const;

    // Xử lý lệnh Serial console
    void processSerialCommand(const String& cmd);

    // FreeRTOS Task chạy ngầm trên Core 0
    static void taskWorker(void* pvParameters);

private:
    Servo _servo;
    std::atomic<bool> _isBusy;        // Atomic flag kiểm soát trạng thái bận
    std::atomic<bool> _stopRequested; // Atomic flag báo dừng khẩn cấp
    QueueHandle_t _cmdQueue;
    TaskHandle_t _taskHandle;

    void executeFullCycle();
};

extern SprayController sprayCtrl;
