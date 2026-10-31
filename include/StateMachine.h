#pragma once
#include <Arduino.h>
#include "config.h"
#include "DisplayUI.h"
#include "SprayController.h"
#include "PaymentProvider.h"

enum AppState {
    STATE_HOME,        // Màn hình chờ / chọn chai
    STATE_PACKAGE,     // Chọn gói giá / số lần xịt
    STATE_PAYMENT,     // Màn hình quét mã QR & chờ thanh toán
    STATE_SPRAYING,    // Màn hình bấm nút xịt & quản lý cooldown 30s
    STATE_DONE,        // Màn hình hoàn tất
    STATE_ADMIN        // Màn hình Admin căn chỉnh cơ cấu & cảm ứng
};

class StateMachine {
public:
    StateMachine();

    void begin(PaymentProvider* provider);
    void update();

    // Chuyển trạng thái
    void changeState(AppState newState);
    AppState getCurrentState() const { return _state; }

    // Dành cho Serial Monitor debug
    void debugTriggerSpray();

private:
    AppState _state;
    PaymentProvider* _payment;

    int _selectedBottle;
    int _selectedPackage;
    int _spraysLeft;
    
    String _orderCode;
    String _qrPayload;

    uint32_t _stateStartTime;
    uint32_t _lastSprayTime;
    bool _sprayWasTriggered;

    int _adminBottle;

    void handleHomeState(int tx, int ty, bool tapped);
    void handlePackageState(int tx, int ty, bool tapped);
    void handlePaymentState(int tx, int ty, bool tapped);
    void handleSprayingState(int tx, int ty, bool tapped);
    void handleDoneState();
    void handleAdminState(int tx, int ty, bool tapped);
};

extern StateMachine stateMachine;

