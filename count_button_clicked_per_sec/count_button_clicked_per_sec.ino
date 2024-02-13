// Wio LTEボード用のライブラリをインクルード
#include <WioLTEforArduino.h>
// 標準入出力関数を使用するためのライブラリをインクルード
#include <stdio.h>

// ボタンが接続されているピン番号を定義
#define BUTTON_PIN       (WIOLTE_D38)
// データ送信の間隔をミリ秒で定義（10秒）
#define SEND_INTERVAL    (10000)

// Wio LTEオブジェクトを作成
WioLTE Wio;

// 前回データを送信した時間を記録する変数を初期化
unsigned long lastSendTime = 0;
// ボタンが押された回数をカウントする変数を初期化
int buttonPressCount = 0;
// 前回のボタンの状態を記録する変数を初期化（LOWは未押下を意味する）
int lastButtonState = LOW;

// セットアップ関数（プログラム起動時に一度だけ実行される）
void setup() {
  // シリアル通信を9600bpsで開始
  SerialUSB.begin(9600);
  // ボタンピンを入力として設定
  pinMode(BUTTON_PIN, INPUT);

  // シリアルモニタにスタートメッセージを表示
  SerialUSB.println("--- START ---");

  // Wio LTEモジュールの初期化
  Wio.Init();
  // LTEモジュールへの電源供給を有効化
  Wio.PowerSupplyLTE(true);
  // 初期化後の短い遅延
  delay(200);

  // LTEモジュールの起動に失敗した場合、エラーメッセージを表示して無限ループ
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("LTE module turn on or reset failed");
    while (1);
  }

  // ネットワーク接続に失敗した場合、エラーメッセージを表示して無限ループ
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("Network connection failed");
    while (1);
  }

  // タイマーの初期化（現在の時間を記録）
  lastSendTime = millis();
}

// メインループ関数（無限に繰り返し実行される）
void loop() {
  // ボタンの現在の状態を読み取る
  int buttonState = digitalRead(BUTTON_PIN);

  // ボタンの状態が前回と異なり、かつボタンが押されている場合
  if (buttonState != lastButtonState && buttonState == HIGH) {
    // ボタン押下回数をカウントアップ
    buttonPressCount++;
    // シリアルモニタに押下回数を表示
    SerialUSB.print("Button pressed: ");
    SerialUSB.println(buttonPressCount);
  }
  // ボタンの状態を更新
  lastButtonState = buttonState;

  // 指定した送信間隔が経過した場合
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    // 送信データを格納するための文字列バッファ
    char data[64];
    // データ文字列をフォーマットしてバッファに格納
    sprintf(data, "{\"buttonPressCount\":%d}", buttonPressCount);

    // 送信データの内容をシリアルモニタに表示
    SerialUSB.println("Sending data to Soracom Harvest...");
    SerialUSB.println(data);

    // Soracom HarvestにUDPで送信するためのソケットをオープン
    int connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
    // ソケットオープンに成功した場合
    if (connectId >= 0) {
      // データ送信を試み、成功したかどうかをチェック
      if (Wio.SocketSend(connectId, data)) {
        // 送信成功をシリアルモニタに表示
        SerialUSB.println("Data sent successfully");
      } else {
        // 送信失敗をシリアルモニタに表示
        SerialUSB.println("Failed to send data");
      }
      // 送信後、ソケットをクローズ
      Wio.SocketClose(connectId);
    } else {
      // ソケットオープン失敗をシリアルモニタに表示
      SerialUSB.println("Failed to open socket");
    }

    // カウンターとタイマーをリセット
    buttonPressCount = 0;
    lastSendTime = millis();
  }
}
