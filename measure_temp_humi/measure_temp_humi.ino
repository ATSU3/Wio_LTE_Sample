#include <WioLTEforArduino.h> // Wio LTEボードのためのライブラリをインクルードします。
#include <stdio.h> // 標準入出力関連の機能を利用するためのライブラリをインクルードします。

#define INTERVAL        (60000) // メインループの間隔をミリ秒単位で定義します。
#define RECEIVE_TIMEOUT (10000) // 受信タイムアウトをミリ秒単位で定義します。

#define SENSOR_PIN    (WIOLTE_D38) // 温度と湿度センサーを接続するピンを定義します。

WioLTE Wio; // Wio LTEボードのインスタンスを作成します。

void setup() { // Arduinoのセットアップ関数です。
  delay(200);

  SerialUSB.println(""); // USB経由でPCに文字列を出力します。
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize."); // I/Oの初期化を開始します。
  Wio.Init(); // Wio LTEボードの初期化を行います。

  SerialUSB.println("### Power supply ON."); // 電源をONにします。
  Wio.PowerSupplyLTE(true); // LTEモジュールの電源を供給します。
  delay(500);

  SerialUSB.println("### Turn on or reset."); // ボードの電源を入れるかリセットします。
  if (!Wio.TurnOnOrReset()) { // ボードの電源を入れるかリセットを試み、失敗した場合はエラーメッセージを出力します。
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connecting to \"soracom.io\"."); // Soracomに接続します。
  if (!Wio.Activate("soracom.io", "sora", "sora")) { // Soracomに接続を試み、失敗した場合はエラーメッセージを出力します。
    SerialUSB.println("### ERROR! ###");
    return;
  }

#ifdef SENSOR_PIN
  TemperatureAndHumidityBegin(SENSOR_PIN); // 温度と湿度センサーの初期化を行います。
#endif // SENSOR_PIN

  SerialUSB.println("### Setup completed."); // セットアップが完了しました。
}

