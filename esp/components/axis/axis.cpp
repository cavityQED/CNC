#include "axis.h"

/*	Initialize static variables	*/
int				axis::m_jog_steps			= 0;	
int				axis::m_cur_pulse			= 0;		
int				axis::m_accel_pulse			= 0;		
int				axis::m_decel_pulse			= 0;		
int				axis::m_final_pulse			= 0;		
int				axis::m_step_position		= 0;
int				axis::m_radius				= 0;
int				axis::m_jog_period_us		= 100;
int				axis::m_rapid_period_us		= 100;
int				axis::m_spmm				= 200;			
int				axis::m_max_mm				= 300;			
int				axis::m_init_period_us		= 2500;	
int				axis::m_accel				= 40000;			
int				axis::m_max_steps			= 60000;	

bool			axis::m_direction			= false;		
bool			axis::m_motion				= false;			
bool			axis::m_jog_mode			= false;			
bool			axis::m_line_mode			= false;		
bool			axis::m_curv_mode			= false;		
bool			axis::m_sync_mode			= false;
bool 			axis::m_CW					= false;
bool			axis::m_homed				= false;

AXIS			axis::m_step_axis			= x_axis;		
AXIS			axis::m_axis				= x_axis;

double			axis::m_pulse_period_sec	= 0;	
double			axis::m_divider_min			= (double)TIMER_BASE_CLK / 1000000.0;		
double			axis::m_divider_max			= 1;		
double			axis::m_divider				= 1;			
double			axis::m_cur_time			= 0;			
double			axis::m_accel_time			= 0;		
double 			axis::m_slope				= 0;

position_t		axis::m_cur_pos				= {0, 0, 0};			
position_t		axis::m_end_pos				= {0, 0, 0};			
position_t		axis::m_jog_pos				= {0, 0, 0};
SpiClient*		axis::m_spi					= nullptr;
esp_err_t		axis::m_esp_err				= ESP_OK;
xQueueHandle	axis::m_syncSem				= xSemaphoreCreateBinary();

void axis::check_error(const char* msg)
{
	if(m_esp_err != ESP_OK) {
		std::cout << msg << '\n' << esp_err_to_name(m_esp_err);
	}
}

void axis::setup_gpio()
{
	/*	Setup the Sync Pin	*/
	gpio_config_t io_conf;
	memset(&io_conf, 0, sizeof(io_conf));
	io_conf.intr_type		= GPIO_INTR_POSEDGE;
	io_conf.mode 			= GPIO_MODE_INPUT;
	io_conf.pull_down_en	= GPIO_PULLDOWN_ENABLE;
	io_conf.pin_bit_mask	= (1 << SYNC_PIN);
	
	gpio_config(&io_conf);	
	gpio_set_intr_type(SYNC_PIN, GPIO_INTR_POSEDGE);
	gpio_isr_handler_add(SYNC_PIN, syncSem_release_isr, NULL);

	/*	Set the pulse, direction, and enable pins as outputs	*/
	gpio_pad_select_gpio(PULSE_PIN);
	m_esp_err = gpio_set_direction(PULSE_PIN, GPIO_MODE_OUTPUT);
	check_error("PULSE_PIN Configure:");

	gpio_pad_select_gpio(DIR_PIN);
	m_esp_err = gpio_set_direction(DIR_PIN, GPIO_MODE_OUTPUT);
	check_error("DIR_PIN Configure:");

	gpio_pad_select_gpio(EN_PIN);
	m_esp_err = gpio_set_direction(EN_PIN, GPIO_MODE_OUTPUT);
	check_error("EN_PIN Configure:");

	gpio_set_level(EN_PIN, 1);
}

