#include "MotorController.h"

sem_t* MotorController::sem = sem_open("sem_ready",O_CREAT, 0, 0);

MotorController::MotorController(QWidget *parent) : QWidget(parent){
	setup_spi();
	setup_gpio();
	
	QAction *x_z = new QAction;
	x_z->setShortcut(Qt::Key_F9);
	connect(x_z, SIGNAL(triggered()), this, SLOT(zero_x()));
	addAction(x_z);
	
	QAction *y_z = new QAction;
	y_z->setShortcut(Qt::Key_F10);
	connect(y_z, SIGNAL(triggered()), this, SLOT(zero_y()));
	addAction(y_z);
	
	QAction *circle = new QAction;
	circle->setShortcut(Qt::Key_F5);
	connect(circle, SIGNAL(triggered()), this , SLOT(run_p()));
	addAction(circle);
	
	QAction *print = new QAction;
	print->setShortcut(Qt::Key_F11);
	connect(print, SIGNAL(triggered()), this, SLOT(print_pos()));
	addAction(print);
}

MotorController::~MotorController() {
	sem_unlink("sem_ready"); 
	std::cout << "MotorController destroyed\n";
}

void MotorController::test_lines() {
	program_moves.clear();
	
	gcode::get_program("cnc.nc", program_moves);
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
	
	sendbuf.resize(12);
	recvbuf.resize(12);
	
	memset(&spi, 0, sizeof(spi));
	
	spi.delay_usecs = 0;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = spi_bitsPerWord;
	spi.cs_change = 1;
}

void MotorController::setup_gpio() {
	gpioTerminate();
	int err = gpioInitialise();
	if(err < 0)
		std::cout << "initialisation failed\n";
	gpioSetMode(READY_PIN, PI_INPUT);
	gpioSetPullUpDown(READY_PIN, PI_PUD_DOWN);
	gpioSetISRFunc(READY_PIN, RISING_EDGE, 0, release_sem_interrupt);
	
	gpioSetMode(SYNC_PIN, PI_OUTPUT);
}

void MotorController::setup_axis(motor::params_t &params) {	
	motor::params_t *p;
	int x_axis;
	
	if(params.a == SPI::X_AXIS) {
		p = &x_params;
		x_axis = 1;
		x_connected = true;
	}
	else if(params.a == SPI::Y_AXIS) {
		p = &y_params;
		x_axis = 0;
		y_connected = true;
	}
	else
		return;
		
	p->pin_num		= params.pin_num;
	p->spmm			= params.spmm;
	p->max_steps	= params.max_steps;
	p->backlash		= params.backlash;
	p->a			= params.a;
	
	if(!p->pin_num) return;
	
	//pigpio
	gpioSetMode(params.pin_num, PI_OUTPUT);
	gpioWrite(params.pin_num, 0);
		
	sendbuf[0] = SPI::SET_X_AXIS;
	sendbuf[1] = x_axis;
	send(params.pin_num);
	
	set_steps_per_mm(params.a, params.spmm);
	set_max_steps(params.a, params.max_steps);
	set_backlash(params.a, params.backlash);
}

void MotorController::timerEvent(QTimerEvent *e) {
	get_position_spi();
	emit positionChanged(x_pos, y_pos);
	if(!in_motion) {
		killTimer(e->timerId());
		if(in_program) {
			cur_program_move += 1;
			std::cout << "Next Move....\n";
			next_move();
		}
	}
}
void MotorController::send(int device_pin) {
	if(!device_pin)
		return;
		
	memset(&recvbuf[0], 0, sizeof(recvbuf));
	
	spi.tx_buf = (unsigned long)(&sendbuf[0]);
	spi.rx_buf = (unsigned long)(&recvbuf[0]);
	spi.len = sizeof(int)*sendbuf.size();
	
	std::cout << "Sending Function: " << SPI::function_code_to_string((SPI::FUNCTION_CODE)sendbuf[0]) << '\n';
	
	//Trigger the interrupt on the correct device to signal a message is ready to be sent
	if(gpioWrite(device_pin, 1))
		std::cout << "Error Writing pin " << device_pin << '\n';
	else
		std::cout << "Wrote pin " << device_pin << " high\n";
	
	//Wait for the device to be ready for the message
	sem_wait(sem);
	
	//Send the message
	ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);
	std::cout << "Function Sent\n";
	
	//Reset the trigger line for the device
	gpioWrite(device_pin, 0);
}

void MotorController::send_curve_data(curve::esp_params_t &data) {	
	sendbuf[0] = SPI::SETUP_CURVE;
	sendbuf[1] = data.dir;
	sendbuf[2] = data.r;
	sendbuf[3] = data.x_i;
	sendbuf[4] = data.y_i;
	sendbuf[5] = data.x_f;
	sendbuf[6] = data.y_f;
	sendbuf[7] = data.feed_rate;
	
	if(x_connected) {
		send(x_params.pin_num);
		sem_wait(sem);
	}
		
	if(y_connected) {
		send(y_params.pin_num);
		sem_wait(sem);
	}
}

