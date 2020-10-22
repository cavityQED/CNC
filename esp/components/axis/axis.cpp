#include "axis.h"

/*	Initialize static variables	*/
bool			axis::motor_direction	= false;
bool			axis::motor_in_motion	= false;
int				axis::position_steps	= 0;
int				axis::step_num			= 0;
xQueueHandle	axis::syncSem			= xSemaphoreCreateBinary();

std::vector<bool> axis::step_vec = {0};
std::vector<bool> axis::dirs_vec = {0};

axis::axis() {
	//setup_gpio();
	setup_timers();
	set_defaults();
}
		
void axis::check_error(const char* msg) {
	if(error != ESP_OK) {
		std::cout << msg << '\n' << esp_err_to_name(error);
	}
}

void axis::setup_gpio() {	
	//Setup the pins to be used as GPIO outputs
	gpio_pad_select_gpio(STEP);
	error = gpio_set_direction(STEP, GPIO_MODE_OUTPUT);
	check_error("step pin set direction");
	
	gpio_pad_select_gpio(DIR);
	error = gpio_set_direction(DIR, GPIO_MODE_OUTPUT);
	check_error("direction pin set direction");
	
	gpio_pad_select_gpio(EN);
	error = gpio_set_direction(EN, GPIO_MODE_OUTPUT);
	check_error("enable pin set direction");
	
	//Enable the motor to get holding torque on startup
	gpio_set_level(EN, 0);
	
	//Setup the Sync Pin
	gpio_config_t io_conf;
	memset(&io_conf, 0, sizeof(io_conf));
	io_conf.intr_type		= GPIO_INTR_POSEDGE;
	io_conf.mode 			= GPIO_MODE_INPUT;
	io_conf.pull_down_en	= GPIO_PULLDOWN_ENABLE;
	io_conf.pin_bit_mask	= (1 << SYNC);
	
	gpio_config(&io_conf);	
	gpio_set_intr_type(SYNC, GPIO_INTR_POSEDGE);
	gpio_isr_handler_add(SYNC, syncSem_release_isr, NULL);		
}

