#include "SprayController.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

SprayController sprayCtrl;

SprayController::SprayController()
    : _isBusy(false), _stopRequested(false), _cmdQueue(nullptr), _taskHandle(nullptr) {}

SprayController::~SprayController() {
    if (_taskHandle != nullptr) vTaskDelete(_taskHandle);
    if (_cmdQueue != nullptr) vQueueDelete(_cmdQueue);
}

bool SprayController::begin() {
    // 1. Tắt Brownout detector phần cứng mềm chống sụt áp nguồn servo
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // 2. Khởi tạo Timer PWM cho Servo trên chân D13 (GPIO 13)
    ESP32PWM::allocateTimer(0);
    _servo.setPeriodHertz(50); // 50Hz tiêu chuẩn cho Servo RC
    _servo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);

    // Đưa servo về góc 0 độ ngay khi bật nguồn
    _servo.write(DEFAULT_REST_ANGLE);
    delay(300);

    // 3. Tạo Queue lệnh điều khiển
    _cmdQueue = xQueueCreate(4, sizeof(int));
    if (_cmdQueue == nullptr) {
        Serial.println("[SprayController] Loi tao Queue!");
        return false;
    }

    // 4. Tạo FreeRTOS Task chạy độc lập trên Core 0 (để màn hình Core 1 không bị giật)
    BaseType_t res = xTaskCreatePinnedToCore(
        taskWorker,
        "ServoD13_Core0",
        4096,
        this,
        5,
        &_taskHandle,
        0 // Ghim cố định Core 0
    );

    if (res != pdPASS) {
        Serial.println("[SprayController] Loi tao Task tren Core 0!");
        return false;
    }

    Serial.println("[SprayController] Da khoi tao 1 Servo tren D13 (Core 0, San sang).");
    return true;
}

bool SprayController::triggerSpray() {
    // Test-and-set nguyên tử (atomic): Ngăn chặn việc bấm liên tiếp 2 lần từ Core 1
    bool expected = false;
    if (!_isBusy.compare_exchange_strong(expected, true)) {
        Serial.println("[SprayController] Servo dang trong chu ky quay, bo qua lenh!");
        return false;
    }

    int cmd = 1;
    if (xQueueSend(_cmdQueue, &cmd, 0) != pdTRUE) {
        _isBusy = false; // Hoàn lại trạng thái nếu hàng đợi đầy
        return false;
    }
    return true;
}

void SprayController::returnToRest() {
    if (!_isBusy) {
        _servo.write(DEFAULT_REST_ANGLE);
    }
}

void SprayController::emergencyStop() {
    Serial.println("\n[!] DUNG KHAN CAP: Yeu cau Core 0 dung chu ky!");
    _stopRequested = true;
    if (_cmdQueue != nullptr) {
        xQueueReset(_cmdQueue);
    }
}

bool SprayController::isBusy() const {
    return _isBusy.load();
}

void SprayController::executeFullCycle() {
    _stopRequested = false;
    Serial.println("\n==============================================");
    Serial.printf (">>> [SERVO D13] BAT DAU CHU KY: 0 -> %d -> 0 <<<\n", DEFAULT_PRESS_ANGLE);
    Serial.println("==============================================");

    // --- Giai đoạn 1: Ép xuống tốc độ tối đa ---
    Serial.printf("[-] Quay MAX SPEED xuong %d do...\n", DEFAULT_PRESS_ANGLE);
    _servo.write(DEFAULT_PRESS_ANGLE);
    vTaskDelay(pdMS_TO_TICKS(SERVO_MOVE_TIME_MS));

    // Giữ ở đỉnh để nước hoa xịt ra đủ lượng
    if (SERVO_HOLD_PEAK_MS > 0 && !_stopRequested) {
        Serial.printf("[-] Giu o dinh %dms...\n", SERVO_HOLD_PEAK_MS);
        vTaskDelay(pdMS_TO_TICKS(SERVO_HOLD_PEAK_MS));
    }

    // --- Giai đoạn 2: Trả về 0 độ ---
    if (!_stopRequested) {
#if SERVO_DIRECT_RETURN == 1
        // Trả về tốc độ tối đa: nhanh tương đương chiều đi xuống
        Serial.println("[-] Quay MAX SPEED tra ve 0 do...");
        _servo.write(DEFAULT_REST_ANGLE);
        vTaskDelay(pdMS_TO_TICKS(SERVO_RETURN_TIME_MS));
#else
        // Trả về theo từng bước nhảy góc
        Serial.printf("[-] STEPPED RETURN: %d -> 0 (step=%d deg, delay=%dms)...\n",
                      DEFAULT_PRESS_ANGLE, SERVO_RETURN_STEP_DEG, SERVO_RETURN_STEP_MS);
        for (int angle = DEFAULT_PRESS_ANGLE; angle >= DEFAULT_REST_ANGLE; angle -= SERVO_RETURN_STEP_DEG) {
            if (_stopRequested) break;
            _servo.write(angle);
            vTaskDelay(pdMS_TO_TICKS(SERVO_RETURN_STEP_MS));
        }
        _servo.write(DEFAULT_REST_ANGLE);
        vTaskDelay(pdMS_TO_TICKS(200));
#endif
    } else {
        Serial.println("[!] Emergency stop triggered, tra ve 0 do ngay!");
        _servo.write(DEFAULT_REST_ANGLE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // Luôn giữ cố định tại 0 độ và giải phóng trạng thái bận
    _servo.write(DEFAULT_REST_ANGLE);
    _isBusy = false;
    _stopRequested = false;
    Serial.println("[V] Da hoan tat chu ky - servo da ve 0 do an toan!\n");
}

void SprayController::taskWorker(void* pvParameters) {
    SprayController* self = static_cast<SprayController*>(pvParameters);
    int cmd = 0;

    while (true) {
        if (xQueueReceive(self->_cmdQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            self->executeFullCycle();
        }
    }
}

void SprayController::processSerialCommand(const String& cmd) {
    String s = cmd;
    s.trim();
    if (s.length() == 0) return;

    if (s == "1" || s.equalsIgnoreCase("spray")) {
        triggerSpray();
    } else if (s == "0") {
        returnToRest();
    } else if (s.equalsIgnoreCase("stop")) {
        emergencyStop();
    } else {
        int angle = s.toInt();
        if (angle >= 0 && angle <= 180) {
            if (!_isBusy) {
                Serial.printf("[Serial] Quay servo toi %d do...\n", angle);
                _servo.write(angle);
            } else {
                Serial.println("[Serial] Servo dang bận chu kỳ, không nhận góc thủ công!");
            }
        }
    }
}
