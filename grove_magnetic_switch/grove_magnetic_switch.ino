#include <WioLTEforArduino.h>
#include <stdio.h>

#define INTERVAL        (60000)
#define RECEIVE_TIMEOUT (10000)
#define MAGNETIC_SWITCH_PIN (WIOLTE_D38)

WioLTE Wio;

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
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

  // Initialize magnetic switch pin
  pinMode(MAGNETIC_SWITCH_PIN, INPUT);

  SerialUSB.println("### Setup completed.");
}

void loop() {
  char data[100];

  // Read magnetic switch state
  int switchState = digitalRead(MAGNETIC_SWITCH_PIN);

  SerialUSB.print("Magnetic switch state = ");
  SerialUSB.println(switchState);

  // Prepare data for sending
  sprintf(data,"{\"magneticSwitch\":%d}", switchState);

  SerialUSB.println("### Open.");
  int connectId;
  connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

  SerialUSB.println("### Send.");
  SerialUSB.print("Send:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }

  // Rest of the code remains the same...

  // Error handling and delay
err_close:
  SerialUSB.println("### Close.");
  if (!Wio.SocketClose(connectId)) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

err:
  delay(INTERVAL);
}
