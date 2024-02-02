#include <WioLTEforArduino.h>
#include <ADXL345.h>

#define INTERVAL        (60000) // データ送信の間隔をミリ秒で指定
#define RECEIVE_TIMEOUT (10000) // 受信タイムアウトをミリ秒で指定

WioLTE Wio;
ADXL345 Accel;

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  // Wio LTEモジュールの初期化
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  // 電源供給
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyGrove(true); // Groveコネクタの電源をON
  delay(500); // 電源が安定するまで待機

  // 加速度センサの初期化
  Accel.powerOn();

  // LTEモジュールの電源をON
  Wio.PowerSupplyLTE(true);
  delay(500); // 電源が安定するまで待機

  // LTEモジュールの起動またはリセット
  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  // Soracom Airに接続
  SerialUSB.println("### Connecting to \"soracom.io\".");
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Setup completed.");
}

void loop() {
  int x, y, z;
  char data[256]; // JSONデータを格納するための配列

  // 加速度センサからのデータを読み取る
  Accel.readXYZ(&x, &y, &z);
  // 加速度データとデバイスの稼働時間をJSON形式でフォーマット
  sprintf(data, "{\"x\":%d,\"y\":%d,\"z\":%d,\"uptime\":%lu}", x, y, z, millis() / 1000);

  SerialUSB.println("### Open.");
  // Soracom Harvestにデータを送信するためのソケットを開く
  int connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

  SerialUSB.println("### Send.");
  SerialUSB.print("Send: ");
  SerialUSB.println(data);
  // ソケットを通じてデータを送信
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }

  // 送信後、ソケットを閉じる
  SerialUSB.println("### Close.");
  if (!Wio.SocketClose(connectId)) {
    SerialUSB.println("### ERROR! ###");
    // エラー処理
  }

err_close:
  // エラーが発生した場合の処理
err:
  delay(INTERVAL); // 次の送信までの間隔
}
