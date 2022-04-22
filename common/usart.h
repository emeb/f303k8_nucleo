/*
 * usart.c - usart diag support for F373 TG
 * 12-13-2018 E. Brombaugh
 */

#ifndef __usart__
#define __usart__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx.h"

void setup_usart(void);
void usart_putc(void* p, char c);
int usart_getc(void);

#ifdef __cplusplus
}
#endif

#endif
