/* **********************************************************************
 *	Main Source File
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

void main_systick_timer(void)
{
	esp_app_1_ms_timer();
}

int main(void)
{
	hardware_init();

	HAL_Delay(1000);

	esp_app_init();

  while (1)
  {
	  esp_app_polling();
  }
}

void main_uart_irq(unsigned char * rx_data, unsigned int rx_data_length)
{
	write_into_api_rx_queue(rx_data, rx_data_length);
}
