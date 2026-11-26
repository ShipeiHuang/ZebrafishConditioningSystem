// ======================================================
// Electrode Voltage Manual Test Program
// Keeps stimulation voltage available for manual testing.
// Researchers should measure electrode pairs with a voltmeter,
// then manually turn off the external DC power supply.
// ======================================================

const int VoltageCircuit = 2;
const int LED_PIN = 13;

void setup() {
  pinMode(VoltageCircuit, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(VoltageCircuit, LOW);

  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  // Keep the signal constant.
  digitalWrite(VoltageCircuit, HIGH);
  digitalWrite(LED_PIN, HIGH);
}
