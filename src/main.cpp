#include <Arduino.h>
#include <BleMouse.h>
#include <M5Unified.h>
#include <MadgwickAHRS.h>

// Madgwickフィルタのインスタンスを作成
Madgwick filter;

// BLEマウスのインスタンスを作成
BleMouse bleMouse("M5Mouse");

float ax, ay, az;  // 加速度データ
float gx, gy, gz;  // 角速度データ

// 前回のオイラー角（デバッグ用）
float preRoll = 0, prePitch = 0, preYaw = 0;

// カウンタ
int count = 0;

// セットアップ関数
void setup() {
    // M5StickCの初期化
    M5.begin();
    M5.Lcd.setRotation(1);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("M5Mouse");

    // BLEマウスの初期化
    bleMouse.begin();

    // センサーの初期化
    M5.Imu.begin();

    // Madgwickフィルタの初期化（サンプリング周波数を100Hzに設定）
    filter.begin(100.0);

    // 少し待機してから開始
    delay(1000);
}

// メインループ関数
void loop() {
    // センサーからデータを取得
    M5.Imu.getAccelData(&ax, &ay, &az);
    M5.Imu.getGyroData(&gx, &gy, &gz);

    // Madgwickフィルタを更新
    filter.updateIMU(gx, gy, gz, ax, ay, az);

    // // オイラー角を取得
    float roll = filter.getRoll();
    float pitch = filter.getPitch();
    float yaw = filter.getYaw();

    delay(10);  // 10ms待機（100Hz）

    // マウスの移動量を計算
    int x = -1 * (yaw - preYaw) * 4;
    int y = (roll - preRoll) * 3;

    // BLEマウスが接続されている場合
    if (bleMouse.isConnected()) {
        // マウスを移動
        bleMouse.move(x, y);
    }

    // 結果を表示
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(2, 2);
    M5.Lcd.printf("Roll:  %.2f\n", roll);
    M5.Lcd.printf("Pitch: %.2f\n", pitch);
    M5.Lcd.printf("Yaw:   %.2f\n", yaw);
    M5.Lcd.printf("x:%d y:%d\n", x, y);

    // 前回の値を記録
    if (count % 10 == 0) {
        preRoll = roll;
        prePitch = pitch;
        preYaw = yaw;
    }
    count++;
}