#include <WioLTEforArduino.h>
#include <stdio.h>

#define SEND_INTERVAL    (10000) // 10秒ごとにデータを送信
#define RECEIVE_TIMEOUT  (10000) // 受信タイムアウト時間（このコードでは使用しませんが、将来のために残します）
#define BUTTON_PIN       (WIOLTE_D38) // ボタンが接続されているピン

WioLTE Wio;

unsigned long lastSendTime = 0; // 最後にデータを送信した時刻を記録
int buttonPressCount = 0; // ボタンが押された回数をカウント

void setup() {
  delay(200);

  SerialUSB.begin(9600);
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connecting to \"soracom.io\".");
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  // ボタンピンを入力として初期化
  pinMode(BUTTON_PIN, INPUT);
  lastSendTime = millis(); // タイマーをリセット

  SerialUSB.println("### Setup completed.");
}

void loop() {
  // ボタンの状態をチェック
  if (digitalRead(BUTTON_PIN) == HIGH) { // ボタンが押された場合
    buttonPressCount++; // カウントを増やす
    delay(10); // デバウンス用の短い遅延
    while(digitalRead(BUTTON_PIN) == HIGH); // ボタンが離されるのを待つ
  }

  // 10秒ごとにカウント値を送信
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    char data[64];
    sprintf(data, "{\"buttonPressCount\":%d}", buttonPressCount);

    SerialUSB.println("Sending data to Soracom Harvest...");
    SerialUSB.println(data);

    int connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
    if (connectId >= 0) {
      if (Wio.SocketSend(connectId, data)) {
        SerialUSB.println("Data sent successfully");
      } else {
        SerialUSB.println("Failed to send data");
      }
      Wio.SocketClose(connectId);
    } else {
      SerialUSB.println("Failed to open socket");
    }

    buttonPressCount = 0; // カウンターをリセット
    lastSendTime = millis(); // タイマーをリセット
  }
}
