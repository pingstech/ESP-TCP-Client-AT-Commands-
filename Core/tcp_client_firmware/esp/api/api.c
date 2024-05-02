/* **********************************************************************
 *	ESP TCP API Source File
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


#include "api.h"

static esp_return_type_e esp_api_command_manager(esp_api_t * self,
											     char * command,
											     char * expected_response,
											     unsigned int attempt_number,
											     unsigned int timeout);
/* --------------------------------------------------------------------- */
static void esp_state_delay(esp_api_t * self, esp_at_state_e next_state);
/* --------------------------------------------------------------------- */
static void esp_receive_handler(esp_api_t * self);
/* --------------------------------------------------------------------- */
static void esp_transmit_handler(esp_api_t * self);
/* --------------------------------------------------------------------- */
static void esp_hard_reset(esp_api_t * self);
/* --------------------------------------------------------------------- */
static unsigned int length_calculator(const char * string);
/* --------------------------------------------------------------------- */
static int string_to_int_converter(const char *string);
/* --------------------------------------------------------------------- */



/* ----------------------> GLOBAL FUNCTION START <---------------------- */

/*
 * @Brief: ESP API create function
 * @Param: Where to call the ESP API object
 * @Param: ESP API object
 * @Param: Callbacks
 * */
esp_return_type_e esp_api_create(void 		* called_by_object,
								 esp_api_t 	* self,
								 esp_api_cb * callback)
{
	if(!called_by_object || !self || !callback) {return ESP_FAIL;}

	self->called_by_object 	= called_by_object;
	self->callback 			= (esp_api_cb *)callback;
	self->callback->power_enable_cb(self);

	self->state = AT_RESTORE;
	self->command.state = COMMAND_TRANSMITTING;

	queue_create(&self->rx_queue);

	queue_create(&self->tx_queue);

	return ESP_SUCCESS;
}


/*
 * @Brief: ESP API create function
 * @Param: ESP API object
 * */
void esp_api_1_ms_timer(esp_api_t * self)
{
	if(!self) {return;}

	self->command.timer++;

	if(self->state == AT_DELAY_STATE)
	{
		self->delay_timer++;
	}

	if(self->state == ESP_HARD_RESET)
	{
		self->reset_timer++;
	}
}

/*
 * @Brief: TCP Client connection process controller
 * @Param: ESP API object
 * */
