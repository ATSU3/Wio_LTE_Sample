#include <WioLTEforArduino.h>
#define console SerialUSB
WioLTE Wio;

#include <DHT.h>
#define DHTPIN WIOLTE_D38
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Wio.Init();
  console.begin(9600);
  while (!console);
  console.println("--- START");

  dht.begin();
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

#define MEASUREMENT_INTERVAL 10000

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // 不快指数を計算
  float discomfortIndex = 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;

  console.print("Humidity: ");
  console.print(humidity);
  console.print("%  Temperature: ");
  console.print(temperature);
  console.print("C  Discomfort Index: ");
  console.println(discomfortIndex); // 不快指数を出力

  char data[80]; // データ文字列のサイズを調整
  sprintf(data, "{\"temp\":%.2f,\"humi\":%.2f,\"discomfortIndex\":%.2f}", temperature, humidity, discomfortIndex);

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

  console.println("### Closing socket.");
  if (!Wio.SocketClose(connectId)) {
    console.println("### ERROR: Failed to close socket.");
  }

  delay(MEASUREMENT_INTERVAL);
}
