#include <WioLTEforArduino.h>
#include <stdio.h>

#define BUTTON_PIN       (WIOLTE_D38)
#define SEND_INTERVAL    (10000) // 10秒間隔でデータを送信

WioLTE Wio;

// カウンターとタイマー用変数
unsigned long lastSendTime = 0;
int buttonPressCount = 0;
int lastButtonState = LOW;

void setup() {
  // シリアルとボタンの初期化
  SerialUSB.begin(9600);
  pinMode(BUTTON_PIN, INPUT);

  SerialUSB.println("--- START ---");

  // Wio LTEの初期化
  Wio.Init();
  Wio.PowerSupplyLTE(true);
  delay(200);

  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("LTE module turn on or reset failed");
    while (1);
  }

  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("Network connection failed");
    while (1);
  }

  lastSendTime = millis(); // タイマーの初期化
}

void loop() {
  // ボタンの状態を読み取る
  int buttonState = digitalRead(BUTTON_PIN);

  // ボタンの状態が変化し、かつボタンが押された場合
  if (buttonState != lastButtonState && buttonState == HIGH) {
    buttonPressCount++;
    SerialUSB.print("Button pressed: ");
    SerialUSB.println(buttonPressCount);
  }
  lastButtonState = buttonState;

  // 10秒ごとにデータを送信
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

    // カウンターとタイマーをリセット
    buttonPressCount = 0;
    lastSendTime = millis();
  }
}
