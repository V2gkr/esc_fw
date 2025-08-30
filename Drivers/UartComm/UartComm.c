#include "main.h"
#include "UartComm.h"


typedef enum{
  IDLE=0,
  BUSY=1,
}UartState;

typedef struct{
  uint8_t addr[2];
  uint8_t payload[8];
}UartFrame;

uint8_t RxBuf[sizeof(UartFrame)];
uint8_t RxBufPointer;

UartState RxState=IDLE;
extern UART_HandleTypeDef huart2;

void UartCommInit(void){
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2,RxBuf,30);
}

void UartCommCallback(uint8_t size){
  RxBufPointer=size;
  RxState=BUSY;
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2,RxBuf,30);
}

void UartCommService(void){
  UartFrame * RxFrame=(UartFrame*)RxBuf;
  if(RxState==BUSY){
    if(RxBufPointer!=sizeof(UartFrame))
      return;
    //processing address and payload
  }
}