void MotorController::send_line_data(line::ops_t &data) {
	std::cout << "Sending line data\n";
	
	sendbuf[0] = SPI::SET_STEPS_TO_MOVE;
	sendbuf[1] = data.x_steps;
	send(x_params.pin_num);
	sendbuf[1] = data.y_steps;
	send(y_params.pin_num);
		
	sendbuf[0] = SPI::SET_STEP_TIME;
	sendbuf[1] = data.x_time;
	std::cout << "X Step Time: " << data.x_time << '\n';
	send(x_params.pin_num);
	sendbuf[1] = data.y_time;
	std::cout << "Y Step Time: " << data.y_time << '\n';
	send(y_params.pin_num);
		
	sendbuf[0] = SPI::SET_DIRECTION;
	sendbuf[1] = data.x_dir;
	send(x_params.pin_num);
	sendbuf[1] = data.y_dir;
	send(y_params.pin_num);		
}

void MotorController::start_program(std::vector<motor::move_t> &moves) {
	program_moves = moves;
	start_program();
}

void MotorController::start_program() {
	if(in_motion || program_moves.empty())
		return;
		
	cur_program_move = 0;
	enable_sync_mode(true);

	if(program_moves[0].laser_op)
	{
		emit setLaserPower(program_moves[0].l_op.power);
		cur_program_move++;
	}
	
	if(program_moves[cur_program_move].line_op) {
		enable_line_mode(true);
		send_line_data(program_moves[cur_program_move].l);
		sync_move();
	}
	
	else if(!program_moves[cur_program_move].line_op) {
		enable_curv_mode(true);
		send_curve_data(program_moves[cur_program_move].c);
		sync_move();
	}
}

void MotorController::next_move() {
	if(cur_program_move == program_moves.size()) {
		in_program = false;
		cur_program_move = 0;
		std::cout << "End of Moves\n";
		enable_jog_mode(true);
		return;
	}
	else {
		std::cout << "Move " << cur_program_move << ":\n";
		if(program_moves[cur_program_move].laser_op)
		{
			emit setLaserPower(program_moves[cur_program_move].l_op.power);
			startTimer(TIMER_PERIOD);
			in_program = true;
			in_motion = true;
		}

		else if(program_moves[cur_program_move].line_op) {
			enable_line_mode(true);
			send_line_data(program_moves[cur_program_move].l);
			sync_move();
		}
		
		else if(!program_moves[cur_program_move].line_op) {
			enable_curv_mode(true);
			send_curve_data(program_moves[cur_program_move].c);
			sync_move();
		}
	}
}

void MotorController::sync_move() {
	gpioWrite(SYNC_PIN, 0);
	
	sendbuf[0] = SPI::MOVE;
	
	if(x_connected) {
		send(x_params.pin_num);
		sem_wait(sem);
	}
	if(y_connected) {
		send(y_params.pin_num);
		sem_wait(sem);
	}
	
	gpioWrite(SYNC_PIN, 1);
	std::cout << "Wrote sync pin high\n";
	startTimer(TIMER_PERIOD);
	parent()->startTimer(2*TIMER_PERIOD);
	in_program = true;
	in_motion = true;
}	

void MotorController::enable_jog_mode(bool enable) {
	sendbuf[0] = SPI::ENA_JOG_MODE;
	sendbuf[1] = (int)enable;
	
	if(x_connected)
		send(x_params.pin_num);
	
	if(y_connected)
		send(y_params.pin_num);
}

void MotorController::enable_line_mode(bool enable) {
	sendbuf[0] = SPI::ENA_LINE_MODE;
	sendbuf[1] = (int)enable;
	
	if(x_connected)
		send(x_params.pin_num);
	
	if(y_connected)
		send(y_params.pin_num);
}

void MotorController::enable_curv_mode(bool enable) {
	sendbuf[0] = SPI::ENA_CURV_MODE;
	sendbuf[1] = (int)enable;
	
	if(x_connected)
		send(x_params.pin_num);
	
	if(y_connected)
		send(y_params.pin_num);
}

void MotorController::enable_sync_mode(bool enable) {
	sendbuf[0] = SPI::ENA_SYNC_MODE;
	sendbuf[1] = (int)enable;
	
	if(x_connected)
		send(x_params.pin_num);
	
	if(y_connected)
		send(y_params.pin_num);
}

void MotorController::enable_travel_limits(bool enable) {
	sendbuf[0] = SPI::ENA_TRAVEL_LIMITS;
	sendbuf[1] = (int)enable;
	
	if(x_connected)
		send(x_params.pin_num);
	if(y_connected)
		send(y_params.pin_num);
}

void MotorController::set_jog_steps(int steps) {
	sendbuf[0] = SPI::SET_JOG_STEPS;
	
	if(x_connected) {
		sendbuf[1] = steps;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[1] = steps;
		send(y_params.pin_num);
	}
}