void esp_api_tcp_api_controller(esp_api_t * self)
{
	if(!self) {return;}

	switch(self->state)
	{
		case AT_RESTORE:{
			esp_return_type_e f_return = esp_api_command_manager(self,
															     (char *)"AT+RESTORE\r\n",
															     (char *)"OK",
															     3, 3000);

			if(f_return == ESP_SUCCESS)
			{
				esp_state_delay(self, AT_CWMODE);
			}

			else if(f_return == ESP_HARD_FAIL)
			{
				self->state = ESP_HARD_RESET;
			}
		break;}

		case AT_CWMODE:{
			esp_return_type_e f_return = esp_api_command_manager(self,
															     (char *)"AT+CWMODE=3\r\n",
															     (char *)"OK",
															     3, 3000);

			if(f_return == ESP_SUCCESS)
			{
				esp_state_delay(self, AT_CIFSR);
			}

			else if(f_return == ESP_HARD_FAIL)
			{
				self->state = ESP_HARD_RESET;
			}
		break;}

		case AT_CIFSR:{
			esp_return_type_e f_return = esp_api_command_manager(self,
															     (char *)"AT+CIFSR\r\n",
															     (char *)"OK",
															     3, 3000);

			if(f_return == ESP_SUCCESS)
			{
				esp_state_delay(self, AT_CIPSTA);
			}

			else if(f_return == ESP_HARD_FAIL)
			{
				self->state = ESP_HARD_RESET;
			}
		break;}

		case AT_CIPSTA:{
			if(self->parameter.gateway_ip[0] == '\0' ||
			   self->parameter.static_ip[0]  == '\0' ||
			   self->parameter.mac_ip[0]     == '\0') {self->state = AT_CWJAP;}

			unsigned char cipsta_buffer[ESP_L_BUFF_SIZE] = {0};

	        snprintf((char *)cipsta_buffer,
	                 sizeof(cipsta_buffer),
	                 "AT+CIPSTA=\"%s\",\"%s\",\"%s\"\r\n",
	                 self->parameter.static_ip,
	                 self->parameter.mac_ip,
	                 self->parameter.gateway_ip);

			esp_return_type_e f_return = esp_api_command_manager(self,
															     (char *)cipsta_buffer,
															     (char *)"OK",
															     3, 3000);

			if(f_return == ESP_SUCCESS)
			{
				esp_state_delay(self, AT_CWJAP);
			}

			else if(f_return == ESP_HARD_FAIL)
			{
				self->state = ESP_HARD_RESET;
			}
		break;}

		case AT_CWJAP:{
		   if(self->parameter.wifi_ssid[0] == '\0' || self->parameter.wifi_password[0] == '\0') {return;}

		   unsigned char cwjap_buffer[ESP_L_BUFF_SIZE] = {0};

		   snprintf((char *)cwjap_buffer,
					sizeof(cwjap_buffer),
					"AT+CWJAP=\"%s\",\"%s\"\r\n",
					self->parameter.wifi_ssid,
					self->parameter.wifi_password);

			esp_return_type_e f_return = esp_api_command_manager(self,
															     (char *)cwjap_buffer,
															     (char *)"OK",
															     3, 5000);

			if(f_return == ESP_SUCCESS)
			{
				self->callback->wifi_connection_success_cb(self->called_by_object, self);


				esp_state_delay(self, AT_CIPSTART);
			}

			else if(f_return == ESP_HARD_FAIL)
			{
				self->state = ESP_HARD_RESET;
			}
		break;}

		case AT_CIPSTART:{
			if(self->parameter.tcp_ip == NULL || self->parameter.tcp_port == 0) {return;}

	        unsigned char cipstart_buffer[ESP_L_BUFF_SIZE] = {0};

	        snprintf((char *)cipstart_buffer,
	                 sizeof(cipstart_buffer),
	                 "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",
	                 self->parameter.tcp_ip,
	                 self->parameter.tcp_port);

			esp_return_type_e f_return = esp_api_command_manager(self,
															     (char *)cipstart_buffer,
															     (char *)"CONNECT",
															     3, 5000);

			if(f_return == ESP_SUCCESS)
			{
				self->callback->tcp_connection_success_cb(self->called_by_object, self);

				esp_state_delay(self, TCP_COMMUNICATION);
			}

			else if(f_return == ESP_HARD_FAIL)
			{
				self->state = ESP_HARD_RESET;
			}
		break;}

		case AT_CIPSEND:{
	        unsigned char cipsend_buffer[ESP_L_BUFF_SIZE] = {0};

	        snprintf((char *)cipsend_buffer,
	                 sizeof(cipsend_buffer),
	                 "AT+CIPSEND=%d\r\n",
	                 length_calculator((const char *)self->tx_buffer));

			esp_return_type_e f_return = esp_api_command_manager(self,
															     (char *)cipsend_buffer,
															     (char *)">",
															     3, 3000);

			if(f_return == ESP_SUCCESS)
			{
				esp_state_delay(self, TRANSMIT_MESSAGE);
			}

			else if(f_return == ESP_HARD_FAIL)
			{
				self->state = ESP_HARD_RESET;
			}
		break;}

		case TCP_COMMUNICATION:{
			if(!queue_is_empty(&self->tx_queue) && self->rx_timer > ESP_UART_SYNC_TIMEOUT)
			{
				esp_receive_handler(self);
			}

			if(!queue_is_empty(&self->rx_queue))
			{
				esp_transmit_handler(self);
			}

		break;}

		case TRANSMIT_MESSAGE:{
			esp_return_type_e f_return = esp_api_command_manager(self,
															     (char *)self->tx_buffer,
															     (char *)"SEND OK",
															     3, 3000);

			if(f_return == ESP_SUCCESS)
			{
				esp_state_delay(self, TCP_COMMUNICATION);
			}

			else if(f_return == ESP_HARD_FAIL)
			{
				self->state = ESP_HARD_RESET;
			}
		break;}

		case AT_DELAY_STATE:
			if(self->state >= ESP_STATE_DELAY_TIMEOUT)
			{
				self->state = self->next_state;
			}
		break;

		case ESP_HARD_RESET:
			esp_hard_reset(self);
		break;
	}
}

