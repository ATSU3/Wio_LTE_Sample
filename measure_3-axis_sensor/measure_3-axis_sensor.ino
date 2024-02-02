#include <WioLTEforArduino.h>
#include <ADXL345.h>
#include <stdio.h>

#define INTERVAL        (60000) // データ送信の間隔をミリ秒で指定
#define RECEIVE_TIMEOUT (10000) // 受信タイムアウトをミリ秒で指定

WioLTE Wio;
ADXL345 Accel;

void setup() {
  delay(200);

  SerialUSB.begin(115200);
  while (!SerialUSB); // For Native USB
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyGrove(true); // Groveコネクタの電源をON
  delay(500); // 電源が安定するまで待機

  Accel.powerOn(); // 加速度センサの電源をON

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

  SerialUSB.println("### Setup completed.");
}

void loop() {
  char data[100];
  int x, y, z;

  // 加速度センサからのデータを読み取る
  Accel.readXYZ(&x, &y, &z);
  sprintf(data, "{\"x\":%d,\"y\":%d,\"z\":%d}", x, y, z);

  SerialUSB.println("### Open.");
  int connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    // ここで直接遅延してからリターンする
    delay(INTERVAL);
    return; // エラー発生時に関数から抜ける
  }

  SerialUSB.println("### Send.");
  SerialUSB.print("Send: ");
  SerialUSB.println(data);
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    Wio.SocketClose(connectId); // エラー発生時にソケットを閉じる
    delay(INTERVAL);
    return; // エラー発生時に関数から抜ける
  }

  SerialUSB.println("### Receive.");
  int length = Wio.SocketReceive(connectId, data, sizeof(data), RECEIVE_TIMEOUT);
  if (length < 0) {
    SerialUSB.println("### ERROR! ###");
  } else if (length == 0) {
    SerialUSB.println("### RECEIVE TIMEOUT! ###");
  } else {
    SerialUSB.print("Receive: ");
    SerialUSB.println(data);
  }

  SerialUSB.println("### Close.");
  Wio.SocketClose(connectId);

  delay(INTERVAL); // 次の送信までの間隔
}

