#include <WioLTEforArduino.h>
#define console SerialUSB
WioLTE Wio;

#include <GroveDriverPack.h>
GroveBoard Board;
GroveUltrasonicRanger Ranger(&Board.D38);

void setup() {
  Wio.Init();
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

#define MEASUREMENT_INTERVAL 10000 // 1 minute in milliseconds

void loop() {
  // Measure the distance to the water surface
  Ranger.Read();
  float distance = Ranger.Distance;
  console.print("Water level distance: "); console.println(distance);

  // Send the data to Soracom
  char payload[100];
  int res_code;
  sprintf(payload, "{\"water_level\":%.2f}", distance);
  if (!Wio.HttpPost("http://uni.soracom.io", payload, &res_code)) {
    console.println("### ERROR! ###");
  } else {
    console.print("Data sent. Status: ");
    console.println(res_code);
  }

  // Wait for next measurement
  delay(MEASUREMENT_INTERVAL);
}
