//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Remote Cannybot - Blue Brain
//
// Authors:  Brian Corteil based on a sketch by Wayne Keenan 
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  28.10.2014  -  Inital Version  ()
//
//////////////////////////////////////////////////////////////////////////////////////////////////

#define GZLL_HOST_ADDRESS 0x12ABCD00           // this needs to match the Joypad sketch value

#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
#include <math.h>

// Motor Pins
#define MOTOR_A1_PIN                 0		// motor direction
#define MOTOR_A2_PIN                 6  	// motor speed (PWM)
#define MOTOR_B1_PIN                 1		// motor direction
#define MOTOR_B2_PIN                 5		// motor speed (PWM)
#define MOTOR_MAX_SPEED            255		// max speed (0..255)

//#define JOYPAD_AXIS_DEADZONE 	    10

#define JOYPAD_Y_AXIS_DEADZONE      10          // motor speed
#define JOYPAD_X_AXIS_DEADZONE      10          // steering

// Joypad
volatile int16_t  xAxisValue    = 0;              // (left) -255 .. 255 (right)
volatile int16_t  yAxisValue    = 0;              // (down) -255 .. 255 (up)
volatile bool     buttonPressed = 0;              // 0 = not pressed, 1 = pressed

void setup() {
  Serial.end();

  // Motor pins
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);
  
  

  radio_setup();
  
  // Serial Setup
  
  Serial.begin(9600, 2,3);
  Serial.println("Hello");
  delay(1000);
}

void loop() {
  radio_loop();    
  //remoteControl(yAxisValue, xAxisValue);
  Serial.println(yAxisValue);
  delay(250);
  
}

void remoteControl(int power,int steering) {
  if ( steering > abs(JOYPAD_X_AXIS_DEADZONE)) motorSpeed(power , (power / 2 ));
  if ( steering < -abs(JOYPAD_X_AXIS_DEADZONE)) motorSpeed((power / 2), power);
  if ( steering == 0 ) motorSpeed(power , power);  
}
  
void motorSpeed(int _speedA, int _speedB) {
  _speedA = constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  digitalWrite(MOTOR_A1_PIN, _speedA >= 0 ? HIGH : LOW) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));

  digitalWrite(MOTOR_B1_PIN, _speedB >= 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
}


void joypad_update(int x, int y, int b) {
  // If the axis readings are small, in the 'deadzone', set them to 0
  if ( abs(x) < JOYPAD_X_AXIS_DEADZONE)  x = 0;
  if ( abs(y) < JOYPAD_Y_AXIS_DEADZONE)  y = 0;

  xAxisValue    = x;
  yAxisValue    = y;
  buttonPressed = b;
}

