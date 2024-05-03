/* **********************************************************************
 *	ESP TCP API Header File
 *
 *
 *	Project Name : ESP TCP client connection driver
 *
 *
 *	Date         : 28.04.2024
 *	Author       : Furkan YAYLA
 *	e-mail       : yaylafurkan@protonmail.com
 *
 * **********************************************************************
*/

#ifndef TCP_CLIENT_FIRMWARE_ESP_API_API_H_
#define TCP_CLIENT_FIRMWARE_ESP_API_API_H_

#include "queue.h"
#include <stdio.h>

#define ESP_RESET_TIMEOUT		((int)2000) // in ms
#define ESP_STATE_DELAY_TIMEOUT	((int)100)	// in ms
#define ESP_UART_SYNC_TIMEOUT	((int)20)	// in ms

#define ESP_XL_BUFF_SIZE ((int)200)
#define ESP_L_BUFF_SIZE ((int)150)
#define ESP_S_BUFF_SIZE ((int)50)

typedef enum
{
  ESP_FAIL,			// Fail
  ESP_SUCCESS,      // Success
  ESP_HARD_FAIL     // Critical failure
}esp_return_type_e;

typedef struct
{
	void (* power_enable_cb)			(void * esp_api_object);
	void (* power_disable_cb)			(void * esp_api_object);
	void (* wifi_connection_success_cb)	(void * called_by_object, void * esp_api_object);
	void (* wifi_connection_fail_cb)	(void * called_by_object, void * esp_api_object);
	void (* tcp_connection_success_cb)	(void * called_by_object, void * esp_api_object);
	void (* tcp_connection_fail_cb)		(void * called_by_object, void * esp_api_object);
	void (* transmit_data_cb)			(void * called_by_object, void * esp_api_object, const unsigned char * rx_data, unsigned int rx_data_length);
	void (* received_data_cb)			(void * called_by_object, void * esp_api_object, const unsigned char * rx_data, unsigned int rx_data_length);
}esp_api_cb;

typedef enum
{
	AT_RESTORE,
	AT_CWMODE,
	AT_CIFSR,
	AT_CIPSTA,
	AT_CWJAP,
	AT_CIPSTART,
	AT_CIPSEND,
	TCP_COMMUNICATION,
	TRANSMIT_MESSAGE,
	AT_DELAY_STATE,
	ESP_HARD_RESET
}esp_at_state_e;

typedef enum
{
	COMMAND_TRANSMITTING,
	COMMAND_RESPONSE_RECEIVING,
	COMMAND_ATTEMPT_CONTROL
}esp_command_state_e;

typedef struct
{
	unsigned char wifi_ssid[ESP_S_BUFF_SIZE];
	unsigned char wifi_password[ESP_S_BUFF_SIZE];
	unsigned char tcp_ip[ESP_S_BUFF_SIZE];
	unsigned int  tcp_port;
	unsigned char gateway_ip[ESP_S_BUFF_SIZE];
	unsigned char static_ip[ESP_S_BUFF_SIZE];
	unsigned char mac_ip[ESP_S_BUFF_SIZE];
}esp_tcp_parameter_t;

typedef struct
{
	esp_command_state_e	  state;
	unsigned char		* tx_data;
	unsigned char		* expected_response;
	unsigned int	  	  number_of_attempt;
	unsigned int	  	  timer;
}esp_command_t;

typedef struct
{
	void 				* called_by_object;
	esp_api_cb			* callback;
	queue_t		  	  	  tx_queue;
	unsigned char 		  tx_buffer[QUEUE_SIZE];
	queue_t		  	  	  rx_queue;
	unsigned int		  rx_timer;
	esp_at_state_e	  	  state;
	esp_at_state_e		  next_state;
	unsigned int		  delay_timer;
	unsigned int		  reset_timer;
	esp_command_t	  	  command;
	esp_tcp_parameter_t	  parameter;
}esp_api_t;

esp_return_type_e esp_api_create(void 		* called_by_object,
								 esp_api_t 	* self,
								 esp_api_cb * callback);
void esp_api_1_ms_timer(esp_api_t * self);
void esp_api_tcp_api_controller(esp_api_t * self);


/* -----------------------> PARAMETER FUNCTIONS <----------------------- */
esp_return_type_e esp_set_wifi_ssid(esp_api_t * self, char * wifi_ssid);
esp_return_type_e esp_set_wifi_password(esp_api_t * self, char * wifi_password);
esp_return_type_e esp_set_tcp_ip(esp_api_t * self, char * tcp_ip);
esp_return_type_e esp_set_tcp_port(esp_api_t * self, int  tcp_port);
esp_return_type_e esp_set_static_ip(esp_api_t * self, char * static_ip);
esp_return_type_e esp_set_gateway_ip(esp_api_t * self, char * gateway_ip);
esp_return_type_e esp_set_mac_ip(esp_api_t * self, char * mac_ip);
/* -----------------------> PARAMETER FUNCTIONS <----------------------- */

#endif /* TCP_CLIENT_FIRMWARE_ESP_API_API_H_ */
