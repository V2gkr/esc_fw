#include "main.h"
#include "DRV8320S.h"
volatile uint8_t bldc_count=0;

extern TIM_HandleTypeDef htim6;

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
}

void MotorTurnOff(void){
  DRV8320_ResetEnable();
  HAL_TIM_Base_Stop_IT(&htim6);
}