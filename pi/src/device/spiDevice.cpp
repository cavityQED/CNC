#include "spiDevice.h"

namespace CNC
{

namespace DEVICE
{

sem_t*		spiDevice::spi_ready_semaphore = sem_open("spi_ready_semaphore", O_CREAT, 0, 0);

spiDevice::spiDevice(QWidget* parent) : QWidget(parent)
{
	spi_cs_fd = open("/dev/spidev0.0", O_RDWR);

	//Check if spi opened successfully
	if(spi_cs_fd < 0)
	{
		std::cout << "Error opening SPI device\n";
		return;
	}

	//SPI Setup
	unsigned char	spi_mode		= SPI_MODE_2;
	unsigned char	spi_bitsPerWord	= 8;
	unsigned int	spi_speed		= 5000000;		//SPI clock speed in hz

	ioctl(spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
	ioctl(spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
	ioctl(spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
	ioctl(spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
	ioctl(spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
	ioctl(spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);

	sendBuffer.resize(spi_transaction_length);
	recvBuffer.resize(spi_transaction_length);

	//Clear the SPI transaction
	memset(&spi, 0, sizeof(spi));

	//Set the transaction parameters
	spi.speed_hz		= spi_speed;
	spi.bits_per_word	= spi_bitsPerWord;
	spi.delay_usecs		= 0;
	spi.cs_change		= 1;				//1 to reset chip select line after transaction, 0 otherwise

	//Attach the semaphore release function to the rising edge of the ready pin
	gpioSetMode(spi_ready_pin, PI_INPUT);
	gpioSetPullUpDown(spi_ready_pin, PI_PUD_DOWN);
	gpioSetISRFunc(spi_ready_pin, RISING_EDGE, 0, release_spi_ready_semaphore);
}

void spiDevice::spiSend(int devicePin)
{
	//Check if valid pin
	if(devicePin < 0 || devicePin > 40)
		return;

	//Clear the recieve buffer
	memset(&recvBuffer[0], 0, sizeof(recvBuffer));

	//Set the buffers and the length
	spi.tx_buf	= (unsigned long)(&sendBuffer[0]);
	spi.rx_buf	= (unsigned long)(&recvBuffer[0]);
	spi.len		= sizeof(int)*sendBuffer.size();

	//Trigger the device to recieve an SPI transaction
	gpioWrite(devicePin, 1);

	//Wait until the device is ready
	sem_wait(spi_ready_semaphore);

	//Send the message
	ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);

	//Reset the device pin
	gpioWrite(devicePin, 0);
}

}//DEVICE namespace
}//CNC namespace