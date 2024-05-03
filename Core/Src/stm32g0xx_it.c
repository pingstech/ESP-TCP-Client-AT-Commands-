/* **********************************************************************
 *	IRQ Source File
 *
 *
 *	Project Name : ESP TCP client connection driver
 *
 *
 *	Date         : 13.01.2024
 *	Author       : Furkan YAYLA
 *	e-mail       : yaylafurkan@protonmail.com
 *
 * **********************************************************************
*/


#include "main.h"
#include "stm32g0xx_it.h"

extern UART_HandleTypeDef huart2;
extern unsigned char rx_uart;

void NMI_Handler(void)
{

  while (1)
  {
  }
}


void HardFault_Handler(void)
{
  while (1)
  {

  }
}


void SVC_Handler(void)
{

}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{

}


void SysTick_Handler(void)
{

  HAL_IncTick();

  main_systick_timer();

}


void USART2_LPUART2_IRQHandler(void)
{

  HAL_UART_IRQHandler(&huart2);

  HAL_UART_Receive_IT(&huart2, &rx_uart, 1);

  //write_into_api_rx_queue(&rx_uart,1);

  main_uart_irq(&rx_uart, 1);
}

