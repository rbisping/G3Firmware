#include "Configuration.h"
#include "Heater.h"
#include "Variables.h"
#include "ThermistorTable.h"

void Heater::init(int inPin, int outPin, bool isThermocoupler)
{
  usesThermocoupler = isThermocoupler;
  inputPin = inPin;
  outputPin = outPin;

  temp_control_enabled = true;
#if TEMP_PID
  temp_iState = 0;
  temp_dState = 0;
  temp_pGain = TEMP_PID_PGAIN;
  temp_iGain = TEMP_PID_IGAIN;
  temp_dGain = TEMP_PID_DGAIN;
  
  temp_pid_update_windup();
#endif

  current_temperature =  0;
  target_temperature =  0;
  max_temperature =  0;
}

void Heater::set_target_temperature(int temp)
{
  target_temperature = temp;
  max_temperature = (int)((float)temp * 1.1);
}

bool Heater::hasReachedTargetTemperature()
{
  return (current_temperature > (int)(target_temperature * 0.95));
}

/**
 *  Samples the temperature and converts it to degrees celsius.
 *  Returns degrees celsius.
 */
int Heater::get_current_temperature()
{
  if(usesThermocoupler)
    return read_thermocouple();
  else
    return read_thermistor();
}

/*
* This function gives us the temperature from the thermistor in Celsius
 */
int Heater::read_thermistor()
{
  int raw = sample_temperature();

  int celsius = 0;
  byte i;

  for (i=1; i<NUMTEMPS; i++)
  {
    if (temptable[i][0] > raw)
    {
      celsius  = temptable[i-1][1] + 
        (raw - temptable[i-1][0]) * 
        (temptable[i][1] - temptable[i-1][1]) /
        (temptable[i][0] - temptable[i-1][0]);

      if (celsius > 255)
        celsius = 255; 

      break;
    }
  }

  // Overflow: We just clamp to 0 degrees celsius
  if (i == NUMTEMPS)
    celsius = 0;

  return celsius;
}

/*
* This function gives us the temperature from the thermocouple in Celsius
 */
int Heater::read_thermocouple()
{
  return ( 5.0 * sample_temperature() * 100.0) / 1024.0;
}

/*
* This function gives us an averaged sample of the analog temperature pin.
 */
int Heater::sample_temperature()
{
  int raw = 0;

  //read in a certain number of samples
  for (byte i=0; i<TEMPERATURE_SAMPLES; i++)
    raw += analogRead(inputPin);

  //average the samples
  raw = raw/TEMPERATURE_SAMPLES;

  //send it back.
  return raw;
}


/*!
 Manages motor and heater based on measured temperature:
 o If temp is too low, don't start the motor
 o Adjust the heater power to keep the temperature at the target
 */
void Heater::manage_temperature()
{
  int output, dt;
  unsigned long time;

  //make sure we know what our temp is.
  current_temperature = get_current_temperature();
    
  // ignoring millis rollover for now
  time = millis();
  dt = time - temp_prev_time;

  if (dt > TEMP_UPDATE_INTERVAL)
  { 
    temp_prev_time = time;
    output = temp_update(dt);
    analogWrite(outputPin, output);
  }
}


#if TEMP_PID
int Heater::temp_update(int dt)
{
  int output;
  int error;
  float pTerm, iTerm, dTerm;
  
  if (temp_control_enabled) {
    error = target_temperature - current_temperature;
    
    pTerm = temp_pGain * error;
    
    temp_iState += error;
    temp_iState = constrain(temp_iState, temp_iState_min, temp_iState_max);
    iTerm = temp_iGain * temp_iState;
    
    dTerm = temp_dGain * (current_temperature - temp_dState);
    temp_dState = current_temperature;
    
    output = pTerm + iTerm - dTerm;
    output = constrain(output, 0, 255);
  } else {
    output = 0;
  }
  return output;
}
 
void Heater::temp_pid_update_windup()
{
  temp_iState_min = -TEMP_PID_INTEGRAL_DRIVE_MAX/temp_iGain;
  temp_iState_max =  TEMP_PID_INTEGRAL_DRIVE_MAX/temp_iGain;
}

#else

int Heater::temp_update(int dt)
{
  int output;
  
  if (temp_control_enabled) {
    //put the heater into high mode if we're not at our target.
    if (current_temperature < target_temperature)
      output = heater_high;
    //put the heater on low if we're at our target.
    else if (current_temperature < max_temperature)
      output = heater_low;
    //turn the heater off if we're above our max.
    else
      output = 0;
  } else {
    output = 0;
  }
  return output;
}
#endif /* TEMP_PID */