/* ----------------------> PARAMETER FUNCTION START <---------------------- */

/*
 * @Brief: WiFi SSID must be necessary for TCP client connection
 * */
esp_return_type_e esp_set_wifi_ssid(esp_api_t * self, char * wifi_ssid)
{
	if(!self || !wifi_ssid) {return ESP_FAIL;}

	snprintf((char *)self->parameter.wifi_ssid, sizeof(self->parameter.wifi_ssid), wifi_ssid);

	return ESP_SUCCESS;
}

/*
 * @Brief: WiFi password must be necessary for TCP client connection
 * */
esp_return_type_e esp_set_wifi_password(esp_api_t * self, char * wifi_password)
{
	if(!self || !wifi_password) {return ESP_FAIL;}

	snprintf((char *)self->parameter.wifi_password, sizeof(self->parameter.wifi_password), wifi_password);

	return ESP_SUCCESS;
}

/*
 * @Brief: TCP IP must be necessary for TCP client connection
 * */
esp_return_type_e esp_set_tcp_ip(esp_api_t * self, char * tcp_ip)
{
	if(!self || !tcp_ip) {return ESP_FAIL;}

	snprintf((char *)self->parameter.tcp_ip, sizeof(self->parameter.tcp_ip), tcp_ip);

	return ESP_SUCCESS;
}

/*
 * @Brief: TCP port must be necessary for TCP client connection
 * */
esp_return_type_e esp_set_tcp_port(esp_api_t * self, int  tcp_port)
{
	if(!self || !tcp_port) {return ESP_FAIL;}

	self->parameter.tcp_port = tcp_port;

	return ESP_SUCCESS;
}

/*
 * @Brief:
 * */
esp_return_type_e esp_set_static_ip(esp_api_t * self, char * static_ip)
{
	if(!self || !static_ip) {return ESP_FAIL;}

	snprintf((char *)self->parameter.static_ip, sizeof(self->parameter.static_ip), static_ip);

	return ESP_SUCCESS;
}

/*
 * @Brief:
 * */
esp_return_type_e esp_set_gateway_ip(esp_api_t * self, char * gateway_ip)
{
	if(!self || !gateway_ip) {return ESP_FAIL;}

	snprintf((char *)self->parameter.gateway_ip, sizeof(self->parameter.gateway_ip), gateway_ip);

	return ESP_SUCCESS;
}

/*
 * @Brief:
 * */
esp_return_type_e esp_set_mac_ip(esp_api_t * self, char * mac_ip)
{
	if(!self || !mac_ip) {return ESP_FAIL;}

	snprintf((char *)self->parameter.mac_ip, sizeof(self->parameter.mac_ip), mac_ip);

	return ESP_SUCCESS;
}

/* -----------------------> PARAMETER FUNCTION END <----------------------- */
/* -----------------------> GLOBAL FUNCTION END <----------------------- */

/* ----------------------> PRIVATE FUNCTION START <---------------------- */

/*
 * @Brief: Function for sending ESP commands.
 * @Param: Pointer to ESP API object.
 * @Param: Command to be sent.
 * @Param: Expected response from ESP.
 * @Param: Timeout duration.
 * @Param: Number of attempts.
 * @Return: ESP_SUCCESS if successful, ESP_FAILED otherwise.
 * */
