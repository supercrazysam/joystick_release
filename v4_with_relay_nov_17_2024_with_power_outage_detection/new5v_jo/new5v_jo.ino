/*
 * Joystick reader — Arduino Mega 2560
 *  ▸ Reads 0–5 V analog joystick on A15
 *  ▸ Prints raw ADC value (0–1023) and calculated voltage
 */

const uint8_t JOYSTICK_PIN = A15;    // Analog input pin
const unsigned long SAMPLE_PERIOD_MS = 20;  // 50 Hz update

void setup() {
  Serial.begin(115200);              // Fast enough for frequent prints
  analogReference(DEFAULT);          // 5 V reference on the Mega 2560
  pinMode(JOYSTICK_PIN, INPUT);
}

void loop() {
  static unsigned long lastSample = 0;
  unsigned long now = millis();

  if (now - lastSample >= SAMPLE_PERIOD_MS) {
    lastSample = now;

    int raw = analogRead(JOYSTICK_PIN);               // 0-1023
    float volts = raw * (5.0 / 1023.0);               // map to 0-5 V

    // Print in a CSV-friendly format: raw,voltage
    Serial.print(raw);
    Serial.print(", ");
    Serial.println(volts, 3);       // 3-digit precision (e.g., 2.536 V)
    Serial.println(raw);       // 3-digit precision (e.g., 2.536 V)
  }
}
