#pragma once
#include <Arduino.h>
#include "config.h"

enum class PaymentStatus {
    PENDING,
    SUCCESS,
    FAILED,
    TIMEOUT
};

class PaymentProvider {
public:
    virtual ~PaymentProvider() = default;
    virtual void begin() = 0;
    virtual bool createOrder(int bottleId, uint32_t amount, String& outOrderCode, String& outQrPayload) = 0;
    virtual PaymentStatus checkStatus(const String& orderCode) = 0;
    virtual void cancelOrder(const String& orderCode) = 0;
};

// ==============================================================================
// MOCK PAYMENT PROVIDER (Dùng cho Demo / Chạy offline không cần WiFi)
// ==============================================================================
class MockPaymentProvider : public PaymentProvider {
public:
    MockPaymentProvider();
    void begin() override;
    bool createOrder(int bottleId, uint32_t amount, String& outOrderCode, String& outQrPayload) override;
    PaymentStatus checkStatus(const String& orderCode) override;
    void cancelOrder(const String& orderCode) override;

    // Giả lập thanh toán thành công (qua nút bấm trên màn hình hoặc lệnh Serial)
    void simulatePaymentSuccess(const String& orderCode);

    // Xử lý lệnh console Serial (ví dụ: gõ "pay" hoặc "paid" để giả lập)
    bool handleSerialInput(const String& cmd);

private:
    String _currentOrderCode;
    bool _isPaid;

    // Các hàm phụ trợ sinh chuỗi VietQR chuẩn EMVCo
    String buildVietQR(uint32_t amount, const String& memo);
    String tlv(const char* tag, const String& val);
    uint16_t crc16(const String& data);
};

extern MockPaymentProvider paymentMock;

