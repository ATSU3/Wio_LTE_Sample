#include <WioLTEforArduino.h>
#include <stdio.h>

#define INTERVAL            (10000)
#define RECEIVE_TIMEOUT     (10000)
#define MAGNETIC_SWITCH_PIN (WIOLTE_D38) // 磁気スイッチのピン定義

WioLTE Wio;

void setup() {
  delay(200);

  SerialUSB.begin(115200);
  while (!SerialUSB); // For Native USB
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(500);

  pinMode(MAGNETIC_SWITCH_PIN, INPUT); // 磁気センサのピンを入力として初期化

  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Setup completed.");
}

void loop() {
  int switchState = digitalRead(MAGNETIC_SWITCH_PIN);
  char data[100];
  sprintf(data, "{\"switchState\":%d}", switchState);

  SerialUSB.println("### Attempting to send data.");
  if (!sendDataToSoracom(data)) {
    SerialUSB.println("### Sending data failed. Attempting to reconnect.");
    reconnect();
  }

  delay(INTERVAL);
}

bool sendDataToSoracom(const char* data) {
  int connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR opening socket!");
    return false;
  }

  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR sending data!");
    Wio.SocketClose(connectId);
    return false;
  }

  Wio.SocketClose(connectId);
  SerialUSB.println("### Data sent successfully.");
  return true;
}

void reconnect() {
  Wio.PowerSupplyLTE(false); // LTEモジュールの電源をOFF
  delay(1000); // 少し待つ
  Wio.PowerSupplyLTE(true); // LTEモジュールの電源をON
  delay(1000); // モジュールが起動するのを待つ

  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR during TurnOnOrReset.");
    return;
  }

  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR during Activate.");
    return;
  }

  SerialUSB.println("### Reconnected.");
}
