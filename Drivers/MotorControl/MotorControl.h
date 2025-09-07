#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#include <stdint.h>

typedef struct{
  float RPM_reference;
  float RPM_measured;
}MotorControlParameterStruct;


void MotorSixStepSwitch(void);

void MotorTurnOn(void);

void MotorTurnOff(void);

float MotorCalculateNewRPM(uint16_t new_value);

void MotorCalculateRotationSpeed(uint32_t time);
#endif