void loop() { // Arduinoのメインループ関数です。
  char data[1024]; // 送信するデータを格納する配列を定義します。

#ifdef SENSOR_PIN
  float temp; // 温度を格納する変数を定義します。
  float humi; // 湿度を格納する変数を定義します。

  if (!TemperatureAndHumidityRead(&temp, &humi)) { // 温度と湿度の読み取りを試み、失敗した場合はエラーメッセージを出力してエラーラベルにジャンプします。
    SerialUSB.println("ERROR!");
    goto err;
  }

  SerialUSB.print("Current humidity = "); // 現在の湿度を出力します。
  SerialUSB.print(humi);
  SerialUSB.print("%  ");
  SerialUSB.print("temperature = "); // 現在の温度を出力します。
  SerialUSB.print(temp);
  SerialUSB.println("C");

  sprintf(data,"{\"temp\":%.1f,\"humi\":%.1f}", temp, humi); // JSON形式で温度と湿度のデータを文字列に格納します。
#else
  sprintf(data, "{\"uptime\":%lu}", millis() / 1000); // SENSOR_PINが定義されていない場合、システムの稼働時間をJSON形式で格納します。
#endif // SENSOR_PIN

  SerialUSB.println("### Open."); // ソケットを開く処理を開始します。
  int connectId;
  connectId = Wio.SocketOpen("uni.soracom.io", 23080, WIOLTE_UDP); // SoracomにUDPで接続を試みます。
  if (connectId < 0) { // 接続に失敗した場合、エラーメッセージを出力してエラーラベルにジャンプします。
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

  SerialUSB.println("### Send."); // データ送信を開始します。
  SerialUSB.print("Send:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.SocketSend(connectId, data)) { // データの送信を試み、失敗した場合はエラーメッセージを出力してクローズラベルにジャンプします。
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }

  SerialUSB.println("### Receive."); // データ受信を開始します。
  int length;
  length = Wio.SocketReceive(connectId, data, sizeof (data), RECEIVE_TIMEOUT); // 受信データを読み取ります。
  if (length < 0) { // 受信に失敗した場合、エラーメッセージを出力してクローズラベルにジャンプします。
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }
  if (length == 0) { // 受信データがない場合、タイムアウトメッセージを出力してクローズラベルにジャンプします。
    SerialUSB.println("### RECEIVE TIMEOUT! ###");
    goto err_close;
  }
  SerialUSB.print("Receive:"); // 受信したデータを出力します。
  SerialUSB.print(data);
  SerialUSB.println("");

err_close:
  SerialUSB.println("### Close."); // ソケットを閉じる処理を開始します。
  if (!Wio.SocketClose(connectId)) { // ソケットの閉じる操作を試み、失敗した場合はエラーメッセージを出力してエラーラベルにジャンプします。
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

err:
  delay(INTERVAL); // 次のループまでの間隔を待機します。
}

////////////////////////////////////////////////////////////////////////////////////////
//

#ifdef SENSOR_PIN

int TemperatureAndHumidityPin; // 温度と湿度センサーが接続されているピンの番号を格納する変数です。

void TemperatureAndHumidityBegin(int pin) // 温度と湿度センサーの初期化を行う関数です。
{
  TemperatureAndHumidityPin = pin; // 温度と湿度センサーが接続されているピンの番号を設定します。
  DHT11Init(TemperatureAndHumidityPin); // DHT11センサーの初期化を行います。
}

bool TemperatureAndHumidityRead(float* temperature, float* humidity) // 温度と湿度を読み取る関数です。
{
  byte data[5]; // センサーから読み取ったデータを格納する配列です。

  DHT11Start(TemperatureAndHumidityPin); // DHT11センサーの読み取りを開始します。
  for (int i = 0; i < 5; i++) data[i] = DHT11ReadByte(TemperatureAndHumidityPin); // 5バイトのデータを読み取ります。
  DHT11Finish(TemperatureAndHumidityPin); // DHT11センサーの読み取りを終了します。

  if(!DHT11Check(data, sizeof (data))) return false; // データのチェックサムを確認し、不正な場合はfalseを返します。
  if (data[1] >= 10) return false; // 湿度の小数部が10以上の場合は不正とみなし、falseを返します。
  if (data[3] >= 10) return false; // 温度の小数部が10以上の場合は不正とみなし、falseを返します。

  *humidity = (float)data[0] + (float)data[1] / 10.0f; // 湿度の値を計算します。
  *temperature = (float)data[2] + (float)data[3] / 10.0f; // 温度の値を計算します。

  return true; // 正常に読み取りが完了した場合はtrueを返します。
}

#endif // SENSOR_PIN

////////////////////////////////////////////////////////////////////////////////////////
//

#ifdef SENSOR_PIN

void DHT11Init(int pin) // DHT11センサーの初期化を行う関数です。
{
  digitalWrite(pin, HIGH); // ピンをHIGHに設定して、センサーを初期化します。
  pinMode(pin, OUTPUT); // ピンのモードをOUTPUTに設定します。
}

void DHT11Start(int pin) // DHT11センサーの読み取りを開始するためのシグナルを送信する関数です。
{
  // ホストからスタートシグナルを送信します。
  digitalWrite(pin, LOW); // ピンをLOWに設定します。
  delay(18); // 18ミリ秒待機します。

  // ピンをINPUTモードに設定して、センサーからの応答を待ちます。
  pinMode(pin, INPUT);
  while (!digitalRead(pin)) ; // ピンがHIGHになるまで待ちます。

  // センサーからの応答シグナルを待ちます。
  while (digitalRead(pin)) ; // ピンがLOWになるまで待ちます。

  // センサーがデータを送信する準備ができたことを示します。
  while (!digitalRead(pin)) ; // ピンがHIGHになるまで待ちます。
}

byte DHT11ReadByte(int pin) // DHT11センサーから1バイトのデータを読み取る関数です。
{
  byte data = 0; // 読み取ったデータを格納する変数です。

  for (int i = 0; i < 8; i++) { // 8ビットのデータを読み取ります。
    while (digitalRead(pin)) ; // ピンがLOWになるまで待ちます。

    while (!digitalRead(pin)) ; // ピンがHIGHになるまで待ちます。
    unsigned long start = micros(); // HIGHになった時刻を記録します。

    while (digitalRead(pin)) ; // ピンが再びLOWになるまで待ちます。
    unsigned long finish = micros(); // LOWになった時刻を記録します。

    if ((unsigned long)(finish - start) > 50) data |= 1 << (7 - i); // HIGH状態の持続時間が50マイクロ秒以上であれば、対応するビットを1に設定します。
  }

  return data; // 読み取ったデータを返します。
}

void DHT11Finish(int pin) // DHT11センサーの読み取りを終了する関数です。
{
  // バスを解放します。
  while (!digitalRead(pin)) ; // ピンがHIGHになるまで待ちます。
  digitalWrite(pin, HIGH); // ピンをHIGHに設定します。
  pinMode(pin, OUTPUT); // ピンのモードをOUTPUTに戻します。
}

bool DHT11Check(const byte* data, int dataSize) // DHT11から読み取ったデータのチェックサムを検証する関数です。
{
  if (dataSize != 5) return false; // データサイズが5バイトでなければfalseを返します。

  byte sum = 0; // チェックサムを計算するための変数です。
  for (int i = 0; i < dataSize - 1; i++) {
    sum += data[i]; // 最後のバイトを除くデータの合計を計算します。
  }

  return data[dataSize - 1] == sum; // 計算したチェックサムと受信したチェックサムが一致するかを検証します。
}

#endif // SENSOR_PIN
