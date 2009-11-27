#include "Variables.h"
#include "Configuration.h"

//this is the version of our host software
unsigned int master_version = 0;

//are we paused?
bool is_tool_paused = false;

// The temperature controller(s)
Heater extruder_heater;
#if HAS_HEATED_BUILD_PLATFORM
Heater platform_heater;
#endif

int heater_low = 64;
int heater_high = 255;

MotorControlStyle motor1_control = MC_PWM;
volatile MotorControlDirection motor1_dir = MC_FORWARD;
volatile byte motor1_pwm = 0;
volatile long motor1_target_rpm = 0;
volatile long motor1_current_rpm = 0;
bool motor1_reversal_state = false;
int motor1_reversal_count = 0;

MotorControlStyle motor2_control = MC_PWM;
MotorControlDirection motor2_dir = MC_FORWARD;
byte motor2_pwm = 0;
long motor2_target_rpm = 0;
long motor2_current_rpm = 0;

#if HAS_I2C_LCD
I2cLCD lcd;
#else // We have to save memory in case of HAS_I2C_LCD
Servo servo1;
Servo servo2;
#endif

//these are for the extruder PID
volatile int speed_error = 0;              // extruder position / error variable.
volatile int pGain = SPEED_INITIAL_PGAIN;  // Proportional gain
volatile int iGain = SPEED_INITIAL_IGAIN;  // Integral gain
volatile int dGain = SPEED_INITIAL_DGAIN;  // Derivative gain
volatile int iMax = 500;                   // Integrator max
volatile int iMin = -500;                  // Integrator min
volatile int iState = 0;                   // Integrator state
volatile int dState = 0;                   // Last position input

//what state are we in?
volatile byte stepper_index = 0;
volatile long stepper_ticks = 0;
volatile byte stepper_high_pwm = 0;
volatile byte stepper_low_pwm = 0;


  

