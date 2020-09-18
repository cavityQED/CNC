#include "SPI.h"

namespace SPI {
	
const std::string function_code_to_string(FUNCTION_CODE code) {
	switch(code) {
		case SET_FEED_RATE:			return "Set Feed Rate";				
		case SET_DIRECTION:			return "Set Direction";			
		case SET_STEP_TIME:			return "Set Step Time";			
		case SET_STEPS_TO_MOVE:		return "Set Steps to Move";		
		case SET_JOG_STEPS:			return "Set Jog Steps";	
		case SET_BACKLASH:			return "Set Backlash";			
		case SET_X_AXIS:			return "Set X Axis";
		case SET_STEPS_PER_MM:		return "Set steps/mm";
		case SET_MAX_STEPS:			return "Set Max Steps";	
		case SETUP_CURVE:			return "Setup Curve";
		case ENA_JOG_MODE:			return "Enable Jog Mode";		
		case ENA_LINE_MODE:			return "Enable Line Mode";
		case ENA_CURV_MODE:			return "Enable Curve Mode";
		case ENA_SYNC_MODE:			return "Enable Sync Mode";
		case ENA_TRAVEL_LIMITS:		return "Enable Travel Limits";
		case FIND_ZERO:				return "Find Zero";
		case MOVE:					return "Move";
		case STOP:					return "Stop";
		case RECEIVE:				return "Receive";
		default:					return "Unrecognized Function";
	}
	
}//function_code_to_string

void setup() {
	unsigned char spi_mode = SPI_MODE_2;
	unsigned char spi_bitsPerWord = 8;
	unsigned int spi_speed = 5000000;
	
	spi_cs_fd = open("/dev/spidev0.0", O_RDWR);
	
	if(spi_cs_fd < 0) {
		std::cout << "Could not open spi device\n";
	}
	
	ioctl(spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
	ioctl(spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
	ioctl(spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
	ioctl(spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
	ioctl(spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
	ioctl(spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
	
	memset(&spi, 0, sizeof(spi));
	
	spi.delay_usecs = 0;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = spi_bitsPerWord;
	spi.cs_change = 1;
}//setup

}//SPI namespace
