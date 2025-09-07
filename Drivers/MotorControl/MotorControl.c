#include "main.h"
#include "DRV8320S.h"
#include "MotorControl.h"
#include "MotorConfig.h"
volatile uint8_t bldc_count=0;

extern TIM_HandleTypeDef htim6;
extern ADC_HandleTypeDef hadc2;

MotorControlParameterStruct MotorControlParameters={0};

uint16_t AdcDmaData=0;

typedef enum{
  HALL_STATE_1=0b101,
  HALL_STATE_2=0b100,
  HALL_STATE_3=0b110,
  HALL_STATE_4=0b010,
  HALL_STATE_5=0b011,
  HALL_STATE_6=0b001
}HallStates;

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

void MotorGetActualHallState(void){
  /* pa5 - tim2 ch1 a), pb3 - tim2 ch2(b) , pb10 - tim2 ch3 (c)*/
  MotorControlParameters.HallState=(((GPIOA->IDR&GPIO_PIN_5)>>5)<<2)
                                    |(((GPIOB->IDR&GPIO_PIN_3)>>3)<<1)
                                    |(((GPIOB->IDR&GPIO_PIN_10)>>10));
}

void MotorCalculateNewHallState(void){
  switch(MotorControlParameters.HallState){
    case HALL_STATE_1:
      if(MotorControlParameters.PrevHallState==HALL_STATE_6){
        MotorControlParameters.Direction=1;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_2){
        MotorControlParameters.Direction=0;
      }
      break;
    case HALL_STATE_2:
      if(MotorControlParameters.PrevHallState==HALL_STATE_1){
        MotorControlParameters.Direction=1;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_3){
        MotorControlParameters.Direction=0;
      }
      break;
    case HALL_STATE_3:
      if(MotorControlParameters.PrevHallState==HALL_STATE_2){
        MotorControlParameters.Direction=1;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_4){
        MotorControlParameters.Direction=0;
      }
      break;
    case HALL_STATE_4:
      if(MotorControlParameters.PrevHallState==HALL_STATE_3){
        MotorControlParameters.Direction=1;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_5){
        MotorControlParameters.Direction=0;
      }
      break;
    case HALL_STATE_5:
      if(MotorControlParameters.PrevHallState==HALL_STATE_4){
        MotorControlParameters.Direction=1;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_6){
        MotorControlParameters.Direction=0;
      }
      break;
    case HALL_STATE_6:
      if(MotorControlParameters.PrevHallState==HALL_STATE_5){
        MotorControlParameters.Direction=1;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_1){
        MotorControlParameters.Direction=0;
      }
      break;
    default:
      break;
  }
  MotorControlParameters.PrevHallState=MotorControlParameters.HallState;
}