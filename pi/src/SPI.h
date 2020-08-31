#ifndef SPI_H
#define SPI_H

#include <string>
#include <cstring>
#include <mutex>
#include <iostream>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

#include "Types.h"

#define READY_PIN	24

namespace SPI {
	static int spi_cs_fd;
	static struct spi_ioc_transfer spi;
	
	
	void unlock_mutex_isr();
	
	void setup();
	
	const std::string function_code_to_string(FUNCTION_CODE code);
		
}//SPI namespace

#endif
