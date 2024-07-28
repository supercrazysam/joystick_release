#include <Servo.h>
Servo myServo;  // create servo object to control a servo

/////////////////////////
float joystick_max_pwm = 1550.0;   //slider at bottom position
float joystick_min_pwm = 1400.0;   //slider at top position

float joystick_mid_pwm = 1460.0;
/////////////////////////

String user_input;
float linear_speed=0.0;
float input_speed=0.0;
float pwmValue = joystick_mid_pwm;
float motor_pwm_target_value = joystick_mid_pwm;


float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
  
void setup() {

  Serial.begin(9600); // Start serial communication at 9600 baud
  Serial.setTimeout(10);
  myServo.attach(9);  // attaches the servo on pin 9 to the servo object
  delay(1000);
  myServo.writeMicroseconds(joystick_mid_pwm);  //always set motor to zero position before operation
  delay(1000);
}

void loop() 
{

  if (motor_pwm_target_value==joystick_mid_pwm)
    {
       Serial.println(pwmValue);
       pwmValue = motor_pwm_target_value;
       myServo.writeMicroseconds(pwmValue);
    }
    else if ((motor_pwm_target_value - pwmValue)>2.0)
    {
       pwmValue+=0.5;
       Serial.println(pwmValue);
       myServo.writeMicroseconds(pwmValue);
    }
    else if ((pwmValue - motor_pwm_target_value)>2.0)
    {
       pwmValue-=0.5;
       Serial.println(pwmValue);
       myServo.writeMicroseconds(pwmValue);
    }  

    else if (pwmValue == motor_pwm_target_value)
    {
       Serial.println(pwmValue);
    }
    delay(5);
  if (Serial.available())
  {
    input_speed = Serial.parseFloat();
    linear_speed = constrain(input_speed, -1.0, 1.0);
    
    if (linear_speed>0.0)
    {
        motor_pwm_target_value = fmap(linear_speed, 0.0, 1.0, joystick_mid_pwm,joystick_max_pwm);
    }
    else if (linear_speed<0.0)
    {
        motor_pwm_target_value = fmap(linear_speed, -1.0, 0.0, joystick_min_pwm, joystick_mid_pwm);
    }
    else
    {
        motor_pwm_target_value = joystick_mid_pwm;

    }
    
    Serial.print("target speed is: ");
    Serial.print(linear_speed, 2);
    Serial.print("  [-1.0 to 1.0], output value = ");
    Serial.println(pwmValue);
    
    delay(5);
  }
}
