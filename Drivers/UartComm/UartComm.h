#ifndef __UART_PROCESSING_H
#define __UART_PROCESSING_H

void UartCommInit(void);

void UartCommCallback(uint8_t size);

void UartCommService(void);

#endif