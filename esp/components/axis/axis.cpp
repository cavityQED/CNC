#include "axis.h"

/*	Initialize static variables	*/
xQueueHandle axis::timer_event_queue = xQueueCreate(10, sizeof(bool));
bool axis::motor_direction = false;
bool axis::motor_in_motion = false;
int axis::position_steps = 0;

timer_info_t axis::static_info = {STEP_ONCE, PULSE};

axis::axis() {
	setup_gpio();
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
	
	//Make sure the motor is not enabled to run initially
	gpio_set_level(EN, 1);
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
	
	//Setup All Timers with Auto Reload
	timer_init(PERIODIC, PULSE, &timer_config);
	timer_init(PERIODIC, ACCEL, &timer_config);
	
//	timer_config.intr_type		= TIMER_INTR_MAX;
	timer_init(ONE_SHOT, PULSE, &timer_config);
	timer_init(ONE_SHOT, ACCEL, &timer_config);
	
	pulse_args.type = STEP_ONCE;
	pulse_args.timer = PULSE;
	
	stop_args.type = STOP_MOTOR;
	stop_args.timer = ACCEL;
			
	//Setup Timer Argument Structs
	memset(&accel_args, 0, sizeof(accel_args));
		
	//Enable Interrupts on the Timers
	timer_enable_intr(PERIODIC, PULSE);
	timer_enable_intr(PERIODIC, ACCEL);
	timer_enable_intr(ONE_SHOT, PULSE);
	timer_enable_intr(ONE_SHOT, ACCEL);

	//Add the Callbacks
	timer_isr_register(PERIODIC, PULSE, periodic_pulse_callback, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_isr_register(PERIODIC, ACCEL, periodic_accel_callback, &accel_args, ESP_INTR_FLAG_IRAM, NULL);	
	timer_isr_register(ONE_SHOT, PULSE, one_shot_pulse_callback, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_isr_register(ONE_SHOT, ACCEL, one_shot_accel_callback, NULL, ESP_INTR_FLAG_IRAM, NULL);
	
}

void axis::periodic_pulse_callback(void* arg) {
	timer_spinlock_take(PERIODIC);

	gpio_set_level(STEP, 1);
	gpio_set_level(STEP, 0);

//	WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1 << STEP));
	(motor_direction)? position_steps += 1 : position_steps -= 1;
//	WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1 << STEP));
/*		
	static_info.type = STEP_ONCE;
	bool x = true;
	xQueueSendFromISR(timer_event_queue, &x, NULL);
	
	if(*(bool*)arg) {
		timer_group_set_counter_enable_in_isr(PERIODIC, PULSE, TIMER_PAUSE);
		motor_in_motion = false;
	}
*/	
	timer_group_clr_intr_status_in_isr(PERIODIC, PULSE);
	timer_group_enable_alarm_in_isr(PERIODIC, PULSE);
	timer_spinlock_give(PERIODIC);
}

void axis::periodic_accel_callback(void* arg) {
	timer_spinlock_take(PERIODIC);
	
	accel_args_t* a = (accel_args_t*) arg;
	
	timer_group_set_alarm_value_in_isr(PERIODIC, PULSE, (a->pulseAlarmTimes[a->accelStep]) / PERIOD_uS);
	a->accelStep = a->accelStep - 1;
	
//	timer_group_clr_intr_status_in_isr(PERIODIC, PULSE);
	timer_group_clr_intr_status_in_isr(PERIODIC, ACCEL);
	timer_group_enable_alarm_in_isr(PERIODIC, ACCEL);
	timer_spinlock_give(PERIODIC);
}

void axis::one_shot_pulse_callback(void* arg) {
	timer_spinlock_take(ONE_SHOT);
	
	timer_group_set_counter_enable_in_isr(PERIODIC, PULSE, TIMER_PAUSE);
	timer_group_set_counter_enable_in_isr(ONE_SHOT, PULSE, TIMER_PAUSE);
	
//	static_info.type = (TIMER_EVENT) arg;
//	bool x = true;
//	xQueueSendFromISR(timer_event_queue, &x, NULL);
	
	motor_in_motion = false;
		
	timer_group_clr_intr_status_in_isr(ONE_SHOT, PULSE);
	timer_group_clr_intr_status_in_isr(PERIODIC, PULSE);
	
	timer_group_enable_alarm_in_isr(ONE_SHOT, PULSE);
	timer_spinlock_give(ONE_SHOT);
}

void axis::one_shot_accel_callback(void* arg) {
	timer_spinlock_take(ONE_SHOT);
	
	timer_group_set_counter_enable_in_isr(PERIODIC, ACCEL, TIMER_PAUSE);
	timer_group_set_counter_enable_in_isr(ONE_SHOT, ACCEL, TIMER_PAUSE);
	
	timer_group_clr_intr_status_in_isr(PERIODIC, ACCEL);
	timer_group_clr_intr_status_in_isr(ONE_SHOT, ACCEL);
	
	timer_group_enable_alarm_in_isr(ONE_SHOT, ACCEL);
	timer_spinlock_give(ONE_SHOT);
}

void axis::set_defaults() {
	SPR 				= 800;
	mm_per_step 		= .005;
	rpm 				= 150;
	move_with_accel 	= false;
	steps_to_move 		= 0;
	jog_once_steps 		= 0;
	max_travel_mm 		= 400;
	max_travel_steps 	= max_travel_mm / mm_per_step;
	position_steps		= 0;
	zero_steps			= 200;
	set_speed_rpm(rpm);
}

void axis::set_speed_rpm(int speed) {
	if(speed <= 0 || speed == rpm || motor_in_motion)
		return;
		
	rpm = speed;
	
	sps = rpm*SPR/60;
		
	pulse_period_us = 1000000/sps;
	std::cout << "Final pulse period: " << pulse_period_us << '\n';
	
	if(speed <= 300)
		move_with_accel = false;
	else {
		calculate_accel_params();
	}
}

void axis::calculate_accel_params() {
	int final_frequency = 1000/pulse_period_us;
	int init_frequency = 1000/init_pulse_period_us;
	accel_steps = final_frequency - init_frequency;
	accel_period_us = 10000;
	accel_move_steps = accel_period_us / init_pulse_period_us;
	
	if(accel_steps > 9) {
		std::cout << "Update Acceleration Calculation!\n";
		return;
	}
	
	for(int i = accel_steps-1; i > 0; i--) {
		accel_args.pulseAlarmTimes[i] = (uint64_t) 1000 / (final_frequency - i);
		accel_move_steps += accel_period_us / accel_args.pulseAlarmTimes[i];
		std::cout << "Time on step " << i << ": " << accel_args.pulseAlarmTimes[i] << '\n';
	}
	accel_args.pulseAlarmTimes[0] = (uint64_t) pulse_period_us;
	std::cout << "Time on step 0: " << accel_args.pulseAlarmTimes[0] << '\n';
	
	
	accel_args.accelStep = accel_steps - 1;

	std::cout << "Accel Move Steps: " << accel_move_steps << '\n';

	accel_move_time = (accel_steps * accel_period_us) + ((steps_to_move - accel_move_steps) * pulse_period_us);
	std::cout << "Total Move Time: " << accel_move_time << '\n';
	move_with_accel = true;
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

void axis::set_mm_to_move(int whole, int dec) {
	steps_to_move = (whole + (double)dec/100)/mm_per_step;
}

void axis::set_jog_speed_steps(int steps) {
	jog_once_steps = steps;
	std::cout << "Jog Steps: " << jog_once_steps << '\n';
}

void axis::set_jog_speed_mm(int mm) {
	jog_once_steps = mm / mm_per_step / 100;
}

void axis::enable_jog_mode(bool enable) {
	jog_mode = enable;
	if(jog_mode) {
		enable_step_mode(false);
		std::cout << "Jog Mode Enabled\n";
	}
	else
		std::cout << "Jog Mode Disabled\n";
}

void axis::enable_step_mode(bool enable) {
	step_mode = enable;
	if(step_mode) {
		std::cout << "Step Mode Enabled\n";
		enable_jog_mode(false);
	}
	else
		std::cout << "Step Mode Disabled\n";
}

void axis::set_mm_per_step(int mm, int ten_power) {
	mm_per_step = pow(10, -ten_power);
}

void axis::set_steps_per_revolution(int steps) {
	SPR = steps;
}

void axis::get_position_steps(unsigned char* sendbuf) {
	sendbuf[1] = position_steps%255;
	sendbuf[2] = (position_steps/255)%255;
	sendbuf[3] = position_steps/255/255;
	std::cout << "Position: " << mm_per_step * position_steps << "mm\n";
}	

void axis::move() {
	if(motor_in_motion) {
		std::cout << "Motor in motion already!\n";
		return;
	}
		
	if(jog_mode) 
		move_jog_mode();
	else if(step_mode) 
		move_step_mode();
	else
		return;
}

void axis::move_jog_mode() {
	if(!jog_mode || motor_in_motion)
		return;
		
	if(motor_direction && (position_steps + jog_once_steps) > max_travel_steps)
		return;
	if(!motor_direction && jog_once_steps > position_steps)
		return;
		
	reset_timer_counters();
	
	timer_set_alarm_value(PERIODIC, PULSE, init_pulse_period_us / PERIOD_uS);
	timer_set_alarm_value(ONE_SHOT, PULSE, ((init_pulse_period_us * jog_once_steps) + 20) / PERIOD_uS);
	gpio_set_level(EN, 0);
	timer_start(PERIODIC, PULSE);
	timer_start(ONE_SHOT, PULSE);
	motor_in_motion = true;
}

void axis::move_step_mode() {
	if(!step_mode || motor_in_motion){
		std::cout << "Motor in motion already!!\n";
		return;
	}
		
	reset_timer_counters();
		
	if(!move_with_accel) {				
		timer_set_alarm_value(PERIODIC, PULSE, pulse_period_us / PERIOD_uS);
		timer_set_alarm_value(ONE_SHOT, PULSE, ((pulse_period_us * steps_to_move) / PERIOD_uS));
		gpio_set_level(EN, 0);
		timer_start(PERIODIC, PULSE);
		timer_start(ONE_SHOT, PULSE);
		motor_in_motion = true;
	}
	else {	
		std::cout << "Moving with acceleration\n";
		accel_args.accelStep = accel_steps - 1;
		
		std::cout << "Accel period: " << accel_period_us << '\n';
		std::cout << "Accel Time: " << accel_period_us * accel_steps << '\n';
		timer_set_alarm_value(PERIODIC, PULSE, init_pulse_period_us / PERIOD_uS);
		timer_set_alarm_value(ONE_SHOT, PULSE, accel_move_time / PERIOD_uS);
		timer_set_alarm_value(PERIODIC, ACCEL, accel_period_us / PERIOD_uS);
		timer_set_alarm_value(ONE_SHOT, ACCEL, accel_period_us * accel_steps / PERIOD_uS);
		
		gpio_set_level(EN, 0);
		
		timer_start(PERIODIC, PULSE);
		timer_start(PERIODIC, ACCEL);
		timer_start(ONE_SHOT, PULSE);
		timer_start(ONE_SHOT, ACCEL);
		motor_in_motion = true;
	}
}

void axis::stop() {
	timer_pause(PERIODIC, PULSE);
	timer_pause(ONE_SHOT, PULSE);
	timer_pause(PERIODIC, ACCEL);
	timer_pause(ONE_SHOT, ACCEL);
	
	motor_in_motion = false;
}

void axis::zero_interlock_stop() {
	timer_pause(PERIODIC, PULSE);
	timer_pause(ONE_SHOT, PULSE);
	timer_pause(PERIODIC, ACCEL);
	timer_pause(ONE_SHOT, ACCEL);
	
	motor_in_motion = false;
	
	position_steps = -zero_steps;
	
	reset_timer_counters();
	
	timer_set_alarm_value(PERIODIC, PULSE, init_pulse_period_us / PERIOD_uS);
	timer_set_alarm_value(ONE_SHOT, PULSE, init_pulse_period_us * zero_steps / PERIOD_uS);
	
	set_direction(1);
	
	timer_start(PERIODIC, PULSE);
	timer_start(ONE_SHOT, PULSE);
	motor_in_motion = true;
}

void axis::zero() {
	set_direction(0);
	
	reset_timer_counters();
	
	timer_set_alarm_value(PERIODIC, PULSE, (init_pulse_period_us * 4) / PERIOD_uS);
	timer_start(PERIODIC, PULSE);
	motor_in_motion = true;
}

void axis::reset_timer_counters() {
	timer_pause(PERIODIC, PULSE);
	timer_pause(PERIODIC, ACCEL);	
	timer_set_counter_value(PERIODIC, PULSE, 0);
	timer_set_counter_value(PERIODIC, ACCEL, 0);	
	timer_enable_intr(PERIODIC, PULSE);
	timer_enable_intr(PERIODIC, ACCEL);	
	timer_set_alarm(PERIODIC, PULSE, TIMER_ALARM_EN);
	timer_set_alarm(PERIODIC, ACCEL, TIMER_ALARM_EN);

	timer_pause(ONE_SHOT, PULSE);
	timer_pause(ONE_SHOT, ACCEL);
	timer_set_counter_value(ONE_SHOT, PULSE, 0);
	timer_set_counter_value(ONE_SHOT, ACCEL, 0);
	timer_enable_intr(ONE_SHOT, PULSE);
	timer_enable_intr(ONE_SHOT, ACCEL);
	timer_set_alarm(ONE_SHOT, PULSE, TIMER_ALARM_EN);
	timer_set_alarm(ONE_SHOT, ACCEL, TIMER_ALARM_EN);	

}

void axis::run() {
	xTaskCreatePinnedToCore(axis::timer_task, "Timer Task", 2048, NULL, 5, NULL, 1);
}

void axis::timer_task(void* arg) {
	bool x;
	while(1) {
		xQueueReceive(timer_event_queue, &x, portMAX_DELAY);
//			WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1 << STEP));
			gpio_set_level(STEP, 1);
			gpio_set_level(STEP, 0);
			(motor_direction)? position_steps += 1 : position_steps -= 1;
//			WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1 << STEP));	

	}
}
		
