#include <Servo.h>
Servo myServo;  // create servo object to control a servo

const int powerOutagePin = 46;  // Pin connected to the UPS power_outage pin

const int joystickPin = 5; // Pin where the joystick PWM signal is connected    (D2 arduino mega 2560)
const int numReadings = 5; // Number of readings to average

unsigned long readings[numReadings]; // Array to store the readings
int readIndex = 0; // Index of the current reading
unsigned long total = 0; // Total of the readings
unsigned long average = 0; // Average of the readings

float joystick_max = 940.0;//true 950   //v3 true joystick max =940   //900.0;//v2 true joystick max become 950 ///// //600.0;//650.0; //650.0;
float joystick_min = 110.0; //true 100  //v3 true joystick min =144  //120.0;//v2 true joystick min become 110 ///// //48.0;

float joystick_mid = joystick_min + ((joystick_max - joystick_min)/2);

int safe_start = 0;

int safe_resume_back_to_manual = 0;

// Define relay control pins
const int relay_neutral = 11;  // neutral Relay connected to pin 11
const int relay_reverse = 12;  // reverse Relay connected to pin 12

//upstream control command (dutycycle,   upstream analogWrite)
int upstream_analogPin = 13; // The digital pin where you read the PWM analog signal from upstream 
float upstream_dutycycle_value  = 0.0;

float upstream_max_forward = 90.0;   //90%
float upstream_max_middle  = 50.0;   //50%
float upstream_max_reverse = 10.0;   //10%

//  1375 is the top limit      1545 is the position of mid point, dont change
// increase the value to move the top limit down, decrease the value to move the top limit up.  
// ==================
float top_limit = 1435; //1375;    // increase the top_limit value to move the top limit down, decrease the value to move the top limit up.  
//  1665 is the bottom limit      1545 is the position of mid point, dont change
float bottom_limit = 1620; //1665;     // increase the bottom_limit value to move the bottom_limit down, decrease the value to move the bottom limit up.  

float center_position = 1535;   //increase the center_position value to move the center_position down, decrease the value to move the center_position up. 

//====================================================================//

void setup() {
    // Initialize all readings to 0
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
  
  Serial.begin(9600); // Start serial communication at 9600 baud
  pinMode(joystickPin, INPUT); // Set joystick pin as input
  myServo.attach(9);  // attaches the servo on pin 9 to the servo object
  
  // Initialize the powerOutagePin as an input
  pinMode(powerOutagePin, INPUT);

  pinMode(upstream_analogPin, INPUT_PULLUP);  // The digital pin where you read the PWM analog signal from upstream 

  // Set relay pins as outputs
  pinMode(relay_neutral, OUTPUT);
  pinMode(relay_reverse, OUTPUT);

  // Initialize relays to high (HIGH turns them off)
  digitalWrite(relay_neutral, HIGH);   //all relay should be off (HIGH state) on startup by default
  digitalWrite(relay_reverse, HIGH);   //all relay should be off (HIGH state) on startup by default
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

      myServo.writeMicroseconds(center_position);  //always set motor to zero position before joystick have returned to zero 
      if (mappedValue==0) 
      { 
        safe_start=1;
        Serial.print("****Reached_safety_region****");
      }
       
  }
  else
  {
    //see if we should override the manual control  ( we override if we get 5% - 95% dutycycle from upstream,   else just do manual)
    // Read the analog value (0-1023)
    float dutyCycle = (pulseIn(upstream_analogPin, HIGH, 100000 )/ 2040.82)*100.0; // mega 2560 normal pin by default = 490hz    timeout 0.1s
    upstream_dutycycle_value = constrain( dutyCycle, 0.0, 100.0);

    //if ((upstream_dutycycle_value>=10.0) &&  (upstream_dutycycle_value<=90.0))  //auto upstream override mode, only when its between 10% and 90%
    
    //if ((upstream_dutycycle_value>=5.0) &&  (upstream_dutycycle_value<=95.0))  //auto upstream override mode, only when its between 10% and 90%
    if (  ((upstream_dutycycle_value>=5.0) &&  (upstream_dutycycle_value<=47.5))  || ((upstream_dutycycle_value>=52.5) &&  (upstream_dutycycle_value<=95.0))  )
      
    //50.0 is middle   10.0 is max backward   90.0 is max forward
    {
      float override_pwm_value = map_upstream(upstream_dutycycle_value);
      Serial.print("[Override from upstream detected] dutycycle = ");
      Serial.print(upstream_dutycycle_value);
      Serial.print("[Override from upstream detected] Mapped Value: ");
      Serial.println(override_pwm_value);
      myServo.writeMicroseconds(override_pwm_value);

      safe_resume_back_to_manual = 0;  //it already went into upstream override mode, so reset manual flag
    }
    else  //manual mode
    {
       if (safe_resume_back_to_manual==0)   //seems like the system just started, or just resume back into manual from upstream override
       {
           float resume_manual_mappedValue = safe_start_mapJoystick(average);
           myServo.writeMicroseconds(center_position);  //always set motor to zero position before joystick have returned to zero
           if (resume_manual_mappedValue==0) 
           {
               safe_resume_back_to_manual = 1;
           }
       }
       else // when its safe, continue manual mode
       {
           float mappedValue = mapJoystick(average);
           Serial.print("Joystick Value: ");
           Serial.print(average);
           Serial.print(" Mapped Value: ");
           Serial.println(mappedValue);
           myServo.writeMicroseconds(mappedValue);  // set the servo to the current PWM value
       }
    }
  }

  delay(10); // Small delay to slow down the output
}

