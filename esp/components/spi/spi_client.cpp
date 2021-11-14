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
	
	//Configure the Signal GPIO. Used to signal the pi that a move has ocurred
	ready_pin_config.pin_bit_mask	= (1ULL << SIGNAL);
	gpio_config(&ready_pin_config);
	
	sendbuf.resize(MAX_TRANSACTION_LENGTH);
}
	
void SpiClient::get_message(std::vector<int> &msg) {	
	//Clear the transaction
	memset(&spi_transaction, 0, sizeof(spi_transaction));
	
	//Set the buffers and transaction length
	spi_transaction.length		= 8*sizeof(int)*MAX_TRANSACTION_LENGTH;
	spi_transaction.tx_buffer	= &sendbuf[0];
	spi_transaction.rx_buffer	= &msg[0];
	
	std::cout << "SPI SLAVE Transmit.....\n";
	//Wait for the message
	esp_err_t err = spi_slave_transmit(SPI3_HOST, &spi_transaction, portMAX_DELAY);
	
	//If no errors, return
	if(err == ESP_OK) {
		return;
	}
	else {
		std::cout << "SPI Transaction Error:\n";
		std::cout << esp_err_to_name(err) << '\n';
	}
}

void SpiClient::get_data(std::vector<int> &data) {
	std::cout << "Getting " << 8*4*data.size() << " Bits of Data...\n";
	memset(&spi_transaction, 0, sizeof(spi_transaction));
	
	spi_transaction.length		= 8*sizeof(int)*data.size();
	spi_transaction.tx_buffer	= &sendbuf[0];
	spi_transaction.rx_buffer	= &data[0];
	
	esp_err_t err = spi_slave_transmit(SPI3_HOST, &spi_transaction, portMAX_DELAY);
	if(err == ESP_OK)
		std::cout << "Received " << spi_transaction.trans_len << " Bits of Data\n";
	if(err != ESP_OK) {
		std::cout << "SPI Transaction Error: \n";
		std::cout << esp_err_to_name(err) << '\n';
	}
}

void SpiClient::set_sendbuffer_value(int index, int value) {
	sendbuf[index] = value;
}

void SpiClient::printFunction(AXIS_FUNCTION_CODE code) {
	std::cout << "Message: ";
	switch(code) {
		case SET_FEED_RATE:			std::cout << "Set Feed Rate";				break;		
		case SET_DIRECTION:			std::cout << "Set Direction";				break;
		case SET_STEP_TIME:			std::cout << "Set Step Time";				break;
		case SET_STEPS_TO_MOVE:		std::cout << "Set Steps to Move";			break;
		case SET_JOG_STEPS:			std::cout << "Set Jog Steps";				break;
		case SET_BACKLASH:			std::cout << "Set Backlash";				break;
		case SET_X_AXIS:			std::cout << "Set X Axis";					break;
		case SET_STEPS_PER_MM:		std::cout << "Set steps/mm";				break;
		case SET_MAX_STEPS:			std::cout << "Set Max Steps";				break;
		case SETUP_CURVE:			std::cout << "Setup Curve";					break;
		case ENA_JOG_MODE:			std::cout << "Enable Jog Mode";				break;
		case ENA_LINE_MODE:			std::cout << "Enable Line Mode";			break;
		case ENA_CURV_MODE:			std::cout << "Enable Curve Mode";			break;
		case ENA_SYNC_MODE:			std::cout << "Enable Sync Mode";			break;
		case ENA_JOG_CONTINUOUS:	std::cout << "Continuous Jog Enabled";		break;
		case ENA_TRAVEL_LIMITS:		std::cout << "Enable Travel Limits";		break;
		case FIND_ZERO:				std::cout << "Find Zero";					break;
		case MOVE:					std::cout << "Move";						break;
		case STOP:					std::cout << "Stop";						break;
		case RECEIVE:				std::cout << "Receive";						break;
		case TEST_FUNCTION:			std::cout << "Test Function";				break;
		default:					std::cout << "Unrecognized Function";		break;
	}
	std::cout << '\n';
}
	 
