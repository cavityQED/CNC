#ifndef SPI_CLIENT_H
#define SPI_CLIENT_H

#include <cstring>
#include <iostream>
#include <vector>
#include <cmath>

#include "driver/gpio.h"
#include "driver/spi_slave.h"

#define MOSI	(gpio_num_t) 23
#define MISO	(gpio_num_t) 19
#define SCLK	(gpio_num_t) 18
#define CS		(gpio_num_t) 5
#define READY	(gpio_num_t) 22	//Signal host we're ready to receive message

#define SPI_MODE				2
#define DMA_CHANNEL				1
#define QUEUE_SIZE				1
#define MAX_TRANSACTION_LENGTH	12

class SpiClient {
public:
	SpiClient();
	~SpiClient() {}
	
	void get_message(std::vector<int> &msg);	
	void get_data(std::vector<int> &data);
	
	void set_sendbuffer_value(int index, int value);
	
	/*	SPI Post Setup Callback
	* 		Executes after spi transaction is set up to signal
	* 		to the host device that a message is ready to be received
	* 		by setting the READY pin high
	*/
	static void spi_post_setup_callback(spi_slave_transaction_t *t) {
		WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1 << READY));
	}	

	/*	SPI Post Transaction Callback
	* 		Executes after spi transaction is completed
	* 		Sets READY pin low again
	*/
	static void spi_post_trans_callback(spi_slave_transaction_t *t) {
		WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1 << READY));	
	}
	
	void toggle_ready() {
		gpio_set_level(READY, 1);
		gpio_set_level(READY, 0);
	}
		
private:
	spi_slave_interface_config_t	slave_config;
	spi_bus_config_t				bus_config;
	gpio_config_t					ready_pin_config;
	
	spi_slave_transaction_t			spi_transaction;
	
	std::vector<int> sendbuf;
};

#endif