// Function to map joystick value to -1 to 1 range
float safe_start_mapJoystick(unsigned long value) {
  // Map the pulse width value (corresponding to 48.0 to 650.0) to the range of -1 to 1
  // Adjust the range values if needed      //48.0  back effective range    0.0 = no output    710.0 = all the way forward, effective range
  float mappedValue = constrain((((float)(value - joystick_min) / (joystick_max - joystick_min)) * 2.0 - 1.0), -1.0, 1.0);
  if ((mappedValue<0.15)&&(mappedValue>-0.15)) 
      { 
        digitalWrite(relay_neutral, LOW);    //joystick reaching initialize zero zone, turn on the netural relay
        return 0.0; 
      }
  else
      {
        digitalWrite(relay_neutral, HIGH);   //joystick not reaching initialize zero zone yet, dont turn on the netural relay  
        return mappedValue; 
      }
}

float mapJoystick(unsigned long value) {

  
  ///////////////////////////
  //relay output section while its in main operation loop, mimicking the logic from initialize section
  float mappedValue = constrain((((float)(value - joystick_min) / (joystick_max - joystick_min)) * 2.0 - 1.0), -1.0, 1.0);
  if ((mappedValue<0.2)&&(mappedValue>-0.2))   //original 0.15
      { 
        digitalWrite(relay_neutral, LOW);    //joystick reaching initialize zero zone, turn on the netural relay
      }
  else
      {
        digitalWrite(relay_neutral, HIGH);   //joystick not reaching initialize zero zone yet, dont turn on the netural relay  
      }

  /////////////////////////////////////    
  //universal map 
  //    1455 up         1545 mid   1630  down   aug 12 2024
  //v1: 1425 up         1545 mid   1640  down
  //v2: 1375 up         1545 mid   1665  down   oct 6 2024
  if (value<=joystick_mid)
  {
        float pwmValue = map(value, joystick_min, joystick_mid, top_limit, center_position);    

        //determine when the backward relay gets trigger:
        float backward_relay_trigger = 1520;    //right now middle is 1545,  
        //so if we want to trigger it when middle then set this to 1545,  setting value less than 1545 will make it only trigger further back.
        if (pwmValue< backward_relay_trigger)
        {
            digitalWrite(relay_reverse, LOW);    //going backward enough, time to turn on the reverse relay (LOW state)
        }
        else
        {
            digitalWrite(relay_reverse, HIGH);    //not going backward enough, turn off the reverse relay  (HIGH state)
        }
        return pwmValue;
  }
  else if (value > joystick_mid)
  {
        float pwmValue = map(value, joystick_mid, joystick_max, center_position, bottom_limit);   

        digitalWrite(relay_reverse, HIGH);    //going forward, turn off the reverse relay
        return pwmValue;
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//UPSTREAM OVERRIDE SECTION!!!

float map_upstream(float value) {

  float upstream_command_value = constrain(value, 10.0, 90.0);    //from 10% (max reverse) to 90%  (max forward)
  if ((upstream_command_value<60.0)&&(upstream_command_value>40.0))   //assume neutral if its anywhere between 45% to 55%  ~50%
      { 
        digitalWrite(relay_neutral, LOW);    //joystick reaching initialize zero zone, turn on the netural relay
      }
  else
      {
        digitalWrite(relay_neutral, HIGH);   //joystick not reaching initialize zero zone yet, dont turn on the netural relay  
      }

  if (upstream_command_value<=upstream_max_middle)   //going backward,  since lower than 50% dutycycle
  {   
        float override_pwmValue = map(upstream_command_value, upstream_max_reverse, upstream_max_middle, top_limit, center_position);    //mapping the reverse range to actual slider travel    

        //determine when the backward relay gets trigger in auto upstream override mode:
        float upstream_backward_relay_trigger = 45.0;   //when its less than 45%, engage the reverse relay
        if (upstream_command_value< upstream_backward_relay_trigger)
        {
            digitalWrite(relay_reverse, LOW);    //going backward enough, time to turn on the reverse relay (LOW state)
        }
        else
        {
            digitalWrite(relay_reverse, HIGH);    //not going backward enough, turn off the reverse relay  (HIGH state)
        }
        return override_pwmValue;
  }
  else if (upstream_command_value > upstream_max_middle)
  {  
        float override_pwmValue = map(upstream_command_value, upstream_max_middle, upstream_max_forward, center_position, bottom_limit);   

        digitalWrite(relay_reverse, HIGH);    //going forward, turn off the reverse relay
        return override_pwmValue;
  }

}
