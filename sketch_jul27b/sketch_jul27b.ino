#include <Servo.h>
Servo myServo;  // create servo object to control a servo

const int joystickPin = 5; // Pin where the joystick PWM signal is connected    (D2 arduino mega 2560)
const int numReadings = 3; // Number of readings to average

unsigned long readings[numReadings]; // Array to store the readings
int readIndex = 0; // Index of the current reading
unsigned long total = 0; // Total of the readings
unsigned long average = 0; // Average of the readings

float joystick_max = 600.0;//650.0; //650.0;
float joystick_min = 48.0;

float joystick_mid = joystick_min + ((joystick_max - joystick_min)/2);

int safe_start = 0;

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud
  pinMode(joystickPin, INPUT); // Set joystick pin as input
  myServo.attach(9);  // attaches the servo on pin 9 to the servo object

  // Initialize all readings to 0
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
}

void loop() {
  // Subtract the last readingmyServo.writeMicroseconds(pwmValue);  // set the servo to the current PWM value
  total -= readings[readIndex];
  
  // Read the pulse width of the PWM signal
  readings[readIndex] = pulseIn(joystickPin, HIGH);
  //Serial.println(readings[readIndex]);
  
  // Add the new reading to the total
  total += readings[readIndex];
  
  // Advance to the next position in the array
  readIndex = (readIndex + 1) % numReadings;
  
  // Calculate the average
  average = total / numReadings;

  if (safe_start==0)
  {
      // Map the average value to the range of -1 to 1
      float mappedValue = safe_start_mapJoystick(average);
  
      // Print the joystick value and mapped value
      Serial.print("before Joystick Value: ");
      Serial.print(average);
      Serial.print("before Mapped Value: ");
      Serial.println(mappedValue);

      myServo.writeMicroseconds(1460);  //always set motor to zero position before joystick have returned to zero 
      if (mappedValue==0) 
      { 
        safe_start=1;
        Serial.print("****Reached_safety_region****");
      }
       
  }
  else
  {
       float mappedValue = mapJoystick(average);
       Serial.print("Joystick Value: ");
       Serial.print(average);
       Serial.print(" Mapped Value: ");
       Serial.println(mappedValue);
       myServo.writeMicroseconds(mappedValue);  // set the servo to the current PWM value
  }

  delay(10); // Small delay to slow down the output
}

// Function to map joystick value to -1 to 1 range
float safe_start_mapJoystick(unsigned long value) {
  // Map the pulse width value (corresponding to 48.0 to 650.0) to the range of -1 to 1
  // Adjust the range values if needed      //48.0  back effective range    0.0 = no output    710.0 = all the way forward, effective range
  float mappedValue = constrain((((float)(value - joystick_min) / (joystick_max - joystick_min)) * 2.0 - 1.0), -1.0, 1.0);
  if ((mappedValue<0.15)&&(mappedValue>-0.15))
      { return 0.0; }
  else
      { return mappedValue; }
}

float mapJoystick(unsigned long value) {
  //universal map 
  //1455 up    1545 mid   1630  down   aug 12 2024
  if (value<=joystick_mid)
  {
        //  1455 is the top limit      1545 is the position of mid point, dont change
        // increase the value to move the top limit down, decrease the value to move the top limit up.  
        // ==================
        float top_limit = 1455;    // increase the top_limit value to move the top limit down, decrease the value to move the top limit up.  
        float pwmValue = map(value, joystick_min, joystick_mid, top_limit, 1545);    
        return pwmValue;
  }
  else if (value > joystick_mid)
  {
        //  1625 is the bottom limit      1545 is the position of mid point, dont change
        float bottom_limit = 1625;     // increase the bottom_limit value to move the bottom_limit down, decrease the value to move the top limit up.  
        float pwmValue = map(value, joystick_mid, joystick_max, 1545, bottom_limit);   
        return pwmValue;
  }
  
}
