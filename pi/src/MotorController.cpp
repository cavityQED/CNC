#include "MotorController.h"

sem_t* MotorController::sem = sem_open("sem_ready",O_CREAT, 0, 0);

MotorController::MotorController() {
	setup_spi();
	setup_gpio();
}

void MotorController::setup_spi() {
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
	memset(&sendbuf, 0, sizeof(sendbuf));
	memset(&recvbuf, 0, sizeof(recvbuf));
	
	spi.tx_buf = (unsigned long)sendbuf;
	spi.rx_buf = (unsigned long)recvbuf;
	spi.len = sizeof(sendbuf);
	spi.delay_usecs = 0;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = spi_bitsPerWord;
	spi.cs_change = 0;
}

void MotorController::setup_gpio() {
	wiringPiSetup();
	
	pinMode(READY_PIN, INPUT);
	pullUpDnControl(READY_PIN, PUD_DOWN);
	
	//Attach the ready signal to the semaphore release interrupt
	wiringPiISR(READY_PIN, INT_EDGE_RISING, release_sem_interrupt);
}

void MotorController::setup_x_axis(motorParameters &params) {
	x_params.SPR 			= params.SPR;
	x_params.mm_per_step	= params.mm_per_step;
	x_params.pin_num		= params.pin_num;
	
	pinMode(x_params.pin_num, OUTPUT);
	digitalWrite(x_params.pin_num, 0);
	
	x_connected = true;
}

void MotorController::setup_y_axis(motorParameters &params) {
	y_params.SPR 			= params.SPR;
	y_params.mm_per_step	= params.mm_per_step;
	y_params.pin_num		= params.pin_num;
	
	pinMode(y_params.pin_num, OUTPUT);
	digitalWrite(y_params.pin_num, 0);
	
	y_connected = true;
}

void MotorController::send(int device_pin) {
	std::cout << "Sending Function: " << (SPI_FUNCTION_CODE) sendbuf[0] << '\n';
	
	//Trigger the interrupt on the correct device to signal a message is ready to be sent
	digitalWrite(device_pin, 1);
	
	//Wait for the device to be ready for the message
	sem_wait(sem);
	
	//Send the message
	ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);
	
	//Reset the trigger line for the device
	digitalWrite(device_pin, 0);
}

void MotorController::enable_jog_mode() {
	if(x_connected) {
		sendbuf[0] = ENA_JOG_MODE;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[0] = ENA_JOG_MODE;
		send(y_params.pin_num);
	}
}

void MotorController::disable_jog_mode() {
	if(x_connected) {
		sendbuf[0] = DIS_JOG_MODE;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[0] = DIS_JOG_MODE;
		send(y_params.pin_num);
	}
}

void MotorController::enable_step_mode() {
	if(x_connected) {
		sendbuf[0] = ENA_STEP_MODE;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[0] = ENA_STEP_MODE;
		send(y_params.pin_num);
	}
}

void MotorController::set_jog_speed_steps(int steps) {
	if(x_connected) {
		sendbuf[0] = SET_JOG_SPEED_STEPS;
		sendbuf[1] = steps;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[0] = SET_JOG_SPEED_STEPS;
		sendbuf[1] = steps;
		send(y_params.pin_num);
	}
}

void MotorController::set_jog_speed_mm(double mm) {
	if(x_connected) {
		sendbuf[0] = SET_JOG_SPEED_MM;
		sendbuf[1] = 100 * mm;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[0] = SET_JOG_SPEED_MM;
		sendbuf[1] = 100 * mm;
		send(y_params.pin_num);
	}
}

void MotorController::get_position(double &x_steps, double &y_steps) {
	if(x_connected) {
		sendbuf[0] = GET_POSITION;
		send(x_params.pin_num);
		sendbuf[0] = RECEIVE;
		send(x_params.pin_num);
		
		x_steps = x_params.mm_per_step * (recvbuf[1] + 255*(recvbuf[2] + 255*recvbuf[3]));
	}
	
	if(y_connected) {
		sendbuf[0] = GET_POSITION;
		send(y_params.pin_num);
		sendbuf[0] = RECEIVE;
		send(y_params.pin_num);
		
		y_steps = y_params.mm_per_step * (recvbuf[1] + 255*(recvbuf[2] + 255*recvbuf[3]));
	}
}

double MotorController::x_get_position() {
	if(!x_connected)
		return -1;
		
	sendbuf[0] = GET_POSITION;
	send(x_params.pin_num);
	sendbuf[0] = RECEIVE;
	send(x_params.pin_num);
	
	return x_params.mm_per_step * (recvbuf[1] + 255*(recvbuf[2] + 255*recvbuf[3]));
}

double MotorController::y_get_position() {
	if(!y_connected)
		return -1;
		
	sendbuf[0] = GET_POSITION;
	send(y_params.pin_num);
	sendbuf[0] = RECEIVE;
	send(y_params.pin_num);
	
	return y_params.mm_per_step * (recvbuf[1] + 255*(recvbuf[2] + 255*recvbuf[3]));
}

void MotorController::x_move() {
	if(!x_connected)
		return;
		
	sendbuf[0] = MOVE;
	send(x_params.pin_num);
}

void MotorController::y_move() {
	if(!y_connected)
		return;
		
	sendbuf[0] = MOVE;
	send(y_params.pin_num);
}

void MotorController::x_zero() {
	if(!x_connected)
		return;
		
	sendbuf[0] = ZERO;
	send(x_params.pin_num);
}

void MotorController::y_zero() {
	if(!y_connected)
		return;
		
	sendbuf[0] = ZERO;
	send(y_params.pin_num);
}

void MotorController::x_set_dir(bool dir) {
	if(!x_connected)
		return;
		
	sendbuf[0] = SET_DIRECTION;
	sendbuf[1] = (int) dir;
	send(x_params.pin_num);
}

void MotorController::y_set_dir(bool dir) {
	if(!y_connected)
		return;
		
	sendbuf[0] = SET_DIRECTION;
	sendbuf[1] = (int) dir;
	send(y_params.pin_num);
}


