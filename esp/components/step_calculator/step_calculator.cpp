#include "step_calculator.h"

namespace calculator {
	void get_linear_steps_with_accel(	std::vector<bool> &step, 
										int total_steps, 
										int final_wait_time_us) 
	{
		if(total_steps <= 0)
			return;
			
		//Get final wait time in seconds
		double t_f = (double)final_wait_time_us/1000000;
		std::cout << "Final Wait Time: " << final_wait_time_us << '\n';
		//Calculate the number of timer pulses it should take to reach final speed 
		int accel_pulses = (1-(1/initial_wait_time_factor))/accel/t_f/t_f;
		std::cout << "Timer Callbacks During Acceleration: " << accel_pulses << '\n';
		
		int cur_step = 0;
		int no_steps = initial_wait_time_factor - 1;
		int cur_pulse_num = 0;
		
		step.clear();
		
		while(cur_step < accel_pulses && cur_pulse_num <= total_steps) {
			for(int i = 0; i < accel_pulses/initial_wait_time_factor/(no_steps+1); i++) {
				step.push_back(1);
				step.insert(step.end(), no_steps, 0);
				cur_step += no_steps + 1;
				cur_pulse_num++;
			}
			no_steps--;
		}
		std::cout << "Step Vector Size after accel steps added: " << step.size() << '\n';
		int add_steps = total_steps - cur_pulse_num;
		std::cout << "Additional Steps Needed: " << add_steps << '\n';		
		if(add_steps <= 0)
			return;
		step.insert(step.end(), add_steps, 1);
	}//get_linear_steps_with_accel
	
}//calculator namespace