void axis::setup_timers() {
	timer_config_t timer_config;
	memset(&timer_config, 0, sizeof(timer_config));
	timer_config.divider		= 80 * PERIOD_uS;
	timer_config.counter_dir	= TIMER_COUNT_UP;
	timer_config.counter_en		= TIMER_PAUSE;
	timer_config.alarm_en		= TIMER_ALARM_EN;
	timer_config.auto_reload	= TIMER_AUTORELOAD_EN;
	timer_config.intr_type		= TIMER_INTR_MAX;
	
	//Setup Step timers with auto reload
	timer_init(LINE_GROUP, STEP_TIMER, &timer_config);
	timer_init(CURV_GROUP, STEP_TIMER, &timer_config);
	
	//Setup Stop timers without auto reload
	timer_config.auto_reload = TIMER_AUTORELOAD_DIS;
	timer_init(LINE_GROUP, STOP_TIMER, &timer_config);
	timer_init(CURV_GROUP, STOP_TIMER, &timer_config);
						
	//Enable Interrupts on the Timers
	timer_enable_intr(LINE_GROUP, STEP_TIMER);
	timer_enable_intr(LINE_GROUP, STOP_TIMER);
	timer_enable_intr(CURV_GROUP, STEP_TIMER);
	timer_enable_intr(CURV_GROUP, STOP_TIMER);

	//Add the Callbacks
	timer_isr_register(LINE_GROUP, STEP_TIMER, line_step_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_isr_register(LINE_GROUP, STOP_TIMER, line_stop_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);	
	timer_isr_register(CURV_GROUP, STEP_TIMER, curv_step_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_isr_register(CURV_GROUP, STOP_TIMER, curv_stop_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
}

void axis::line_step_isr(void* arg) {
	timer_spinlock_take(LINE_GROUP);

	gpio_set_level(STEP, 1);
	gpio_set_level(STEP, 0);

	(motor_direction)? position_steps += 1 : position_steps -= 1;

	timer_group_clr_intr_status_in_isr(LINE_GROUP, STEP_TIMER);
	timer_group_enable_alarm_in_isr(LINE_GROUP, STEP_TIMER);
	timer_spinlock_give(LINE_GROUP);
}

void axis::line_stop_isr(void* arg) {
	timer_spinlock_take(LINE_GROUP);
	
	timer_group_set_counter_enable_in_isr(LINE_GROUP, STEP_TIMER, TIMER_PAUSE);
	timer_group_set_counter_enable_in_isr(LINE_GROUP, STOP_TIMER, TIMER_PAUSE);
	
	motor_in_motion = false;
	
	timer_group_clr_intr_status_in_isr(LINE_GROUP, STOP_TIMER);
	timer_group_enable_alarm_in_isr(LINE_GROUP, STOP_TIMER);
	timer_spinlock_give(LINE_GROUP);
}

void axis::curv_step_isr(void* arg) {
	timer_spinlock_take(CURV_GROUP);
		
	gpio_set_level(STEP, step_vec[step_num]);
	gpio_set_level(STEP, 0);
		
	(motor_direction)? position_steps += (int)step_vec[step_num] : position_steps -= (int)step_vec[step_num];
	
	motor_direction = dirs_vec[++step_num];
	gpio_set_level(DIR, motor_direction);
	
	timer_group_clr_intr_status_in_isr(CURV_GROUP, STEP_TIMER);	
	timer_group_enable_alarm_in_isr(CURV_GROUP, STEP_TIMER);
	timer_spinlock_give(CURV_GROUP);
}

void axis::curv_stop_isr(void* arg) {
	timer_spinlock_take(CURV_GROUP);
	
	timer_group_set_counter_enable_in_isr(CURV_GROUP, STEP_TIMER, TIMER_PAUSE);
	timer_group_set_counter_enable_in_isr(CURV_GROUP, STOP_TIMER, TIMER_PAUSE);
	
	motor_in_motion = false;
	
	timer_group_clr_intr_status_in_isr(CURV_GROUP, STOP_TIMER);
	timer_group_enable_alarm_in_isr(CURV_GROUP, STOP_TIMER);
	timer_spinlock_give(CURV_GROUP);
}

void axis::syncSem_release_isr(void* arg) {
	xSemaphoreGiveFromISR(syncSem, NULL);
}

void axis::set_defaults() {
	steps_per_mm		= 200;
	max_travel_steps 	= 400 * steps_per_mm;
	backlash			= 0;
	jog_steps	 		= 200;
	set_direction(1);
}

void axis::set_feed_rate(int speed) {
	if(speed <= 0)
		return;
	
	feed_rate = speed;
	pulse_period_us = 1000000/(feed_rate*steps_per_mm);
	
	std::cout << feed_rate << "mm/s = " << pulse_period_us << "us/step\n";
}

void axis::set_step_time(int time) {
	if(time < 250)
		return;
		
	pulse_period_us = time;
	set_feed_rate(1000000/(pulse_period_us*steps_per_mm));
}

void axis::set_direction(bool dir) {
	if(motor_in_motion)
		return;
		
	motor_direction = dir;
	gpio_set_level(DIR, dir);
}

void axis::set_steps_to_move(int steps) {		
	std::cout << "Will move " << steps << " steps on next move\n";
	steps_to_move = steps;
}

void axis::set_jog_steps(int steps) {
	jog_steps = steps;
	std::cout << "Jog Steps: " << jog_steps << '\n';
}

void axis::enable_jog_mode(bool enable) {
	jog_mode = enable;
	if(jog_mode) {
		std::cout << "Jog Mode Enabled\n";
		enable_line_mode(false);
		enable_curv_mode(false);
		enable_sync_mode(false);
	}
	else
		std::cout << "Jog Mode Disabled\n";
}

void axis::enable_line_mode(bool enable) {
	line_mode = enable;
	if(line_mode) {
		std::cout << "Line Mode Enabled\n";
		enable_jog_mode(false);
		enable_curv_mode(false);
	}
	else
		std::cout << "Step Mode Disabled\n";
}

void axis::enable_curv_mode(bool enable) {
	curv_mode = enable;
	if(curv_mode) {
		std::cout << "Curve Mode Enabled\n";
		enable_jog_mode(false);
		enable_line_mode(false);
	}
	else
		std::cout << "Curve Mode Disabled\n";
}

void axis::enable_sync_mode(bool enable) {
	sync_mode = enable;
	if(sync_mode) {
		std::cout << "Sync Mode Enabled\n";
	}
	else
		std::cout << "Sync Mode Disabled\n";
}

void axis::enable_continuous_jog(bool enable) {
	if(!jog_mode && enable)
		enable_jog_mode(true);
	jog_continuous = enable;
}

void axis::enable_travel_limits(bool enable) {
	travel_limits = enable;
}

void axis::move() {
	if(motor_in_motion) {
		std::cout << "Motor in motion already!\n";
		if(sync_mode) {
			spi->toggle_ready();
			xSemaphoreTake(syncSem, portMAX_DELAY);
		}
		return;
	}
		
	if(jog_mode) 
		move_jog_mode();
	else if(line_mode) 
		move_line_mode();
	else if(curv_mode)
		move_curv_mode();
	else
		return;
}

void axis::move_jog_mode() {
	if(!jog_mode || motor_in_motion || jog_steps == 0)
		return;
	
	if(travel_limits) {	
		if(motor_direction && (position_steps + jog_steps) > max_travel_steps)
			return;
		if(!motor_direction && jog_steps > position_steps)
			return;
	}
	
	int wait_time;
	if(jog_continuous) {
		jog_steps = motor_direction? max_travel_steps - position_steps : position_steps;
		wait_time = pulse_period_us;
	}
	else
		wait_time = jog_wait_time;
	
	std::cout << "Jogging " << jog_steps << " steps\n";
		
	reset_timer_counters();
	
	timer_set_alarm_value(LINE_GROUP, STEP_TIMER, wait_time / PERIOD_uS);
	timer_set_alarm_value(LINE_GROUP, STOP_TIMER, wait_time * jog_steps / PERIOD_uS);
	
	timer_start(LINE_GROUP, STEP_TIMER);
	timer_start(LINE_GROUP, STOP_TIMER);
	motor_in_motion = true;
}

void axis::move_line_mode() {
	if(motor_in_motion || (steps_to_move == 0 && !sync_mode))
		return;
	
	if(sync_mode && steps_to_move == 0) {
		spi->toggle_ready();
		xSemaphoreTake(syncSem, portMAX_DELAY);
		return;
	}
	
	if(travel_limits) {	
		if(motor_direction && (position_steps + steps_to_move) > max_travel_steps)
			return;
		if(!motor_direction && steps_to_move > position_steps)
			return;
	}
	
	reset_timer_counters();
				
	timer_set_alarm_value(LINE_GROUP, STEP_TIMER, pulse_period_us / PERIOD_uS);
	timer_set_alarm_value(LINE_GROUP, STOP_TIMER, pulse_period_us * steps_to_move / PERIOD_uS);
	
	if(sync_mode) {
		spi->toggle_ready();
		xSemaphoreTake(syncSem, portMAX_DELAY);
	}
		
	timer_start(LINE_GROUP, STEP_TIMER);
	timer_start(LINE_GROUP, STOP_TIMER);
	motor_in_motion = true;
}

void axis::move_curv_mode() {
	if(motor_in_motion)
		return;
		
	reset_timer_counters();
	
	step_num = 0;
	
	timer_set_alarm_value(CURV_GROUP, STEP_TIMER, pulse_period_us / PERIOD_uS);
	timer_set_alarm_value(CURV_GROUP, STOP_TIMER, pulse_period_us * step_vec.size() / PERIOD_uS);
	
	set_direction(dirs_vec[0]);
	
	if(sync_mode) {
		spi->toggle_ready();
		xSemaphoreTake(syncSem, portMAX_DELAY);
	}
	
	timer_start(CURV_GROUP, STEP_TIMER);
	timer_start(CURV_GROUP, STOP_TIMER);
	motor_in_motion = true;
}
		

void axis::stop() {
	timer_pause(LINE_GROUP, STEP_TIMER);
	timer_pause(CURV_GROUP, STEP_TIMER);
	timer_pause(LINE_GROUP, STOP_TIMER);
	timer_pause(CURV_GROUP, STOP_TIMER);
	
	motor_in_motion = false;
}

void axis::stop_zero_interlock() {
	stop();
	
	position_steps = -zero_steps;
	
	reset_timer_counters();
	
	timer_set_alarm_value(LINE_GROUP, STEP_TIMER, jog_wait_time / PERIOD_uS);
	timer_set_alarm_value(LINE_GROUP, STOP_TIMER, jog_wait_time * zero_steps / PERIOD_uS);
	
	set_direction(1);
	
	timer_start(LINE_GROUP, STEP_TIMER);
	timer_start(LINE_GROUP, STOP_TIMER);
	motor_in_motion = true;
}

void axis::find_zero() {
	set_direction(0);
	
	reset_timer_counters();
	
	timer_set_alarm_value(LINE_GROUP, STEP_TIMER, (jog_wait_time * 4) / PERIOD_uS);
	timer_start(LINE_GROUP, STEP_TIMER);
	motor_in_motion = true;
}

void axis::reset_timer_counters() {
	timer_pause(LINE_GROUP, STEP_TIMER);
	timer_pause(LINE_GROUP, STOP_TIMER);
	timer_pause(CURV_GROUP, STEP_TIMER);
	timer_pause(CURV_GROUP, STOP_TIMER);
		
	timer_set_counter_value(LINE_GROUP, STEP_TIMER, 0);
	timer_set_counter_value(LINE_GROUP, STOP_TIMER, 0);
	
	timer_set_counter_value(CURV_GROUP, STEP_TIMER, 0);
	timer_set_counter_value(CURV_GROUP, STOP_TIMER, 0);	
	
	timer_enable_intr(LINE_GROUP, STEP_TIMER);
	timer_enable_intr(LINE_GROUP, STOP_TIMER);	
	
	timer_enable_intr(CURV_GROUP, STEP_TIMER);
	timer_enable_intr(CURV_GROUP, STOP_TIMER);
	
	timer_set_alarm(LINE_GROUP, STEP_TIMER, TIMER_ALARM_EN);
	timer_set_alarm(LINE_GROUP, STOP_TIMER, TIMER_ALARM_EN);
	
	timer_set_alarm(CURV_GROUP, STEP_TIMER, TIMER_ALARM_EN);
	timer_set_alarm(CURV_GROUP, STOP_TIMER, TIMER_ALARM_EN);	
}

void axis::setup_curve(std::vector<int> &info) {	
	set_feed_rate(info[7]);

	int r 		= info[2];	
	int x2		= info[3];
	int y2 		= info[4];	
	int x3 		= info[5];
	int y3 		= info[6];	
	int xo 		= 0;
	int mask	= 0;
	int yo 		= 0;		
	int fxy;
	int dx;
	int dy;	
	bool f;
	bool a;
	bool b;
	bool d 		= info[1];	
		
	step_vec.clear();
	dirs_vec.clear();
	
	std::cout << "R: " << r << "\tXi: " << x2 << "\tYi: " << y2 << "\tXf: " << x3 << "\tYf: " << y3 << "\twait time: " << pulse_period_us << '\n';
	
	do {
		fxy = x2*x2 + y2*y2 - r*r;
		dx = 2*x2;
		dy = 2*y2;
		
		f = (fxy < 0)? 0 : 1;
		a = (dx < 0)? 0 : 1;
		b = (dy < 0)? 0 : 1;
		
		mask = 0;
		if(f) mask += 8;
		if(a) mask += 4;
		if(b) mask += 2;
		if(d) mask += 1;
		
		xo = 0;
		yo = 0;
		switch(mask) {
			case 0:		yo = -1;	break;
			case 1:		xo = -1;	break;
			case 2:		xo = -1;	break;
			case 3:		yo = 1;		break;
			case 4:		xo = 1;		break;
			case 5:		yo = -1;	break;
			case 6:		yo = 1;		break;
			case 7:		xo = 1;		break;
			case 8:		xo = 1;		break;
			case 9:		yo = 1;		break;
			case 10:	yo = -1;	break;
			case 11:	xo = 1;		break;
			case 12:	yo = 1;		break;
			case 13:	xo = -1;	break;
			case 14:	xo = -1;	break;
			case 15:	yo = -1;	break;
		}
		
		if(x2 == 0 && abs(y2) == r && step_vec.size() != 0) {
			std::cout << "Adding " << backlash << " backlash steps in y at position: " << x2 << ", " << y2 << '\n';
			if(!x_axis) {
				step_vec.insert(step_vec.end(), backlash, 1);
				dirs_vec.insert(dirs_vec.end(), backlash, (y2 < 0)? 1 : 0);
			}
			else {
				step_vec.insert(step_vec.end(), backlash, 0);
				dirs_vec.insert(dirs_vec.end(), backlash, dirs_vec[dirs_vec.size() - 1]);
			}
		}
		else if(y2 == 0 && abs(x2) == r && step_vec.size() != 0) {
			std::cout << "Adding " << backlash << " backlash steps in x at position: " << x2 << ", " << y2 << '\n';
			if(x_axis) {
				step_vec.insert(step_vec.end(), backlash, 1);
				dirs_vec.insert(dirs_vec.end(), backlash, (x2 < 0)? 1 : 0);
			}
			else {
				step_vec.insert(step_vec.end(), backlash, 0);
				dirs_vec.insert(dirs_vec.end(), backlash, dirs_vec[dirs_vec.size() - 1]);
			}
		}
			
		if(x_axis) {
			step_vec.push_back(xo);
			dirs_vec.push_back((xo == 0)? dirs_vec[dirs_vec.size() - 1] : (bool)(xo+1));
		}
		else {
			step_vec.push_back(yo);
			dirs_vec.push_back((yo == 0)? dirs_vec[dirs_vec.size() - 1] : (bool)(yo+1));
		}
				
		x2 += xo;
		y2 += yo;
		
	}while(abs(x2 - x3) > 1 || abs(y2 - y3) > 1 || step_vec.size() < 2);
	
	if(x2 != x3) {
		if(x_axis){
			step_vec.push_back(1);
			dirs_vec.push_back(std::signbit(x2-x3));
		}
		else {
			step_vec.push_back(0);
			dirs_vec.push_back(dirs_vec[dirs_vec.size() - 1]);
		}
	}
	if(y2 != y3) {
		if(!x_axis){
			step_vec.push_back(1);
			dirs_vec.push_back(std::signbit(y2-y3));
		}
		else {
			step_vec.push_back(0);
			dirs_vec.push_back(dirs_vec[dirs_vec.size() - 1]);
		}
	}
	
	std::cout << "Curve Steps: " << step_vec.size() << '\n';
	spi->toggle_ready();
}
