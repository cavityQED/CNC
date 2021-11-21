#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

#include <pigpio.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <semaphore.h>

#include <cmath>
#include <vector>
#include <iostream>

#include <QWidget>

namespace CNC
{

namespace DEVICE
{

class spiDevice : public QWidget
{
	Q_OBJECT

public:

	static void release_spi_ready_semaphore(int gpio, int level, uint32_t tick)
	{
		sem_post(spi_ready_semaphore);
	}

	spiDevice(QWidget* parent = nullptr);
	~spiDevice() {sem_unlink("spi_ready_semaphore");}

	void spiSend(int devicePin);
	void spiWaitForReady()		{sem_wait(spi_ready_semaphore);}

protected:

	//Spi Stuff
	int							spi_cs_fd;
	int							spi_transaction_length = 12;	//Transaction length in bytes
	int							spi_ready_pin = 24;
	struct	spi_ioc_transfer 	spi;
	std::vector<int>			sendBuffer;
	std::vector<int>			recvBuffer;
	static sem_t*				spi_ready_semaphore;
};

}//DEVICE namespace
}//CNC namespace

#endif