static esp_return_type_e esp_api_command_manager(esp_api_t * self,
											     char * command,
											     char * expected_response,
											     unsigned int attempt_number,
											     unsigned int timeout)
{
	if(!self                 ||
	   !command              ||
	   !expected_response    ||
	   attempt_number <= 0   ||
	   timeout <= 0) {return ESP_HARD_FAIL;}

	switch(self->command.state)
	{
		case COMMAND_TRANSMITTING:
			self->command.timer = 0;
			self->command.tx_data = (unsigned char *)command;
			self->command.number_of_attempt++;
			self->command.expected_response = (unsigned char *)expected_response;
			self->command.state = COMMAND_RESPONSE_RECEIVING;

			self->callback->transmit_data_cb(self->called_by_object,
											 self,
											 (unsigned char *)command,
											 length_calculator((char *)command));
		break;

		case COMMAND_RESPONSE_RECEIVING:
			if(!queue_is_empty(&self->rx_queue) &&
			   self->command.timer <= timeout 	&&
			   self->rx_timer > ESP_UART_SYNC_TIMEOUT)
			{
				unsigned char buffer[QUEUE_SIZE] = {0};

				queue_read_data(&self->rx_queue, buffer, queue_get_unreaden_size(&self->rx_queue));

				if(strstr((char *)buffer, expected_response))
				{
					self->command.number_of_attempt = 0;
					self->command.state =  COMMAND_TRANSMITTING;
					return ESP_SUCCESS;
				}
			}

			else if(self->command.timer > timeout)
			{
				self->command.state =  COMMAND_ATTEMPT_CONTROL;
			}
		break;

		case COMMAND_ATTEMPT_CONTROL:
			if(self->command.number_of_attempt >= attempt_number)
			{
				return ESP_HARD_FAIL;
			}

			self->command.state =  COMMAND_TRANSMITTING;
		break;
	}

	return ESP_FAIL;
}


/*
 * @Brief: ESP received data handler
 * */
static void esp_receive_handler(esp_api_t * self)
{
	if(!self) {return;}

	unsigned char buffer[255] = {0};

	queue_read_data(&self->rx_queue, (unsigned char *)buffer, queue_get_unreaden_size(&self->rx_queue));

	if(strstr((char *)buffer, "+IPD,"))
	{
		char * temp_hold = strstr((char *)buffer,"+IPD,");

		unsigned int received_data_size = string_to_int_converter((const char *)temp_hold);

		if(received_data_size)
		{
			char * data_hold = strstr((const char *)temp_hold, ":");

			if(data_hold)
			{
				self->callback->received_data_cb(self->called_by_object,
												 self,
												 (unsigned char*)data_hold + 1,
												 received_data_size);
			}
		}
	}

	else if(strstr((char *)buffer, "CLOSED"))
	{
		self->callback->tcp_connection_fail_cb(self->called_by_object, self);
	}

	else if(strstr((char *)buffer, "WIFI DISCONNECT"))
	{
		self->callback->wifi_connection_fail_cb(self->called_by_object, self);
	}
}

/*
 * @Brief: MCU to ESP transmit handler
 * */
static void esp_transmit_handler(esp_api_t * self)
{
	if(!self) {return;}

	unsigned int unreaden_data_size = queue_get_unreaden_size(&self->tx_queue);

	if(unreaden_data_size > 1)
	{
		memset((char *)self->tx_buffer, 0, sizeof(self->tx_buffer));

		queue_read_data(&self->tx_queue, self->tx_buffer, unreaden_data_size);

		self->state = AT_CIPSEND;
	}
}

/*
 * @Brief: Creating delay for the next step
 * @Param: ESP API object
 * @Param: Next AT state
 * */
static void esp_state_delay(esp_api_t * self, esp_at_state_e next_state)
{
  if(!self) {return;}

  self->delay_timer = 0;
  self->state = AT_DELAY_STATE;
  self->next_state = next_state;
}

static void esp_hard_reset(esp_api_t * self)
{
	if(!self) {return;}

	self->callback->power_disable_cb(self);

	if(self->reset_timer > ESP_RESET_TIMEOUT)
	{
		self->reset_timer = 0;
		self->callback->power_enable_cb(self);
		self->state = AT_RESTORE;
		self->command.state = COMMAND_TRANSMITTING;
		queue_create(&self->rx_queue);
		queue_create(&self->tx_queue);
	}
}

/*
 * @Brief: Calculating the length of string
 * */
static unsigned int length_calculator(const char * string)
{
  int index = 0;

  while (*string)
  {
      index++;
      string++;
  }

  return index;
}

/*
 * @Brief: String to integer converter
 * */
static int string_to_int_converter(const char * string)
{
    int result = 0; int flag = 0;
    while (*string) {
        if (*string >= '0' && *string <= '9') {
            result = result * 10 + (*string - '0');
            flag = 1;
        }
        else{
            if(flag == 1){break;}
        }
        string++;
    }
    return result;
}

/* -----------------------> PRIVATE FUNCTION END <----------------------- */
