#include "move_timer.h"

//Initialize the static class variables
std::shared_ptr<std::vector<bool>>		Timer::pulse_step_vector {0};
std::shared_ptr<std::vector<bool>>		Timer::pulse_dirs_vector {0};

int			Timer::cur_pulse			= 0;
int			Timer::accel_stop_pulse		= 0;
int			Timer::decel_start_pulse	= 0;			
int			Timer::final_pulse			= 0;

int			Timer::divider_us			= BASE_TIMER_FREQUENCY * 0.000001;
bool		Timer::step_direction		= 0;
double		Timer::divider_tp			= 1;
double		Timer::cur_time				= 0;
double		Timer::const_time			= 0;
double		Timer::min_wait_time		= 0;
int*		Timer::position_steps		= 0;
bool*		Timer::in_motion			= 0;

Timer::Timer(int* pos, bool* motion)
{
	configure_timers();
	configure_gpio();

	position_steps = pos;
	in_motion = motion;
}
	
void Timer::configure_timers() {
	timer_config_t	timer_config;
	memset(&timer_config, 0, sizeof(timer_config));

	timer_config.divider		= divider_us;		//Set the divider so default timer counter period is 1 microsecond
	timer_config.counter_dir	= TIMER_COUNT_UP;	//Set counter to increase in value
	timer_config.counter_en		= TIMER_PAUSE;
	timer_config.intr_type		= TIMER_INTR_LEVEL;

	/*	PULSE_TIMER config
	*		Pulse timer should auto-reload and have its alarm enabled
	*/
	timer_config.auto_reload	= TIMER_AUTORELOAD_EN;
	timer_config.alarm_en		= TIMER_ALARM_EN;

	/*	Initiate the pulse timers
	*		Initiate timer and register the interrupt callback 
	*/
	timer_init(LINEAR_GROUP, PULSE_TIMER, &timer_config);
	timer_init(VECTOR_GROUP, PULSE_TIMER, &timer_config);
	timer_isr_register(LINEAR_GROUP, PULSE_TIMER, linear_pulse_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_isr_register(VECTOR_GROUP, PULSE_TIMER, vector_pulse_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);


	/*	SECONDS_TIMER config
	*		Seconds timer does not auto-reload (one-shot) and does not have an
	*		alarm enabled; counts until stopped. Used to get time passed in seconds
	*/
	timer_config.auto_reload	= TIMER_AUTORELOAD_DIS;
	timer_config.alarm_en		= TIMER_ALARM_DIS;

	/*	Initiate the seconds timers	*/
	timer_init(LINEAR_GROUP, SECONDS_TIMER, &timer_config);
	timer_init(VECTOR_GROUP, SECONDS_TIMER, &timer_config);

}//configure_timers
	
void Timer::configure_gpio() {
	/*	Set the pulse, direction, and enable pins as outputs	*/
	esp_err_t error;	
	gpio_pad_select_gpio(PULSE_PIN);
/*	error =*/ gpio_set_direction(PULSE_PIN, GPIO_MODE_OUTPUT);

	gpio_pad_select_gpio(DIR_PIN);
	error = gpio_set_direction(DIR_PIN, GPIO_MODE_OUTPUT);
	if(error != ESP_OK) {
		std::cout << "Error setting direction pin\n";
	}

	gpio_pad_select_gpio(EN_PIN);
/*	error =*/ gpio_set_direction(EN_PIN, GPIO_MODE_OUTPUT);

}//configure_gpio

void Timer::linear_move_config(	int initial_wait_time_us,
								int final_wait_time_us,
								int accel_steps_per_s_per_s,
								int steps_to_move,
								bool dir)
{
	reset();


	if(accel_steps_per_s_per_s * final_wait_time_us != 0)
	{
		//Time to stop accelration phase
		const_time = 1000000.0 / (double)(accel_steps_per_s_per_s * final_wait_time_us);
		std::cout << "Time to accelerate to final speed: " << const_time << '\n';
	}

	final_pulse = steps_to_move;
	min_wait_time = final_wait_time_us / 1000000.0;

	//Set direction
	step_direction = dir;
	gpio_set_level(DIR_PIN, dir);

	//Initial divider value
	int d0 = divider_us * initial_wait_time_us / final_wait_time_us;
	timer_set_divider(LINEAR_GROUP, PULSE_TIMER, d0);

	//Divider time product
	divider_tp = BASE_TIMER_FREQUENCY / (double)accel_steps_per_s_per_s / (double)final_wait_time_us;
	if(!divider_tp)
		divider_tp = 1;
	std::cout << "Divider Time Product: " << divider_tp << '\n';

	//Set Pulse timer alarm value
	timer_set_alarm_value(LINEAR_GROUP, PULSE_TIMER, final_wait_time_us);
}

void Timer::vector_move_config(	int initial_wait_time_us,
								int final_wait_time_us,
								int accel_steps_per_s_per_s,
								std::shared_ptr<std::vector<bool>> step_vec,
								std::shared_ptr<std::vector<bool>> dirs_vec)
{
	reset();

	if(accel_steps_per_s_per_s * final_wait_time_us != 0)
	{
		//Time to stop accelration phase
		const_time = 1000000.0 / (double)(accel_steps_per_s_per_s * final_wait_time_us);
		std::cout << "Time to accelerate to final speed: " << const_time << '\n';
	}
	else
	{
		const_time = 0;
		decel_start_pulse = BASE_TIMER_FREQUENCY;
	}

	pulse_step_vector = step_vec;
	pulse_dirs_vector = dirs_vec;

	final_pulse = pulse_step_vector->size();
	min_wait_time = final_wait_time_us / 1000000.0;

	//Initial divider value
	int d0 = divider_us * initial_wait_time_us / final_wait_time_us;
	timer_set_divider(VECTOR_GROUP, PULSE_TIMER, d0);

	//Divider time product
	divider_tp = BASE_TIMER_FREQUENCY / (double)accel_steps_per_s_per_s / (double)final_wait_time_us;
	if(!divider_tp)
		divider_tp = 1;
	std::cout << "Divider Time Product: " << divider_tp << '\n';

	//Set Pulse timer alarm value
	timer_set_alarm_value(VECTOR_GROUP, PULSE_TIMER, final_wait_time_us);

	//Set the initial direction
	step_direction = dirs_vec->at(0);
	gpio_set_level(DIR_PIN, step_direction);
}								

void Timer::start_linear()
{
	timer_start(LINEAR_GROUP, PULSE_TIMER);
	timer_start(LINEAR_GROUP, SECONDS_TIMER);
	*in_motion = true;
}

void Timer::start_vector()
{
	timer_start(VECTOR_GROUP, PULSE_TIMER);
	timer_start(VECTOR_GROUP, SECONDS_TIMER);
	*in_motion = true;
}
	
void Timer::reset() {
	cur_pulse = 0;
	const_time = 0;
	decel_start_pulse = BASE_TIMER_FREQUENCY;

	timer_pause					(LINEAR_GROUP, PULSE_TIMER);
	timer_enable_intr			(LINEAR_GROUP, PULSE_TIMER);
	timer_set_counter_value		(LINEAR_GROUP, PULSE_TIMER, 0);
	timer_set_alarm				(LINEAR_GROUP, PULSE_TIMER, TIMER_ALARM_EN);

	timer_pause					(LINEAR_GROUP, SECONDS_TIMER);
	timer_set_counter_value		(LINEAR_GROUP, SECONDS_TIMER, 0);

	timer_pause					(VECTOR_GROUP, PULSE_TIMER);
	timer_enable_intr			(VECTOR_GROUP, PULSE_TIMER);
	timer_set_counter_value		(VECTOR_GROUP, PULSE_TIMER, 0);
	timer_set_alarm				(VECTOR_GROUP, PULSE_TIMER, TIMER_ALARM_EN);

	timer_pause					(VECTOR_GROUP, SECONDS_TIMER);
	timer_set_counter_value		(VECTOR_GROUP, SECONDS_TIMER, 0);
}//reset
	
void Timer::linear_pulse_isr(void* arg)
{
	timer_spinlock_take(LINEAR_GROUP);
		
	gpio_set_level(PULSE_PIN, 1);
	gpio_set_level(PULSE_PIN, 0);
	
	*position_steps += (step_direction)? 1 : -1;
	timer_get_counter_time_sec(LINEAR_GROUP, SECONDS_TIMER, &cur_time);
	cur_pulse++;

	if(cur_pulse >= final_pulse)
	{
		/*	Stop the motor	
		*		Set the in_motion variable to false
		*/
		*in_motion = false;
		timer_group_set_counter_enable_in_isr(LINEAR_GROUP, PULSE_TIMER, TIMER_PAUSE);
	}

	else if(cur_time < const_time && cur_pulse < decel_start_pulse)
	{
		//	Acceleration phase
		//		Decrease the timer divider to decrease the time between counter ticks
		timer_set_divider(LINEAR_GROUP, PULSE_TIMER, (int)(divider_tp / cur_time) );
	}

	else if(std::fabs(cur_time - const_time) < min_wait_time)
	{
		//	Start constant speed phase, use current pulse to set pulse number to start deceleration 
		decel_start_pulse = final_pulse - cur_pulse;
	}

	else if(cur_pulse == decel_start_pulse)
	{
		//	Reset seconds timer
		timer_set_counter_value(LINEAR_GROUP, SECONDS_TIMER, 0);
	}

	else if(cur_pulse > decel_start_pulse)
	{
		//	Deceleration phase
		timer_set_divider(LINEAR_GROUP, PULSE_TIMER, (int)(divider_tp/std::fabs(const_time - cur_time)) );
	}

	timer_group_clr_intr_status_in_isr	(LINEAR_GROUP, PULSE_TIMER);
	timer_group_enable_alarm_in_isr		(LINEAR_GROUP, PULSE_TIMER);
	timer_spinlock_give					(LINEAR_GROUP);
}//pulse_step_isr

void Timer::vector_pulse_isr(void* arg)
{
	timer_spinlock_take(VECTOR_GROUP);

	if(pulse_step_vector->at(cur_pulse))
	{
		gpio_set_level(PULSE_PIN, pulse_step_vector->at(cur_pulse));
		gpio_set_level(PULSE_PIN, 0);
		
		*position_steps += (step_direction)? 1 : -1;
	}
	
	timer_get_counter_time_sec(VECTOR_GROUP, SECONDS_TIMER, &cur_time);
	cur_pulse++;

	step_direction = pulse_dirs_vector->at(cur_pulse);
	gpio_set_level(DIR_PIN, step_direction);

	if(cur_time < const_time && cur_pulse < decel_start_pulse)
	{
		//	Acceleration phase
		//		Decrease the timer divider to decrease the time between counter ticks
		timer_set_divider(VECTOR_GROUP, PULSE_TIMER, (int)(divider_tp / cur_time));
	}

	else if(std::fabs(cur_time - const_time) < min_wait_time)
	{
		accel_stop_pulse = cur_pulse;
		decel_start_pulse = final_pulse - accel_stop_pulse;
	}

	else if(cur_pulse == decel_start_pulse)
	{
		//	Reset seconds timer
		timer_set_counter_value(VECTOR_GROUP, SECONDS_TIMER, 0);
	}

	else if(cur_pulse > decel_start_pulse)
	{
		timer_set_divider(VECTOR_GROUP, PULSE_TIMER, (int)(divider_tp/(const_time - cur_time)) );
	}

	if(cur_pulse >= final_pulse)
	{
		/*	Stop the motor	
		*		Set the in_motion variable to false
		*/
		timer_group_set_counter_enable_in_isr(VECTOR_GROUP, PULSE_TIMER, TIMER_PAUSE);
		*in_motion = false;
	}

	timer_group_clr_intr_status_in_isr	(VECTOR_GROUP, PULSE_TIMER);
	timer_group_enable_alarm_in_isr		(VECTOR_GROUP, PULSE_TIMER);
	timer_spinlock_give					(VECTOR_GROUP);
}