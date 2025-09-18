#include "main.h"
#include "DRV8320S.h"
#include "MotorControl.h"
#include "MotorConfig.h"
volatile uint8_t bldc_count=0;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim6;
extern ADC_HandleTypeDef hadc2;

volatile MotorControlParameterStruct MotorControlParameters={0};

/* step 1 - phase 3 pwm , phase 2 force inactive , phase 1 off*/
#define STEP1_CCMR1   0x4808
#define STEP1_CCMR2   0x6868
#define STEP14_CCER   0x1550

/* step 2 - phase 3 off , phase 2 force inactive , phase 1 pwm*/
#define STEP2_CCMR1   0x4868
#define STEP2_CCMR2   0x6808
#define STEP25_CCER   0x1055

/* step 3 - phase 3 force inactive , phase 2 off , phase 1 pwm*/
#define STEP3_CCMR1   0x0868
#define STEP3_CCMR2   0x6848
#define STEP36_CCER   0x1505

/* step 4 - phase 3 force inactive , phase 2 pwm , phase 1 off*/
#define STEP4_CCMR1   0x6808
#define STEP4_CCMR2   0x6848

/* step 5 - phase 3 off , phase 2 pwm , phase 1 force inactive*/
#define STEP5_CCMR1   0x6848
#define STEP5_CCMR2   0x6808

/* step 6 - phase 3 pwm , phase 2 off , phase 1 force inactive*/
#define STEP6_CCMR1   0x0848
#define STEP6_CCMR2   0x6868




typedef enum{
  HALL_STATE_1=0b101,
  HALL_STATE_2=0b100,
  HALL_STATE_3=0b110,
  HALL_STATE_4=0b010,
  HALL_STATE_5=0b011,
  HALL_STATE_6=0b001
}HallStates;

volatile HallStates log_step[256];
volatile uint8_t log_i=0;
uint16_t AdcDmaData=0;
volatile uint32_t log_ccr1[1024];
volatile uint16_t log_i_ccr1=0;
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

void MotorLoadNewStep(void){
  if(MotorControlParameters.StepEvent){
    MotorControlParameters.StepEvent=0;
    switch(MotorControlParameters.Step){
      case 1:
        TIM1->CCER=STEP14_CCER;
        TIM1->CCMR1=STEP1_CCMR1;
        TIM1->CCMR2=STEP1_CCMR2;
        break;
      case 2:
        TIM1->CCER=STEP25_CCER;
        TIM1->CCMR1=STEP2_CCMR1;
        TIM1->CCMR2=STEP2_CCMR2;
        break;
      case 3:
        TIM1->CCER=STEP36_CCER;
        TIM1->CCMR1=STEP3_CCMR1;
        TIM1->CCMR2=STEP3_CCMR2;
        break;
      case 4:
        TIM1->CCER=STEP14_CCER;
        TIM1->CCMR1=STEP4_CCMR1;
        TIM1->CCMR2=STEP4_CCMR2;
        break;
      case 5:
        TIM1->CCER=STEP25_CCER;
        TIM1->CCMR1=STEP5_CCMR1;
        TIM1->CCMR2=STEP5_CCMR2;
        break;
      case 6:
        TIM1->CCER=STEP36_CCER;
        TIM1->CCMR1=STEP6_CCMR1;
        TIM1->CCMR2=STEP6_CCMR2;
        break;
      default:
        break;
    }
    TIM1->EGR |= (uint16_t) TIM_EGR_COMG;
  }
}

void MotorTurnOn(void){
  DRV8320_SetEnable();
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_ADC_Start_DMA(&hadc2,(uint32_t*)&AdcDmaData,1);
  TIM1->CCR1 = 2000;
	TIM1->CCR2 = 2000;
	TIM1->CCR3 = 2000;
	TIM1->CCR4 = 1;
  
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
  TIM1->CR2 |= (uint16_t) TIM_CR2_CCPC;
  __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC4);
  __HAL_TIM_MOE_ENABLE(&htim1);
}

