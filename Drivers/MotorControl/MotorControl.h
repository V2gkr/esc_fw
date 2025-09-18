#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#include <stdint.h>

typedef struct{
  float RPM_reference;
  float RPM_measured;
  uint8_t Direction;
  uint8_t HallState;
  uint8_t PrevHallState;
  int16_t ElectricalAngle;
  uint8_t Step;
  uint8_t StepEvent;
}MotorControlParameterStruct;


void MotorSixStepSwitch(void);

void MotorTurnOn(void);

void MotorTurnOff(void);

float MotorCalculateNewRPM(uint16_t new_value);

void MotorCalculateRotationSpeed(uint32_t time);
void MotorGetActualHallState(void);
void MotorCalculateNewHallState(void);
void MotorLoadNewStep(void);
void MotorGetNextStep(void);
#endif