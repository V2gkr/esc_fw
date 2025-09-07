#include "main.h"
#include "DRV8320S.h"
#include "MotorControl.h"
#include "MotorConfig.h"
volatile uint8_t bldc_count=0;

extern TIM_HandleTypeDef htim6;
extern ADC_HandleTypeDef hadc2;

MotorControlParameterStruct MotorControlParameters={0};

uint16_t AdcDmaData=0;



void MotorControlInit(void){
  MotorControlParameters.RPM_reference=MotorCalculateNewRPM(SIX_STEP_FREQ);
}

void MotorSixStepSwitch(void){
	switch(bldc_count){
		case 0:
			GPIOA->BSRR=GPIO_BSRR_BS11|GPIO_BSRR_BR12;
			GPIOB->BSRR=GPIO_BSRR_BR1|GPIO_BSRR_BS2;
			GPIOC->BSRR=GPIO_BSRR_BR_6|GPIO_BSRR_BR_8;
			bldc_count++;
		break;
		case 1:
			GPIOA->BSRR=GPIO_BSRR_BS11|GPIO_BSRR_BR12;
			GPIOB->BSRR=GPIO_BSRR_BR1|GPIO_BSRR_BR2;
			GPIOC->BSRR=GPIO_BSRR_BR_6|GPIO_BSRR_BS_8;
			bldc_count++;
			break;
		case 2:
			GPIOA->BSRR=GPIO_BSRR_BR11|GPIO_BSRR_BR12;
			GPIOB->BSRR=GPIO_BSRR_BS1|GPIO_BSRR_BR2;
			GPIOC->BSRR=GPIO_BSRR_BR_6|GPIO_BSRR_BS_8;
			bldc_count++;
			break;
		case 3:
			GPIOA->BSRR=GPIO_BSRR_BR11|GPIO_BSRR_BS12;
			GPIOB->BSRR=GPIO_BSRR_BS1|GPIO_BSRR_BR2;
			GPIOC->BSRR=GPIO_BSRR_BR_6|GPIO_BSRR_BR_8;
			bldc_count++;
			break;
		case 4:
			GPIOA->BSRR=GPIO_BSRR_BR11|GPIO_BSRR_BS12;
			GPIOB->BSRR=GPIO_BSRR_BR1|GPIO_BSRR_BR2;
			GPIOC->BSRR=GPIO_BSRR_BS_6|GPIO_BSRR_BR_8;
			bldc_count++;
			break;
		case 5:
			GPIOA->BSRR=GPIO_BSRR_BR11|GPIO_BSRR_BR12;
			GPIOB->BSRR=GPIO_BSRR_BR1|GPIO_BSRR_BS2;
			GPIOC->BSRR=GPIO_BSRR_BS_6|GPIO_BSRR_BR_8;
			bldc_count=0;
			break;
	}
}

void MotorTurnOn(void){
  DRV8320_SetEnable();
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_ADC_Start_DMA(&hadc2,(uint32_t*)&AdcDmaData,1);
}

void MotorTurnOff(void){
  DRV8320_ResetEnable();
  HAL_TIM_Base_Stop_IT(&htim6);
  HAL_ADC_Stop_DMA(&hadc2);
}

/* new value is a value inside ARR register*/
float MotorCalculateNewRPM(uint16_t new_value){
  return (float)(new_value/(MOTOR_POLE_PAIR*6.0f))*60;
}

void MotorCalculateRotationSpeed(uint32_t time){
  /* time is ccr1 register which is in a units of prescaler*/
  float ftime=(float)time/HALL_CLOCK;
  MotorControlParameters.RPM_measured=(float)60/(MOTOR_POLE_PAIR*ftime);
}