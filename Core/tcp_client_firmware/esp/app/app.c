/* **********************************************************************
 *	ESP TCP APP Source File
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

#include "app.h"

/* ------------------------------- */
static void esp_set_parameters(void);
/* ---------------------------------------------------------------------------------------------------------------------------------------------- */
static void power_enable_fp				(void * esp_api_object);
static void power_disable_fp			(void * esp_api_object);
static void wifi_connection_success_fp	(void * called_by_object, void * esp_api_object);
static void wifi_connection_fail_fp		(void * called_by_object, void * esp_api_object);
static void tcp_connection_success_fp	(void * called_by_object, void * esp_api_object);
static void tcp_connection_fail_fp		(void * called_by_object, void * esp_api_object);
static void transmit_data_fp			(void * called_by_object, void * esp_api_object, const unsigned char * rx_data, unsigned int rx_data_length);
static void received_data_fp			(void * called_by_object, void * esp_api_object, const unsigned char * rx_data, unsigned int rx_data_length);
/* ---------------------------------------------------------------------------------------------------------------------------------------------- */

static esp_api_cb _esp_api_cb=
{
		&power_enable_fp,
		&power_disable_fp,
		&wifi_connection_success_fp,
		&wifi_connection_fail_fp,
		&tcp_connection_success_fp,
		&tcp_connection_fail_fp,
		&transmit_data_fp,
		&received_data_fp
};

/* Private variables --------- */
static esp_app_t _app;

static unsigned int counter = 0;

/* ----------------------> GLOBAL FUNCTION START <---------------------- */
/*
 * @Brief: ESP APP init
 * */
void esp_app_init(void)
{
	queue_create(&_app.queue);

	esp_api_create(&_app, &_app._api, &_esp_api_cb);

	esp_set_parameters();
}

/*
 * @Brief: ESP APP 1 ms timer
 * */
void esp_app_1_ms_timer(void)
{
	esp_api_1_ms_timer(&_app._api);

	_app.tx_timer++;
}

/*
 * @Brief: ESP APP polling
 * */
void esp_app_polling(void)
{
	esp_api_tcp_api_controller(&_app._api);

	if(_app.flag.tcp == TCP_CONNECTION_SUCCESS &&
	   _app.tx_timer > SEND_MESSAGE_TIMEOUT)
	{
		_app.tx_timer = 0;
		counter++;
		unsigned char test_text[255] = {0};
		snprintf((char *)test_text,
		   sizeof(test_text),
		   "The journey of life is filled with twists and turns, leading us down paths unknown. In the depths of uncertainty, we find strength to face the challenges ahead.Test number %d\r\n",
		   counter);

		queue_write_data(&_app._api.tx_queue, test_text, strlen((char *)test_text));
	}
}


/*
 * @Brief: ESP API queue filler
 * */
void write_into_api_rx_queue(unsigned char * rx_data, unsigned int rx_data_length)
{
	queue_write_data(&_app._api.rx_queue, (unsigned char *)rx_data, rx_data_length);
	_app._api.rx_timer = 0;
}
/* -----------------------> GLOBAL FUNCTION END <----------------------- */


static void esp_set_parameters(void)
{
	esp_set_wifi_ssid(&_app._api, WIFI_SSID);
	esp_set_wifi_password(&_app._api, WIFI_PASSWORD);
	esp_set_tcp_ip(&_app._api, TCP_IP);
	esp_set_tcp_port(&_app._api, TCP_PORT);
}
/* -------------------------------------------- */
static void power_enable_fp(void * esp_api_object)
{
	hardware_gpio_enable(GPIOB, GPIO_PIN_2);
}
/* --------------------------------------------- */
static void power_disable_fp(void * esp_api_object)
{
	hardware_gpio_disable(GPIOB, GPIO_PIN_2);
}
/* ------------------------------------------------------------------------------- */
static void wifi_connection_success_fp(void * called_by_object, void * esp_api_object)
{
	_app.flag.wifi = WIFI_CONNECTION_SUCCESS;
}
/* ----------------------------------------------------------------------------- */
static void wifi_connection_fail_fp(void * called_by_object, void * esp_api_object)
{
	_app.flag.wifi = WIFI_CONNECTION_FAIL;
}
/* ------------------------------------------------------------------------------- */
static void tcp_connection_success_fp(void * called_by_object, void * esp_api_object)
{
	_app.flag.tcp = TCP_CONNECTION_SUCCESS;
}
/* ---------------------------------------------------------------------------- */
static void tcp_connection_fail_fp(void * called_by_object, void * esp_api_object)
{
	_app.flag.tcp = TCP_CONNECTION_FAIL;
}
/* ---------------------------------------------------------------------------------------------------------------------------------- */
static void transmit_data_fp(void * called_by_object, void * esp_api_object, const unsigned char * rx_data, unsigned int rx_data_length)
{
	hardware_transmit_w_uart(rx_data, rx_data_length);
}
/* ---------------------------------------------------------------------------------------------------------------------------------- */
static void received_data_fp(void * called_by_object, void * esp_api_object, const unsigned char * rx_data, unsigned int rx_data_length)
{
	queue_write_data(&_app.queue, (unsigned char *)rx_data, rx_data_length);
}
