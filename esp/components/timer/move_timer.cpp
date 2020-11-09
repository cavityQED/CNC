#include "move_timer.h"

//Initialize the static class variables

std::vector<bool>	Timer::pulse_step_vector {0};
std::vector<bool*>	Timer::pulse_dirs_vector {0};

int					Timer::cur_pulse = 0;
int*				Timer::position_steps = 0;
bool				Timer::direction = 0;
bool				Timer::accel_mode = true;
bool*				Timer::in_motion = 0;

Timer::Timer() {
	configure_timers();
	configure_gpio();
}
	
void Timer::configure_timers() {
	timer_config_t timer_config;
	memset(&timer_config, 0, sizeof(timer_config));
	
	//Parameters shared between step and stop timers
	timer_config.divider		= FINAL_TICK_PERIOD * BASE_TIMER_FREQUENCY;
	timer_config.counter_dir	= TIMER_COUNT_UP;
	timer_config.counter_en		= TIMER_PAUSE;
	timer_config.alarm_en		= TIMER_ALARM_EN;
	timer_config.intr_type		= TIMER_INTR_MAX;
	
	//Step timer will auto reload
	timer_config.auto_reload	= TIMER_AUTORELOAD_EN;
	esp_err_t err = timer_init(PULSE_GROUP, STEP_TIMER, &timer_config);
	ESP_ERROR_CHECK(err);
	
	//Stop timer will not auto reload, only fires once to stop the motor
	timer_config.auto_reload	= TIMER_AUTORELOAD_DIS;
	err = timer_init(PULSE_GROUP, STOP_TIMER, &timer_config);
	ESP_ERROR_CHECK(err);
	
	//Enable interrupts
	timer_enable_intr(PULSE_GROUP, STEP_TIMER);
	timer_enable_intr(PULSE_GROUP, STOP_TIMER);
	
	//Register the callback functions
	timer_isr_register(PULSE_GROUP, STEP_TIMER, pulse_step_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_isr_register(PULSE_GROUP, STOP_TIMER, pulse_stop_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
}//configure_timers
	
void Timer::configure_gpio() {
	esp_err_t err;
	
	gpio_pad_select_gpio(STEP_PIN);
	err = gpio_set_direction(STEP_PIN, GPIO_MODE_OUTPUT);
	ESP_ERROR_CHECK(err);
	
	gpio_pad_select_gpio(DIR_PIN);
	err = gpio_set_direction(DIR_PIN, GPIO_MODE_OUTPUT);
	ESP_ERROR_CHECK(err);
	
	gpio_pad_select_gpio(EN_PIN);
	err = gpio_set_direction(EN_PIN, GPIO_MODE_OUTPUT);
	ESP_ERROR_CHECK(err);
		
	gpio_set_level(EN_PIN, 0);
		
}//configure_gpio
	
void Timer::move_setup(	const std::vector<bool> &step,
						const std::vector<bool*> &dirs,
						int wait_time_us,
						int* pos,
						bool* motion,
						bool accel)
{
	//SHOULD CHECK IF pos OR motion ARE NULLPTR
		
	reset();
	pulse_step_vector = step;
	pulse_dirs_vector = dirs;
	position_steps = pos;
	in_motion = motion;
	cur_pulse = 0;
	direction = *pulse_dirs_vector[0];
	gpio_set_level(DIR_PIN, direction);
	
	esp_err_t err = timer_set_alarm_value(PULSE_GROUP, STEP_TIMER, wait_time_us);
	ESP_ERROR_CHECK(err);
	err = timer_set_alarm_value(PULSE_GROUP, STOP_TIMER, wait_time_us * pulse_step_vector.size());
	ESP_ERROR_CHECK(err);
}//setup_move
	
void Timer::start_move() {
	*in_motion = true;
	timer_start(PULSE_GROUP, STEP_TIMER);
	timer_start(PULSE_GROUP, STOP_TIMER);
}
	
void Timer::reset() {
	timer_pause(PULSE_GROUP, STEP_TIMER);
	timer_pause(PULSE_GROUP, STOP_TIMER);
	
	timer_set_counter_value(PULSE_GROUP, STEP_TIMER, 0);
	timer_set_counter_value(PULSE_GROUP, STOP_TIMER, 0);
	
	timer_enable_intr(PULSE_GROUP, STEP_TIMER);
	timer_enable_intr(PULSE_GROUP, STOP_TIMER);
	
	timer_set_alarm(PULSE_GROUP, STEP_TIMER, TIMER_ALARM_EN);
	timer_set_alarm(PULSE_GROUP, STOP_TIMER, TIMER_ALARM_EN);
}//reset
	
void Timer::pulse_step_isr(void* arg) {
	timer_spinlock_take(PULSE_GROUP);
		
	gpio_set_level(STEP_PIN, pulse_step_vector[cur_pulse]);
	gpio_set_level(STEP_PIN, 0);
	
	(direction)? *position_steps += (int)pulse_step_vector[cur_pulse] : *position_steps -= (int)pulse_step_vector[cur_pulse];
	
	cur_pulse += (int)accel_mode;
	direction = *pulse_dirs_vector[cur_pulse];
	gpio_set_level(DIR_PIN, direction);
	
	timer_group_clr_intr_status_in_isr(PULSE_GROUP, STEP_TIMER);
	timer_group_enable_alarm_in_isr(PULSE_GROUP, STEP_TIMER);
	timer_spinlock_give(PULSE_GROUP);
}//pulse_step_isr

void Timer::pulse_stop_isr(void* arg) {
	timer_spinlock_take(PULSE_GROUP);
	
	timer_group_set_counter_enable_in_isr(PULSE_GROUP, STEP_TIMER, TIMER_PAUSE);
	timer_group_set_counter_enable_in_isr(PULSE_GROUP, STOP_TIMER, TIMER_PAUSE);
	
	*in_motion = false;
	
	timer_group_clr_intr_status_in_isr(PULSE_GROUP, STOP_TIMER);
	timer_group_enable_alarm_in_isr(PULSE_GROUP, STOP_TIMER);
	timer_spinlock_give(PULSE_GROUP);
}//pulse_stop_isr
