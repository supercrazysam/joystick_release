int pwmInputPin = 13; 
unsigned long duration; // Variable to store the duration of the HIGH pulse
float dutyCycle; // Variable to store the duty cycle as a percentage

void setup() {
    pinMode(pwmInputPin, INPUT); // Set the pin as input
    Serial.begin(9600); // Initialize serial communication for debugging
}

void loop() {
    // Measure the length of the HIGH pulse (in microseconds)
    duration = pulseIn(pwmInputPin, HIGH);


    dutyCycle = duration / 2040.82; // mega 2560 normal pin by default = 490hz   The period for a 490 Hz signal is approximately 2040.82 microseconds
    dutyCycle = constrain( dutyCycle, 0.0, 1.0);

    // Print the duty cycle as a float (0.0 to 1.0)
    Serial.println(dutyCycle);

    delay(100); // Adjust delay as needed
}
