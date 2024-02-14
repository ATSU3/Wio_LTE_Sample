// Wio LTEボードと連携するためのライブラリをインクルード
#include <WioLTEforArduino.h>
// USB経由でのデバッグ出力用にSerialUSBをconsoleとして定義
#define console SerialUSB
// Wio LTEオブジェクトを作成
WioLTE Wio;

// 温湿度センサーを扱うための定義
#include <DHT.h>

// 温湿度センサーのピン番号定義
#define DHTPIN WIOLTE_D38
// 温湿度センサーのタイプ定義（DHT11, DHT22等）
#define DHTTYPE DHT11
// DHTオブジェクトのインスタンス化
DHT dht(DHTPIN, DHTTYPE);

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

  // 温湿度センサーの初期化
  dht.begin();

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
  // 温湿度を測定
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // 測定データをコンソールに表示
  console.print("Humidity: ");
  console.print(humidity);
  console.print("%  Temperature: ");
  console.print(temperature);
  console.println("C");

  // Soracomへデータを送信するための文字列を作成
  char data[64];
  sprintf(data, "{\"temp\":%.2f,\"humi\":%.2f}", temperature, humidity);

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
