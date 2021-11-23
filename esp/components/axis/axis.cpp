#include "axis.h"

/*	Initialize static variables	*/
bool			axis::motor_direction	= false;
bool			axis::motor_in_motion	= false;
int				axis::position_steps	= 0;
int				axis::jog_steps			= 0;
xQueueHandle	axis::syncSem			= xSemaphoreCreateBinary();

std::vector<bool> axis::step_vec = {0};
std::vector<bool> axis::dirs_vec = {0};

axis::axis() {
	set_defaults();
	step_timer = std::make_shared<Timer>(&position_steps, &motor_in_motion);
}
		
void axis::check_error(const char* msg) {
	if(error != ESP_OK) {
		std::cout << msg << '\n' << esp_err_to_name(error);
	}
}

void axis::setup_gpio() {
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
	if(time <= 0)
		return;
	pulse_period_us = time;
	std::cout << "Pulse Period Set: " << pulse_period_us << "us\n";
	set_feed_rate(1000000/(pulse_period_us*steps_per_mm));
}

void axis::set_direction(bool dir) {
	if(motor_in_motion)
		return;
		
	motor_direction = dir;
	std::cout << "Direction set to " << (int)motor_direction << '\n';
	//gpio_set_level(DIR, dir);
}

void axis::set_steps_to_move(int steps) {		
	steps_to_move = steps;
	std::cout << "Will move " << steps_to_move << " steps on next move\n";
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

void axis::linear_move(bool sync, bool dir, int steps, int time_us)
{
	enable_line_mode(true);
	enable_sync_mode(sync);
	set_direction(dir);
	set_steps_to_move(steps);
	set_step_time(time_us);
	move();
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

	step_timer->linear_move_config(	5000, 
									jog_wait_time,
									40000,
									jog_steps,
									motor_direction);
	step_timer->start_linear();
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

	step_timer->linear_move_config(	5000,
									pulse_period_us,
									40000,
									steps_to_move,
									motor_direction);

	if(sync_mode) {
		spi->toggle_ready();
		xSemaphoreTake(syncSem, portMAX_DELAY);
	}
	
	step_timer->start_linear();
}

void axis::move_curv_mode() {
	if(motor_in_motion)
		return;
	
	step_timer->vector_move_config(	5000,
									pulse_period_us,
									40000,
									std::make_shared<std::vector<bool>>(step_vec),
									std::make_shared<std::vector<bool>>(dirs_vec));

	if(sync_mode) {
		spi->toggle_ready();
		xSemaphoreTake(syncSem, portMAX_DELAY);
	}

	step_timer->start_vector();
}

void axis::stop() {	
	step_timer->reset();
	motor_in_motion = false;
}

void axis::stop_zero_interlock() {

}

void axis::find_zero() {
	set_direction(0);

}

void axis::setup_curve(std::vector<int> &info) {	
	set_feed_rate(info[7]);

	int r 		= info[2];	
	int x2		= info[3];
	int y2 		= info[4];	
	int x3 		= info[5];
	int y3 		= info[6];	
	int xo 		= 0;
	int yo 		= 0;		
	int mask	= 0;
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
		fxy = (x2*x2) + (y2*y2) - (r*r);
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
		
		if(x_axis) {
			step_vec.push_back(xo);
			dirs_vec.push_back((xo == 0)? dirs_vec[step_vec.size() - 1] : (bool)(xo+1));
		}
		else {
			step_vec.push_back(yo);
			dirs_vec.push_back((yo == 0)? dirs_vec[step_vec.size() - 1] : (bool)(yo+1));
		}
				
		x2 += xo;
		y2 += yo;

	}while(abs(x2 - x3) > 1 || abs(y2 - y3) > 1 || step_vec.size() < 3);
	
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
	
	//Add one more step so the timer isr doesn't try to read past the last element
	dirs_vec.push_back(*dirs_vec.end());

	//spi->toggle_ready();
	gpio_set_level(spi->ready_pin(), 1);
	gpio_set_level(spi->ready_pin(), 0);
}

void axis::test_function(std::vector<int> args) {
	
}
