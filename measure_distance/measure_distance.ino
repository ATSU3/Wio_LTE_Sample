// Wio LTEボードと連携するためのライブラリをインクルード
#include <WioLTEforArduino.h>
// USB経由でのデバッグ出力用にSerialUSBをconsoleとして定義
#define console SerialUSB
// Wio LTEオブジェクトを作成
WioLTE Wio;

// Groveシステムのセンサーとアクチュエータを管理するためのライブラリをインクルード
#include <GroveDriverPack.h>
// Groveシステムのボードオブジェクトを作成
GroveBoard Board;
// D38ポートに接続された超音波距離センサーを初期化
GroveUltrasonicRanger Ranger(&Board.D38);

// スケッチの初期設定を行うsetup関数
void setup() {
  // Wio LTEボードの初期化
  Wio.Init();
  // コンソール（SerialUSB）の開始
  console.begin(9600);
  // コンソールが利用可能になるまで待機
  while (!console);
  // スタートメッセージを表示
  console.println("--- START");

  // D38ポートを有効にする
  Board.D38.Enable();
  // 超音波距離センサーの初期化
  Ranger.Init();

  // 電源供給の開始メッセージ
  console.println("### Power supply ON.");
  // LTEモジュールへの電源供給を開始
  Wio.PowerSupplyLTE(true);
  // 少し待機
  delay(500);

  // LTEモジュールの起動またはリセット
  console.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    console.println("### ERROR! ###");
    return;
  }

  // SORACOMに接続するための情報を表示
  console.println("### Connecting to \"soracom.io\".");
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    console.println("### ERROR! ###");
    return;
  }
}

// 測定間隔を定義（10秒）
#define MEASUREMENT_INTERVAL 10000

// 繰り返し実行されるloop関数
void loop() {
  // 距離を測定
  Ranger.Read();
  // 測定した距離を取得
  float distance = Ranger.Distance;
  // 測定距離をコンソールに表示
  console.print("distance: ");
  console.println(distance, 2); // 小数点以下2桁で表示

  // Soracomへデータを送信するための文字列を作成
  char data[64];
  sprintf(data, "{\"distance\":%.2f}", distance);

  // ソケットのオープンを試みる
  console.println("### Opening socket.");
  int connectId = Wio.SocketOpen("uni.soracom.io", 23080, WIOLTE_UDP);
  if (connectId < 0) {
    console.println("### ERROR: Failed to open socket.");
    return;
  }

  // データの送信を試みる
  console.println("### Sending data.");
  if (!Wio.SocketSend(connectId, data)) {
    console.println("### ERROR: Failed to send data.");
  } else {
    console.println("Data sent successfully.");
  }

  // ソケットのクローズを試みる
  console.println("### Closing socket.");
  if (!Wio.SocketClose(connectId)) {
    console.println("### ERROR: Failed to close socket.");
  }

  // 次の測定まで待機
  delay(MEASUREMENT_INTERVAL);
}
