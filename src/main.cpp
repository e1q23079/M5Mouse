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

// BLE接続状態
bool bleStatus = false;

// マウス状態
bool mouseStatus = false;

// ディスプレイに表示する関数
void showDisplay(const char text[], const char text2[]) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.printf("%s\n%s\nM5->TOP\nBtnA:Next\nBtnB:Back\n", text, text2);
}

// セットアップ関数
void setup() {
    // M5StickCの初期化
    M5.begin();
    M5.Lcd.setRotation(1);
    M5.Lcd.setTextSize(2);

    // 初期メッセージの表示
    showDisplay("M5Mouse", "Initializing...");

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
    // IMUセンサーのデータを更新
    M5.update();

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
    int x = -1 * (yaw - preYaw) * 5;
    int y = (roll - preRoll) * 3;

    // BLEマウスが接続されている場合
    if (bleMouse.isConnected()) {
        if (!bleStatus) {
            bleStatus = true;
            showDisplay("M5Mouse",
                        "Connected");  // 接続中メッセージを表示
        }
        //  移動量が閾値を超えた場合にマウスを移動
        if (mouseStatus) {  // マウス有効状態のとき
            if (abs(x) > 5 || abs(y) > 5) {
                bleMouse.move(x, y);  // マウスを移動
            }
        }
        //  ボタンA
        if (M5.BtnA.wasPressed()) {
            bleMouse.move(0, 0, -1);  // ホイールを上にスクロール
        }
        //  ボタンB
        if (M5.BtnB.wasPressed()) {
            bleMouse.move(0, 0, 1);  // ホイールを下にスクロール
        }
        //  電源ボタン
        if (M5.Power.getKeyState() == 2) {
            mouseStatus = !mouseStatus;
        }
    } else {
        if (bleStatus) {
            bleStatus = false;
            showDisplay("M5Mouse",
                        "Disconnected");  // 接続待機メッセージを表示
        }
    }

    // // 結果を表示
    // M5.Lcd.fillScreen(BLACK);
    // M5.Lcd.setCursor(2, 2);
    // M5.Lcd.printf("Roll:  %.2f\n", roll);
    // M5.Lcd.printf("Pitch: %.2f\n", pitch);
    // M5.Lcd.printf("Yaw:   %.2f\n", yaw);
    // M5.Lcd.printf("x:%d y:%d\n", x, y);

    // 前回の値を記録
    if (count % 10 == 0) {
        preRoll = roll;
        prePitch = pitch;
        preYaw = yaw;
    }
    count++;  // カウンタをインクリメント
}