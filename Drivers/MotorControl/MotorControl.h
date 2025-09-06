#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#include <stdint.h>

typedef struct{
  float RPM;
}MotorControlParameterStruct;


void MotorSixStepSwitch(void);

void MotorTurnOn(void);

void MotorTurnOff(void);

float MotorCalculateNewRPM(uint16_t new_value);
#endif