void MotorTurnOff(void){
  DRV8320_ResetEnable();
  HAL_TIM_Base_Stop_IT(&htim6);
  HAL_ADC_Stop_DMA(&hadc2);
  __HAL_TIM_MOE_DISABLE_UNCONDITIONALLY(&htim1);
}

/* new value is a value inside ARR register*/
float MotorCalculateNewRPM(uint16_t new_value){
  return (float)(new_value/(MOTOR_POLE_PAIR*6.0f))*60;
}

void MotorCalculateRotationSpeed(uint32_t time){
  /* time is ccr1 register which is in a units of prescaler*/
  if(log_i_ccr1!=1023)
    log_ccr1[log_i_ccr1++]=time;
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
        MotorControlParameters.ElectricalAngle=PHASE_SHIFT_60;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_2){
        MotorControlParameters.Direction=0;
        MotorControlParameters.ElectricalAngle=PHASE_SHIFT_120;
      }
      break;
    case HALL_STATE_2:
      if(MotorControlParameters.PrevHallState==HALL_STATE_1){
        MotorControlParameters.Direction=1;
        MotorControlParameters.ElectricalAngle=PHASE_SHIFT_120;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_3){
        MotorControlParameters.Direction=0;
        MotorControlParameters.ElectricalAngle=PHASE_SHIFT_60+PHASE_SHIFT_120;
      }
      break;
    case HALL_STATE_3:
      if(MotorControlParameters.PrevHallState==HALL_STATE_2){
        MotorControlParameters.Direction=1;
        MotorControlParameters.ElectricalAngle=PHASE_SHIFT_120+PHASE_SHIFT_60;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_4){
        MotorControlParameters.Direction=0;
        MotorControlParameters.ElectricalAngle=-PHASE_SHIFT_120;
      }
      break;
    case HALL_STATE_4:
      if(MotorControlParameters.PrevHallState==HALL_STATE_3){
        MotorControlParameters.Direction=1;
        MotorControlParameters.ElectricalAngle=-PHASE_SHIFT_120;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_5){
        MotorControlParameters.Direction=0;
        MotorControlParameters.ElectricalAngle=-PHASE_SHIFT_60;
      }
      break;
    case HALL_STATE_5:
      if(MotorControlParameters.PrevHallState==HALL_STATE_4){
        MotorControlParameters.Direction=1;
        MotorControlParameters.ElectricalAngle=-PHASE_SHIFT_60;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_6){
        MotorControlParameters.Direction=0;
        MotorControlParameters.ElectricalAngle=0;
      }
      break;
    case HALL_STATE_6:
      if(MotorControlParameters.PrevHallState==HALL_STATE_5){
        MotorControlParameters.Direction=1;
        MotorControlParameters.ElectricalAngle=0;
      }
      else if(MotorControlParameters.PrevHallState==HALL_STATE_1){
        MotorControlParameters.Direction=0;
        MotorControlParameters.ElectricalAngle=PHASE_SHIFT_60;
      }
      break;
    default:
      break;
  }
  if(MotorControlParameters.PrevHallState!=MotorControlParameters.HallState){
    MotorControlParameters.StepEvent=1;
  }
  MotorControlParameters.PrevHallState=MotorControlParameters.HallState;
  if(log_i!=0xFF && MotorControlParameters.HallState!=log_step[log_i])
    log_step[log_i++]=MotorControlParameters.HallState;
}

void MotorGetNextStep(void){
  if(MotorControlParameters.ElectricalAngle==0)
    MotorControlParameters.Step=1;
  else if(MotorControlParameters.ElectricalAngle==PHASE_SHIFT_60)
    MotorControlParameters.Step=2;
  else if(MotorControlParameters.ElectricalAngle==PHASE_SHIFT_120)
    MotorControlParameters.Step=3;
  else if(MotorControlParameters.ElectricalAngle==(PHASE_SHIFT_120+PHASE_SHIFT_60))
    MotorControlParameters.Step=4;
  else if(MotorControlParameters.ElectricalAngle==(-PHASE_SHIFT_120))
    MotorControlParameters.Step=5;
  else if(MotorControlParameters.ElectricalAngle==(-PHASE_SHIFT_60))
    MotorControlParameters.Step=6;

}