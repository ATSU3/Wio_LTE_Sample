#include <WioLTEforArduino.h>
#define console SerialUSB
WioLTE Wio;

#include <GroveDriverPack.h>
GroveBoard Board;
GroveUltrasonicRanger Ranger(&Board.D38);

void setup() {
  Wio.Init();
  console.begin(9600);
  while (!console); // コンソールが利用可能になるまで待機
  console.println("--- START");

  Board.D38.Enable();
  Ranger.Init();

  console.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(500);

  console.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    console.println("### ERROR! ###");
    return;
  }

  console.println("### Connecting to \"soracom.io\".");
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    console.println("### ERROR! ###");
    return;
  }
}

#define MEASUREMENT_INTERVAL 10000 // 測定間隔（ミリ秒）

void loop() {
  // 水面までの距離を測定
  Ranger.Read();
  float distance = Ranger.Distance;
  console.print("Water level distance: ");
  console.println(distance, 2); // 小数点以下2桁で表示

  // Soracomへデータを送信
  char data[64];
  sprintf(data, "{\"water_level\":%.2f}", distance);

  console.println("### Opening socket.");
  int connectId = Wio.SocketOpen("uni.soracom.io", 23080, WIOLTE_UDP);
  if (connectId < 0) {
    console.println("### ERROR: Failed to open socket.");
    return;
  }

  console.println("### Sending data.");
  if (!Wio.SocketSend(connectId, data)) {
    console.println("### ERROR: Failed to send data.");
  } else {
    console.println("Data sent successfully.");
  }

  // ソケットを閉じる
  console.println("### Closing socket.");
  if (!Wio.SocketClose(connectId)) {
    console.println("### ERROR: Failed to close socket.");
  }

  // 次の測定まで待機
  delay(MEASUREMENT_INTERVAL);
}