void MotorController::set_jog_speed_mm(double mm) {
	sendbuf[0] = SPI::SET_JOG_STEPS;

	if(x_connected) {
		sendbuf[1] = mm*x_params.spmm;
		send(x_params.pin_num);
	}
	
	if(y_connected) {
		sendbuf[1] = mm*y_params.spmm;
		send(y_params.pin_num);
	}
}

void MotorController::set_step_time(int period) {
	sendbuf[0] = SPI::SET_STEP_TIME;
	sendbuf[1] = period;
	
	if(x_connected)
		send(x_params.pin_num);
	if(y_connected)
		send(y_params.pin_num);
}

void MotorController::zero_x() {
	x_offset = 0;
	get_position_spi();
	x_offset = x_pos;
	std::cout << "X Offset: " << x_offset << '\n';
	x_pos = 0;
	parent()->startTimer(20);
}

void MotorController::zero_y() {
	y_offset = 0;
	get_position_spi();
	y_offset = y_pos;
	std::cout << "Y Offset: " << y_offset << '\n';
	y_pos = 0;
	parent()->startTimer(20);
}

void MotorController::get_position_spi() {
	if(x_connected) {
		sendbuf[0] = SPI::RECEIVE;
		send(x_params.pin_num);
		
		x_pos = (recvbuf[1]/(double)x_params.spmm) - x_offset;
		x_motion = (bool)recvbuf[2];
	}
	
	if(y_connected) {
		sendbuf[0] = SPI::RECEIVE;
		send(y_params.pin_num);
		
		y_pos = (recvbuf[1]/(double)y_params.spmm) - y_offset;
		y_motion = (bool)recvbuf[2];
	}
	
	in_motion = x_motion || y_motion;
}

int MotorController::get_pin(SPI::AXIS a) {
		if(a == SPI::X_AXIS)
			return x_params.pin_num;
		else if(a == SPI::Y_AXIS)
			return y_params.pin_num;
		else
			return -1;
}

void MotorController::move(SPI::AXIS a) {
	if(in_motion)
		return;
		
	sendbuf[0] = SPI::MOVE;
	send(get_pin(a));
	in_motion = true;
	startTimer(TIMER_PERIOD);
}

void MotorController::home(SPI::AXIS a) {
	if(in_motion)
		return;
		
	sendbuf[0] = SPI::FIND_ZERO;
	send(get_pin(a));
	in_motion = true;
	x_offset = 0;
	startTimer(TIMER_PERIOD);
}

void MotorController::set_dir(SPI::AXIS a, bool dir) {
	if(in_motion)
		return;
		
	sendbuf[0] = SPI::SET_DIRECTION;
	sendbuf[1] = (int) dir;
	send(get_pin(a));
}

void MotorController::set_feed_rate(SPI::AXIS a, int feed_rate) {
	sendbuf[0] = SPI::SET_FEED_RATE;
	sendbuf[1] = feed_rate;
	send(get_pin(a));
}

void MotorController::set_feed_rate(int feed_rate) {
	sendbuf[0] = SPI::SET_FEED_RATE;
	sendbuf[1] = feed_rate;
	if(x_connected)
		send(x_params.pin_num);
	if(y_connected)
		send(y_params.pin_num);
}

void MotorController::set_steps_to_move(SPI::AXIS a, int steps) {		
	sendbuf[0] = SPI::SET_STEPS_TO_MOVE;
	sendbuf[1] = steps;
	send(get_pin(a));
}

void MotorController::set_steps_per_mm(SPI::AXIS a, int spmm) {
	sendbuf[0] = SPI::SET_STEPS_PER_MM;
	sendbuf[1] = spmm;
	send(get_pin(a));
}

void MotorController::set_max_steps(SPI::AXIS a, int max_steps) {
	sendbuf[0] = SPI::SET_MAX_STEPS;
	sendbuf[1] = max_steps;
	send(get_pin(a));
}

void MotorController::set_backlash(SPI::AXIS a, int backlash) {
	sendbuf[0] = SPI::SET_BACKLASH;
	sendbuf[1] = backlash;
	send(get_pin(a));
}

void MotorController::updateAxisConfig() {
	ConfigureUtility config;
	config.get_axis_params(SPI::X_AXIS, x_params);
	if(x_params.pin_num)
		setup_axis(x_params);
	config.get_axis_params(SPI::Y_AXIS, y_params);
	if(y_params.pin_num)
		setup_axis(y_params);
}

void MotorController::jog_event_handler(JOG::event_t &event) {
	switch(event.type) {
		case JOG::ENABLE_JOG:
			enable_jog_mode(event.enable);
			break;
		case JOG::SET_JOG_SPEED_MM:
			set_jog_speed_mm(event.jog_mm);
			break;
		case JOG::JOG_MOVE:
			set_dir(event.axis, event.direction);
			move(event.axis);
			break;
		default:
			break;
	}
}
