int pwmPin = 9; // Use Pin 9 for PWM output

void setup() {
    Serial.begin(9600); // For debugging purposes
    pinMode(pwmPin, OUTPUT);
}

void loop() {
    // Loop through PWM values from 0% to 100%
    for (int pwmValue = 26; pwmValue <= 230; pwmValue++) {   // 0   255
        analogWrite(pwmPin, pwmValue); // Output PWM signal
        delay(50); // 50ms delay between each step
        Serial.println(pwmValue);
    }

    // Loop through PWM values from 100% back down to 0%
    for (int pwmValue = 230; pwmValue >= 26; pwmValue--) {      //0   255
        analogWrite(pwmPin, pwmValue); // Output PWM signal
        delay(50); // 50ms delay between each step
        Serial.println(pwmValue);
    }
}
