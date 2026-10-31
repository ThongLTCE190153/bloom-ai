#pragma once
#include <Arduino.h>

// ==============================================================================
// CẤU HÌNH PHẦN CỨNG 1 SERVO DUY NHẤT (GPIO 13 / D13)
// ==============================================================================
#define NUM_SERVOS            1
#define SERVO_PIN             13     // Chân tín hiệu Servo D13 (GPIO 13)

// Dải xung điều khiển chuẩn cho MG996R (500us - 2400us)
#define SERVO_MIN_US          500
#define SERVO_MAX_US          2400

// ==============================================================================
// HÀNH TRÌNH VÀ THỜI GIAN ĐIỀU KHIỂN
// ==============================================================================
#define DEFAULT_REST_ANGLE    0      // Góc nghỉ an toàn: 0 độ
#define DEFAULT_PRESS_ANGLE   170    // Góc xịt tối đa: 170 độ (tránh điểm chết 180 độ gây kẹt cơ khí)

// Thời gian ép xuống
#define SERVO_MOVE_TIME_MS    600    // ms để ép xuống (Max Speed)
#define SERVO_HOLD_PEAK_MS    300    // ms giữ ở đỉnh để nước hoa phun ra đủ lượng

// ==============================================================================
// CHẾ ĐỘ TRẢ VỀ CỦA SERVO (RETURN SPEED CONTROL)
// ==============================================================================
// 1 = TRẢ VỀ MAX SPEED (Tốc độ tối đa, xuống nhanh thì về cũng nhanh tương đương ~500ms)
//     Lưu ý: Dùng tốt nhất khi DEFAULT_PRESS_ANGLE <= 170 độ để không bị kẹt điểm chết
// 0 = TRẢ VỀ THEO BƯỚC (STEPPED RETURN): Dành cho cơ cấu cần về từ từ
#define SERVO_DIRECT_RETURN   1      

// Thời gian chờ khi trả về MAX SPEED (chỉ áp dụng khi SERVO_DIRECT_RETURN = 1)
#define SERVO_RETURN_TIME_MS  600    // ms để servo bung hết tốc độ về lại 0 độ

// Cấu hình khi dùng SERVO_DIRECT_RETURN = 0 (trả về từng bước):
#define SERVO_RETURN_STEP_DEG 5      // Nhảy mỗi bước 5 độ (đủ lớn để motor có lực kéo mạnh)
#define SERVO_RETURN_STEP_MS  8      // ms giữa các bước
