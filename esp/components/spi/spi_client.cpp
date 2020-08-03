#include "spi_client.h"

SpiClient::SpiClient() {
	//Clear the config structures to avoid warnings about
	//not setting all parameters
	memset(&bus_config, 0, sizeof(bus_config));
	memset(&slave_config, 0, sizeof(slave_config));
	
	//Setup Bus Configuration
	bus_config.mosi_io_num	= MOSI;
	bus_config.miso_io_num	= MISO;
	bus_config.sclk_io_num	= SCLK;
	bus_config.quadwp_io_num = -1;	//Only needed for 4-bit transactions
	bus_config.quadhd_io_num = -1;	//Only needed for 4-bit transactions
	
	//Setup Slave Transaction Configuration
	slave_config.mode			= SPI_MODE;
	slave_config.spics_io_num	= CS;
	slave_config.queue_size		= QUEUE_SIZE;
	slave_config.post_setup_cb	= spi_post_setup_callback;
	slave_config.post_trans_cb	= spi_post_trans_callback;
	
	//Enable Pullups on SPI Pins
	gpio_set_pull_mode(MOSI, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(SCLK, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(CS, GPIO_PULLUP_ONLY);
	
	//Initialize SPI
	esp_err_t err = spi_slave_initialize(SPI3_HOST, &bus_config, &slave_config, DMA_CHANNEL);
	if(err != ESP_OK)
		std::cout << "Failed to initialize SPI\n";
		
	//Configure the Ready Signal
	ready_pin_config.intr_type		= GPIO_INTR_DISABLE;
	ready_pin_config.mode 			= GPIO_MODE_OUTPUT;
	ready_pin_config.pull_down_en	= GPIO_PULLDOWN_ENABLE;
	ready_pin_config.pull_up_en		= GPIO_PULLUP_DISABLE;
	ready_pin_config.pin_bit_mask	= (1 << READY);
	gpio_config(&ready_pin_config);
	
	sendbuf.resize(MAX_TRANSACTION_LENGTH);
}
	
void SpiClient::get_message(std::vector<int> &msg) {	
	//Clear the transaction
	memset(&spi_transaction, 0, sizeof(spi_transaction));
	
	//Set the buffers and transaction length
	spi_transaction.length		= 8*MAX_TRANSACTION_LENGTH; //Max length in bits
	spi_transaction.tx_buffer	= &sendbuf[0];
	spi_transaction.rx_buffer	= &msg[0];
	
	//Wait for the message
	esp_err_t err = spi_slave_transmit(SPI3_HOST, &spi_transaction, portMAX_DELAY);
	
	//If no errors, return
	if(err == ESP_OK)
		return;
	else {
		std::cout << "SPI Transaction Error:\n";
		std::cout << esp_err_to_name(err) << '\n';
	}
}

void SpiClient::set_sendbuffer(int index, int value) {
	sendbuf[index] = value;
}
