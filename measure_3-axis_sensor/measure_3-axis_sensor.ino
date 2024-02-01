#include <WioLTEforArduino.h>
#include <ADXL345.h> // 加速度センサー用ライブラリ

#define SEND_INTERVAL (10000) // 10秒ごとにデータを送信

WioLTE Wio;
ADXL345 Accel;

void setup() {
  delay(200);
  SerialUSB.begin(9600);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyGrove(true);
  delay(500);
  
  // 加速度センサーの初期化
  Accel.powerOn();

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
  // 加速度センサーからのデータ読み取り
  int x, y, z;
  Accel.readXYZ(&x, &y, &z);

  // 加速度データをJSON形式で準備
  char data[100];
  sprintf(data, "{\"x\":%d,\"y\":%d,\"z\":%d}", x, y, z);

  SerialUSB.println("### Open socket.");
  int connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

  // データ送信
  SerialUSB.print("Send: ");
  SerialUSB.println(data);
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }

err_close:
  SerialUSB.println("### Close socket.");
  if (!Wio.SocketClose(connectId)) {
    SerialUSB.println("### ERROR! ###");
  }

err:
  delay(SEND_INTERVAL); // 次の送信まで待機
}