void axis::setup_timers()
{
	timer_config_t	timer_config;
	memset(&timer_config, 0, sizeof(timer_config));

	timer_config.divider		= m_divider_min;	//Set the divider so default timer counter period is 1 microsecond
	timer_config.counter_dir	= TIMER_COUNT_UP;	//Set counter to increase in value
	timer_config.counter_en		= TIMER_PAUSE;
	timer_config.intr_type		= TIMER_INTR_LEVEL;

	/*	PULSE_TIMER config
	*		Pulse timer should auto-reload and have its alarm enabled
	*/
	timer_config.auto_reload	= TIMER_AUTORELOAD_EN;
	timer_config.alarm_en		= TIMER_ALARM_EN;

	/*	Initiate the pulse timers
	*		Initiate timer and register the interrupt handler 
	*/
	timer_init(VECTOR_GROUP, PULSE_TIMER, &timer_config);
	timer_init(SCALAR_GROUP, PULSE_TIMER, &timer_config);
	timer_isr_register(VECTOR_GROUP, PULSE_TIMER, vector_move_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_isr_register(SCALAR_GROUP, PULSE_TIMER, scalar_move_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);


	/*	SECONDS_TIMER config
	*		Seconds timer does not auto-reload (one-shot) and does not have an
	*		alarm enabled; counts until stopped. Used to get time passed in seconds
	*/
	timer_config.auto_reload	= TIMER_AUTORELOAD_DIS;
	timer_config.alarm_en		= TIMER_ALARM_DIS;

	/*	Initiate the seconds timers	*/
	timer_init(VECTOR_GROUP, SECONDS_TIMER, &timer_config);
	timer_init(SCALAR_GROUP, SECONDS_TIMER, &timer_config);
}

void axis::reset_timers()
{
	timer_pause					(VECTOR_GROUP, PULSE_TIMER);
	timer_disable_intr			(VECTOR_GROUP, PULSE_TIMER);
	timer_set_counter_value		(VECTOR_GROUP, PULSE_TIMER, 0);
	timer_set_alarm				(VECTOR_GROUP, PULSE_TIMER, TIMER_ALARM_EN);

	timer_pause					(VECTOR_GROUP, SECONDS_TIMER);
	timer_set_counter_value		(VECTOR_GROUP, SECONDS_TIMER, 0);

	timer_pause					(SCALAR_GROUP, PULSE_TIMER);
	timer_disable_intr			(SCALAR_GROUP, PULSE_TIMER);
	timer_set_counter_value		(SCALAR_GROUP, PULSE_TIMER, 0);
	timer_set_alarm				(SCALAR_GROUP, PULSE_TIMER, TIMER_ALARM_EN);

	timer_pause					(SCALAR_GROUP, SECONDS_TIMER);
	timer_set_counter_value		(SCALAR_GROUP, SECONDS_TIMER, 0);
}

void axis::pause_timers(bool pause)
{
	if(!pause && m_motion)
	{
		timer_start(VECTOR_GROUP, PULSE_TIMER);
		timer_start(VECTOR_GROUP, SECONDS_TIMER);
		timer_start(SCALAR_GROUP, PULSE_TIMER);
		timer_start(SCALAR_GROUP, SECONDS_TIMER);
	}

	else
	{
		timer_pause(VECTOR_GROUP, PULSE_TIMER);
		timer_pause(VECTOR_GROUP, SECONDS_TIMER);
		timer_pause(SCALAR_GROUP, PULSE_TIMER);
		timer_pause(SCALAR_GROUP, SECONDS_TIMER);
	}
}

void axis::enable_jog_mode(bool enable)
{
	m_jog_mode = enable;
	if(m_jog_mode) {
		std::cout << "\tJog Mode Enabled\n";
		enable_line_mode(false);
		enable_curv_mode(false);
		enable_sync_mode(false);
	}
	else
		std::cout << "\tJog Mode Disabled\n";
}

void axis::enable_line_mode(bool enable)
{
	m_line_mode = enable;
	if(m_line_mode) {
		std::cout << "\tLine Mode Enabled\n";
		enable_jog_mode(false);
		enable_curv_mode(false);
	}
	else
		std::cout << "\tStep Mode Disabled\n";
}

void axis::enable_curv_mode(bool enable)
{
	m_curv_mode = enable;
	if(m_curv_mode) {
		std::cout << "\tCurve Mode Enabled\n";
		enable_jog_mode(false);
		enable_line_mode(false);
	}
	else
		std::cout << "\tCurve Mode Disabled\n";
}

void axis::enable_sync_mode(bool enable)
{
	m_sync_mode = enable;
	if(m_sync_mode) {
		std::cout << "\tSync Mode Enabled\n";
	}
	else
		std::cout << "\tSync Mode Disabled\n";
}

void axis::vector_move(	const position_t& vec,
						int final_period_us,
						int start_period_us,
						int accel)
{
	if(final_period_us < 0)
		final_period_us = m_rapid_period_us;

	switch(m_axis)
	{
		case x_axis:
			m_final_pulse = std::abs(vec.x);
			break;
		case y_axis:
			m_final_pulse = std::abs(vec.y);
			break;
		case z_axis:
			m_final_pulse = std::abs(vec.z);
			break;
		default:
			break;
	}

	m_cur_time			= 0;
	m_cur_pulse			= 0;
	m_cur_pos			= {0, 0, 0};
	m_end_pos			= vec;
	m_accel				= accel;
	m_init_period_us	= start_period_us;
	m_decel_pulse		= m_final_pulse;
	m_pulse_period_sec	= (double)final_period_us / 1000000.0;
	m_accel_time 		= 1.0 / m_pulse_period_sec / (double)m_accel;
	m_divider_max 		= m_divider_min * m_init_period_us / final_period_us;
	m_slope 			= (double)(vec.y)/(double)(vec.x);
	
	std::cout << "\t*****Vector Move Setup*****\n";
	std::cout << "\tAxis:\t\t\t"			<< (int)m_axis			<< '\n';
	std::cout << "\tEnd:\t\t\t"				<< vec;
	std::cout << "\tFinal Pulse:\t\t"		<< m_final_pulse 		<< '\n';
	std::cout << "\tSlope:\t\t\t"			<< m_slope 				<< '\n';
	std::cout << "\tPulse Period (us):\t"	<< final_period_us		<< "\n";
	std::cout << "\tPulse Period (s):\t"	<< m_pulse_period_sec	<< "\n";
	std::cout << "\tAccel Time (s):\t\t"	<< m_accel_time			<< '\n';
	std::cout << "\tInitial Divider:\t"		<< m_divider_max		<< '\n';

	linear_interpolation_2D();

	reset_timers();
	timer_enable_intr(VECTOR_GROUP, PULSE_TIMER);
	timer_set_divider(VECTOR_GROUP, PULSE_TIMER, m_divider_max);
	timer_set_alarm_value(VECTOR_GROUP, PULSE_TIMER, final_period_us);

	if(m_sync_mode)
	{
		m_spi->toggle_ready();
		xSemaphoreTake(m_syncSem, portMAX_DELAY);
		if(!m_final_pulse)
			return;
	}

	timer_start(VECTOR_GROUP, PULSE_TIMER);
	timer_start(VECTOR_GROUP, SECONDS_TIMER);
	m_motion = true;
}

void axis::circle_move(	const position_t& start,
						const position_t& end,
						int final_period_us,
						int r,
						bool cw,
						int start_period_us,
						int accel)
{
	m_cur_time			= 0;
	m_cur_pulse			= 0;
	m_cur_pos			= start;
	m_end_pos			= end;
	m_accel				= accel;
	m_radius			= r;
	m_CW				= cw;
	m_init_period_us	= start_period_us;
	m_final_pulse		= TIMER_BASE_CLK;
	m_decel_pulse		= TIMER_BASE_CLK;
	m_pulse_period_sec	= (double)final_period_us / 1000000.0;
	m_accel_time 		= 1.0 / m_pulse_period_sec / (double)m_accel;
	m_divider_max 		= m_divider_min * m_init_period_us / final_period_us;
	
	std::cout << "\t*****Circle Move Setup*****\n";
	std::cout << "\tAxis:\t\t\t"			<< (int)m_axis			<< '\n';
	std::cout << "\tStart:\t\t\t" 			<< start;
	std::cout << "\tEnd:\t\t\t"				<< end;
	std::cout << "\tRadius:\t\t\t"			<< m_radius				<< '\n';
	std::cout << "\tPulse Period (us):\t"	<< final_period_us		<< "\n";
	std::cout << "\tPulse Period (s):\t"	<< m_pulse_period_sec	<< "\n";
	std::cout << "\tAccel Time (s):\t\t"	<< m_accel_time			<< '\n';
	std::cout << "\tInitial Divider:\t"		<< m_divider_max		<< '\n';

	circular_interpolation_2D();

	reset_timers();
	timer_enable_intr(VECTOR_GROUP, PULSE_TIMER);
	timer_set_divider(VECTOR_GROUP, PULSE_TIMER, m_divider_max);
	timer_set_alarm_value(VECTOR_GROUP, PULSE_TIMER, final_period_us);

	if(m_sync_mode)
	{
		m_spi->toggle_ready();
		xSemaphoreTake(m_syncSem, portMAX_DELAY);
		if(!m_final_pulse)
			return;
	}

	timer_start(VECTOR_GROUP, PULSE_TIMER);
	timer_start(VECTOR_GROUP, SECONDS_TIMER);
	m_motion = true;	
}

void axis::scalar_move(	int steps_to_move,
						bool dir,
						int final_period_us,
						int accel)
{
	if(m_motion)
		return;

	m_cur_time			= 0;
	m_cur_pulse			= 0;
	m_final_pulse		= steps_to_move;
	m_decel_pulse		= m_final_pulse;
	m_direction			= dir;
	m_pulse_period_sec	= (double)final_period_us / 1000000.0;
	m_accel_time		= 1.0 / m_pulse_period_sec / (double)m_accel;
	m_divider_max		= m_divider_min * m_init_period_us / final_period_us;

	std::cout << "\t*****Scalar Move*****\n";
	std::cout << "\tSteps:\t\t\t"			<< steps_to_move << '\n';
	std::cout << "\tPulse Period (us):\t"	<< final_period_us << "\n";
	std::cout << "\tPulse Period (s):\t"	<< m_pulse_period_sec << "\n";
	std::cout << "\tAccel Time (s):\t\t"	<< m_accel_time << '\n';
	std::cout << "\tInitial Divider:\t"		<< m_divider_max << '\n';

	gpio_set_level(DIR_PIN, m_direction);
	ets_delay_us(8);

	reset_timers();
	timer_enable_intr(SCALAR_GROUP, PULSE_TIMER);
	timer_set_divider(SCALAR_GROUP, PULSE_TIMER, m_divider_max);
	timer_set_alarm_value(SCALAR_GROUP, PULSE_TIMER, final_period_us);

	timer_start(SCALAR_GROUP, PULSE_TIMER);
	timer_start(SCALAR_GROUP, SECONDS_TIMER);
	m_motion = true;
}

void axis::jog_move(bool dir)
{
	if(!m_jog_mode)
		return;

	if(m_jog_mode && m_motion && dir == m_direction)
	{
		m_decel_pulse += m_jog_steps;
		m_final_pulse += m_jog_steps;
		std::cout << "Added Jog Steps\n";
		return;
	}

	scalar_move(m_jog_steps, dir, m_jog_period_us);
}

void axis::linear_interpolation_2D()
{
	if(m_cur_pos == m_end_pos)
	{
		timer_pause(VECTOR_GROUP, PULSE_TIMER);
		timer_pause(VECTOR_GROUP, SECONDS_TIMER);
		m_motion = false;
		return;
	}

	int mask = 0;
	int y = (m_end_pos.x)? m_slope*m_cur_pos.x : m_end_pos.y;

	mask += (m_cur_pos.y > y)?	 	4 : 0;
	mask += (m_end_pos.x > 0)?		2 : 0;
	mask += (m_end_pos.y > 0)?		1 : 0;

	switch(mask)
	{
		case 0:		m_cur_pos.x--;	m_step_axis = x_axis;	m_direction = 0;	break;
		case 1:		m_cur_pos.y++;	m_step_axis = y_axis;	m_direction = 1;	break;
		case 2:		m_cur_pos.x++;	m_step_axis = x_axis;	m_direction = 1;	break;
		case 3:		m_cur_pos.y++;	m_step_axis = y_axis;	m_direction = 1;	break;
		case 4:		m_cur_pos.y--;	m_step_axis = y_axis;	m_direction = 0;	break;
		case 5:		m_cur_pos.x--;	m_step_axis = x_axis;	m_direction = 0;	break;
		case 6:		m_cur_pos.y--;	m_step_axis = y_axis;	m_direction = 0;	break;
		case 7:		m_cur_pos.x++;	m_step_axis = x_axis;	m_direction = 1;	break;
	}

	gpio_set_level(DIR_PIN, m_direction);
	ets_delay_us(8);
}

void axis::circular_interpolation_2D()
{
	if(m_cur_pos == m_end_pos && m_cur_pulse > 0)
	{
		timer_pause(VECTOR_GROUP, PULSE_TIMER);
		timer_pause(VECTOR_GROUP, SECONDS_TIMER);
		m_motion = false;
		return;
	}

	int fxy = (m_cur_pos.x*m_cur_pos.x) + (m_cur_pos.y*m_cur_pos.y) - (m_radius*m_radius);
	int dx = 2*m_cur_pos.x;
	int dy = 2*m_cur_pos.y;
	
	bool f = (fxy < 0)? 0 : 1;
	bool a = (dx < 0)? 0 : 1;
	bool b = (dy < 0)? 0 : 1;
	
	int mask = 0;
	if(f) mask += 8;
	if(a) mask += 4;
	if(b) mask += 2;
	if(m_CW) mask += 1;
	
	switch(mask) {
		case 0:		m_cur_pos.y--;	m_step_axis = y_axis;	m_direction = 0;	break;
		case 1:		m_cur_pos.x--;	m_step_axis = x_axis;	m_direction = 0;	break;
		case 2:		m_cur_pos.x--;	m_step_axis = x_axis;	m_direction = 0;	break;
		case 3:		m_cur_pos.y++;	m_step_axis = y_axis;	m_direction = 1;	break;
		case 4:		m_cur_pos.x++;	m_step_axis = x_axis;	m_direction = 1;	break;
		case 5:		m_cur_pos.y--;	m_step_axis = y_axis;	m_direction = 0;	break;
		case 6:		m_cur_pos.y++;	m_step_axis = y_axis;	m_direction = 1;	break;
		case 7:		m_cur_pos.x++;	m_step_axis = x_axis;	m_direction = 1;	break;
		case 8:		m_cur_pos.x++;	m_step_axis = x_axis;	m_direction = 1;	break;
		case 9:		m_cur_pos.y++;	m_step_axis = y_axis;	m_direction = 1;	break;
		case 10:	m_cur_pos.y--;	m_step_axis = y_axis;	m_direction = 0;	break;
		case 11:	m_cur_pos.x++;	m_step_axis = x_axis;	m_direction = 1;	break;
		case 12:	m_cur_pos.y++;	m_step_axis = y_axis;	m_direction = 1;	break;
		case 13:	m_cur_pos.x--;	m_step_axis = x_axis;	m_direction = 0;	break;
		case 14:	m_cur_pos.x--;	m_step_axis = x_axis;	m_direction = 0;	break;
		case 15:	m_cur_pos.y--;	m_step_axis = y_axis;	m_direction = 0;	break;
	}

	gpio_set_level(DIR_PIN, m_direction);
	ets_delay_us(8);
}

void axis::update_divider(timer_group_t TIMER_GROUP)
{

	timer_get_counter_time_sec(TIMER_GROUP, SECONDS_TIMER, &m_cur_time);

	if(m_cur_time > m_accel_time && m_cur_pulse > m_decel_pulse)
		return;

	if(m_cur_pulse > m_decel_pulse)
	{
		m_divider = m_divider_min * m_accel_time / (m_accel_time - m_cur_time);
		m_divider = std::min(m_divider, m_divider_max);

		timer_set_divider(TIMER_GROUP, PULSE_TIMER, m_divider);
		return;
	}

	if(m_cur_time < m_accel_time)
	{
		m_divider = m_divider_min * m_accel_time / m_cur_time;
		m_divider = std::min(m_divider, m_divider_max);
		timer_set_divider(TIMER_GROUP, PULSE_TIMER, m_divider);
		return;
	}

	if(std::fabs(m_cur_time - m_accel_time) < m_pulse_period_sec)
	{
		m_decel_pulse = m_final_pulse - m_cur_pulse;
		timer_set_divider(TIMER_GROUP, PULSE_TIMER, m_divider_min);
		return;
	}

	if(m_cur_pulse == m_decel_pulse)
	{
		timer_set_counter_value(TIMER_GROUP, SECONDS_TIMER, 0);
	}

}

void axis::vector_move_isr(void* arg)
{
	timer_spinlock_take(VECTOR_GROUP);

	if(m_axis == m_step_axis || m_jog_mode)
	{
		gpio_set_level(PULSE_PIN, 1);
		m_step_position += (m_direction)? 1 : -1;
		ets_delay_us(8);

		if(++m_cur_pulse == m_final_pulse)
		{
			timer_pause(VECTOR_GROUP, PULSE_TIMER);
			timer_pause(VECTOR_GROUP, SECONDS_TIMER);
			m_motion = false;
		}
	}

	if(m_line_mode)
		linear_interpolation_2D();
	else if(m_curv_mode)
		circular_interpolation_2D();

	update_divider(VECTOR_GROUP);
	timer_group_clr_intr_status_in_isr	(VECTOR_GROUP, PULSE_TIMER);
	timer_group_enable_alarm_in_isr		(VECTOR_GROUP, PULSE_TIMER);
	timer_spinlock_give					(VECTOR_GROUP);
	gpio_set_level(PULSE_PIN, 0);
}

void axis::scalar_move_isr(void* arg)
{
	timer_spinlock_take(SCALAR_GROUP);

	gpio_set_level(PULSE_PIN, 1);
	ets_delay_us(8);
	gpio_set_level(PULSE_PIN, 0);

	m_step_position += (m_direction)? 1 : -1;

	if(++m_cur_pulse == m_final_pulse)
	{
		timer_pause(SCALAR_GROUP, PULSE_TIMER);
		timer_pause(SCALAR_GROUP, SECONDS_TIMER);
		m_motion = false;
	}

	update_divider(SCALAR_GROUP);
	timer_group_clr_intr_status_in_isr	(SCALAR_GROUP, PULSE_TIMER);
	timer_group_enable_alarm_in_isr		(SCALAR_GROUP, PULSE_TIMER);
	timer_spinlock_give					(SCALAR_GROUP);
}

void axis::syncSem_release_isr(void* arg)
{
	xSemaphoreGiveFromISR(m_syncSem, NULL);
}