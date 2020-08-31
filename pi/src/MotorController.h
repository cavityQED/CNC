#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <semaphore.h>

#include <wiringPi.h>

#include <iostream>
#include <cstring>

#include <vector>

#include <QWidget>
#include <QAction>
#include <QTimer>
#include <QTimerEvent>

#include "Curve.h"
#include "Line.h"
#include "Types.h"
#include "GCode.h"
#include "SPI.h"

#define READY_PIN 	24
#define SYNC_PIN	18

#define TIMER_PERIOD 25

typedef struct {
	int pin_num;		//GPIO pin number of axis device select
	int SPR;			//Steps per revolution
	double mm_per_step;	//Millimeters traveled in one step
} motorParameters;

typedef struct {
	bool line_move; 	//True for line move, false for curve move
	line::ops_t l;	
	curve::esp_params_t c;
} move_params_t;

class MotorController : public QWidget {
	Q_OBJECT;
public:
	MotorController(QWidget *parent = 0);
	~MotorController() {sem_unlink("sem_ready"); std::cout << "MotorController destroyed\n";}
	
	void setup_spi();
	void setup_gpio();
	void test_lines();
	
	static void release_sem_interrupt() {
	//	std::cout << "Semaphore Unlocked\n";
		sem_post(sem);
	}
	
	void setup_axis(motor::params_t &params);
	
	//Functions to run all the moves of the current program contained in program_move
	void start_program(std::vector<motor::move_t> &moves);
	void start_program();
	void next_move();
	void add_move(motor::move_t &move) {
		program_moves.push_back(move);
	}
	void clear_program() {
		program_moves.clear();
	}
	
	//Send a message to a particular device
	void send(int device_pin);
	void send_curve_data(curve::esp_params_t &data);
	void send_line_data(line::ops_t &data);
	
	//Functions that execute on all axes
	void sync_move();
	
	void enable_jog_mode(bool enable);
	void enable_line_mode(bool enable);
	void enable_curv_mode(bool enable);
	void enable_sync_mode(bool enable);
	
	void set_jog_steps(int steps);
	void set_jog_speed_mm(double mm);
	void set_step_time(int period);
	
	void get_position_spi();
	void get_position(double &x, double &y) {get_position_spi(); x = x_pos; y = y_pos;}
	void get_position_abs(double &x, double &y) {x = x_pos + x_offset; y = y_pos + y_offset;}
			
	//Axis-specific functions
	void move(SPI::AXIS a);	
	void home(SPI::AXIS a);	
	void set_dir(SPI::AXIS a, bool dir);
	void set_feed_rate(SPI::AXIS a, int feed_rate);
	void set_steps_to_move(SPI::AXIS a, int steps);

	void set_steps_per_mm(SPI::AXIS a, int spmm);
	void set_max_steps(SPI::AXIS a, int max_steps);
	void set_backlash(SPI::AXIS a, int b);
	
	//Set the in_motion bool
	void set_motion(bool m) {
		in_motion = m;
	}
	
	//Get pin number for particular axis
	int get_pin(SPI::AXIS a) {
		if(a == SPI::X_AXIS)
			return x_params.pin_num;
		else if(a == SPI::Y_AXIS)
			return y_params.pin_num;
		else
			return -1;
	}
	
	void timerEvent(QTimerEvent *e) {
		double prev_x = x_pos;
		double prev_y = y_pos;
		get_position_spi();
		if(!in_motion) {
			killTimer(e->timerId());
			if(in_program) {
				cur_program_move += 1;
				std::cout << "Next Move....\n";
				next_move();
			}
		}
	}		
	
public slots:
	
	void run_p() {
		test_lines();
		std::cout << "Number of Moves: " << program_moves.size() << '\n';
		start_program();
	}
	
	void zero_x();
	void zero_y();
	
private:
	//SPI
	int spi_cs_fd;
	std::vector<int> sendbuf;
	std::vector<int> recvbuf;
	struct spi_ioc_transfer spi;
	
	//Semaphore
	static sem_t* sem;
	
	//Axis Info
	bool x_connected = false;
	bool y_connected = false;
	bool x_motion = false;
	bool y_motion = false;
	bool in_motion = false;
	bool in_program = false;	//True if running a program, false if jogging or on last move of program
	
	double x_pos = 0;
	double y_pos = 0;
	
	double x_offset = 0;
	double y_offset = 0;
	
	motor::params_t x_params;
	motor::params_t y_params;
		
	//Vector to store all the moves of a program
	std::vector<motor::move_t> program_moves;
	int cur_program_move = 0;	//Index of the current move of the program
};

#endif
