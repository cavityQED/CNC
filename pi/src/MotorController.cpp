#include "MotorController.h"

sem_t* MotorController::sem = sem_open("sem_ready",O_CREAT, 0, 0);

MotorController::MotorController(QWidget *parent) : QWidget(parent){
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
	
	sendbuf.resize(8);
	recvbuf.resize(8);
	
	memset(&spi, 0, sizeof(spi));
	
	spi.delay_usecs = 0;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = spi_bitsPerWord;
	spi.cs_change = 1;
	
	circle_params.r = 50;
	circle_params.s = 16;
	circle_params.theta_i = 0;
	circle_params.theta_f = 360;
	circle_params.x_spmm = 200;
	circle_params.y_spmm = 200;
	
	QAction *calc = new QAction;
	calc->setShortcut(Qt::Key_F9);
	connect(calc, SIGNAL(triggered()), this, SLOT(calc_circle()));
	addAction(calc);
	
	QAction *send = new QAction;
	send->setShortcut(Qt::Key_F10);
	connect(send, SIGNAL(triggered()), this, SLOT(send_circle()));
	addAction(send);
}

void MotorController::setup_gpio() {
	wiringPiSetupGpio();
	
	pinMode(READY_PIN, INPUT);
	pullUpDnControl(READY_PIN, PUD_DOWN);
	
	//Attach the ready signal to the semaphore release interrupt
	wiringPiISR(READY_PIN, INT_EDGE_RISING, release_sem_interrupt);
	
	pinMode(SYNC_PIN, OUTPUT);
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
	memset(&recvbuf[0], 0, sizeof(recvbuf));
	
	spi.tx_buf = (unsigned long)(&sendbuf[0]);
	spi.rx_buf = (unsigned long)(&recvbuf[0]);
	spi.len = 4*sendbuf.size();
	
	std::cout << "Sending Function: " << (SPI_FUNCTION_CODE) sendbuf[0] << '\n';
	
	//Trigger the interrupt on the correct device to signal a message is ready to be sent
	digitalWrite(device_pin, 1);
	
	//Wait for the device to be ready for the message
	sem_wait(sem);
	
	//Send the message
	ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);
	std::cout << "Function Sent\n";
	
	//Reset the trigger line for the device
	digitalWrite(device_pin, 0);
}

void MotorController::send_data(int device_pin, std::vector<int> &data, SPI_FUNCTION_CODE type) {
	sendbuf[0] = type;
	send(device_pin);
	
	spi.tx_buf = (unsigned long)(&data[0]);
	spi.rx_buf = NULL;
	spi.len = 4*data.size();
	std::cout << "Sending " << 4*data.size() << " Bytes of Data...\n";
	
	sem_wait(sem);
	
	ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);
	std::cout << "Data Sent\n";
}

void MotorController::send_circle_ops(int device_pin, int ops) {
	sendbuf[0] = SET_CIRCLE_OPS;
	sendbuf[1] = ops;
	send(device_pin);
}

void MotorController::circle_move() {
	digitalWrite(SYNC_PIN, 0);
	
	bool x_ready = false;
	bool y_ready = false;
	
	if(x_connected) {
		sendbuf[0] = SETUP_CIRCLE_MOVE;
		send(x_params.pin_num);
		sem_wait(sem);
		sendbuf[0] = RECEIVE;
		send(x_params.pin_num);
		if(recvbuf[1] == MOTOR_READY) {
			std::cout << "X Axis Ready\n";
			x_ready = true;
		}
	}
		
	if(y_connected) {
		sendbuf[0] = SETUP_CIRCLE_MOVE;
		send(y_params.pin_num);
		sem_wait(sem);
		sendbuf[0] = RECEIVE;
		send(x_params.pin_num);
		if(recvbuf[1] = MOTOR_READY) {
			std::cout << "Y Axis Ready\n";
			y_ready = true;
		}
	}
	
	if(x_ready && y_ready) {
		sendbuf[0] = CIRCLE_MOVE;
		send(x_params.pin_num);
		sem_wait(sem);
		send(y_params.pin_num);
		sem_wait(sem);	
		digitalWrite(SYNC_PIN, 1);
	}
}

void MotorController::enable_jog_mode(bool enable) {
	SPI_FUNCTION_CODE en;
	(enable)? en = ENA_JOG_MODE : en = DIS_JOG_MODE;
	
	if(x_connected) {
		sendbuf[0] = en;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[0] = en;
		send(y_params.pin_num);
	}
}

void MotorController::enable_step_mode(bool enable) {
	SPI_FUNCTION_CODE en;
	(enable)? en = ENA_STEP_MODE : en = DIS_STEP_MODE;
	
	if(x_connected) {
		sendbuf[0] = en;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[0] = en;
		send(y_params.pin_num);
	}
}

void MotorController::enable_sync_mode(bool enable) {
	SPI_FUNCTION_CODE en;
	(enable)? en = ENA_SYNC_MODE : en = DIS_SYNC_MODE;
	
	if(x_connected) {
		sendbuf[0] = en;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[0] = en;
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
		
		x_steps = x_params.mm_per_step * recvbuf[1];
	}
	
	if(y_connected) {
		sendbuf[0] = GET_POSITION;
		send(y_params.pin_num);
		sendbuf[0] = RECEIVE;
		send(y_params.pin_num);
		
		y_steps = y_params.mm_per_step * recvbuf[1];
	}
}

double MotorController::x_get_position() {
	if(!x_connected)
		return -1;
		
	sendbuf[0] = GET_POSITION;
	send(x_params.pin_num);
	sendbuf[0] = RECEIVE;
	send(x_params.pin_num);
	
	return x_params.mm_per_step * recvbuf[1];
}

double MotorController::y_get_position() {
	if(!y_connected)
		return -1;
		
	sendbuf[0] = GET_POSITION;
	send(y_params.pin_num);
	sendbuf[0] = RECEIVE;
	send(y_params.pin_num);
	
	return y_params.mm_per_step * recvbuf[1];
}

void MotorController::sync_move() {
	digitalWrite(SYNC_PIN, 0);
	sendbuf[0] = MOVE;	
	if(x_connected) {
		send(x_params.pin_num);
		sem_wait(sem);
	}
	
	if(y_connected) {
		send(y_params.pin_num);
		sem_wait(sem);
	}
	
	digitalWrite(SYNC_PIN, 1);	
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

void MotorController::x_set_steps_to_move(int steps) {
	if(!x_connected)
		return;
		
	sendbuf[0] = SET_STEPS_TO_MOVE;
	sendbuf[1] = steps;
	send(x_params.pin_num);
}

void MotorController::y_set_steps_to_move(int steps) {
	if(!y_connected)
		return;
		
	sendbuf[0] = SET_STEPS_TO_MOVE;
	sendbuf[1] = steps;
	send(y_params.pin_num);
}

void MotorController::copy_circle_ops(curve::motor_ops_t &ops) {
	circle_ops.num_ops = ops.num_ops;
	
	for(int i = 0; i < circle_ops.num_ops; i++) {
		circle_ops.x_steps.push_back(ops.x_steps[i]);
		circle_ops.x_times.push_back(ops.x_times[i]);
		circle_ops.x_dirs.push_back(ops.x_dirs[i]);
		
		circle_ops.y_steps.push_back(ops.y_steps[i]);
		circle_ops.y_times.push_back(ops.y_times[i]);
		circle_ops.y_dirs.push_back(ops.y_dirs[i]);
	}
}
