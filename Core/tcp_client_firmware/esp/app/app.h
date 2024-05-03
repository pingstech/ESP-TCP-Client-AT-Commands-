/* **********************************************************************
 *	ESP TCP APP Header File
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

#ifndef TCP_CLIENT_FIRMWARE_ESP_APP_APP_H_
#define TCP_CLIENT_FIRMWARE_ESP_APP_APP_H_

#include "hardware.h"
#include "api.h"
#include <string.h>

#define SEND_MESSAGE_TIMEOUT ((int)1000) // in ms


#define WIFI_SSID 		"SAYKAL2_2G"			// @Note: WiFi SSID info
#define WIFI_PASSWORD	"SAYKAL2_5023058"		// @Note: WiFi Pasword info
#define TCP_IP			"192.168.1.45"	// @Note: TCP IP info
#define TCP_PORT		((int)28)			// @Note: TCP Port info
#define STATIC_IP		"192.168.1.1"	// @Note: Static IP info
#define GATEWAY_IP		"192.168.1.3"	// @Note: Gateway IP info
#define MAC_IP			"255.255.255.0"	// @Note: Mac IP info

typedef enum
{
	WIFI_CONNECTION_FAIL,
	WIFI_CONNECTION_SUCCESS,
	TCP_CONNECTION_FAIL,
	TCP_CONNECTION_SUCCESS
}esp_app_flag_type_e;

typedef struct
{
	esp_app_flag_type_e	wifi;
	esp_app_flag_type_e	tcp;
}esp_app_flag_t;

typedef struct
{
	esp_api_t		_api;
	queue_t			queue;
	esp_app_flag_t	flag;
	unsigned int	tx_timer;
}esp_app_t;

void esp_app_init(void);
void esp_app_1_ms_timer(void);
void esp_app_polling(void);
void write_into_api_rx_queue(unsigned char * rx_data,
							 unsigned int rx_data_length);

#endif /* TCP_CLIENT_FIRMWARE_ESP_APP_APP_H_ */
