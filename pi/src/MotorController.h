#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <semaphore.h>
#include <wiringPi.h>

#include <vector>

#include <QWidget>
#include <QAction>

#include "Curve.h"

#define READY_PIN 	24
#define SYNC_PIN	18

enum SPI_FUNCTION_CODE {
	SET_SPEED, 			
	SET_DIRECTION,
	SET_STEPS_TO_MOVE,
	SET_MM_TO_MOVE,
	SET_JOG_SPEED_STEPS,
	SET_JOG_SPEED_MM,
	ENA_JOG_MODE,		
	DIS_JOG_MODE,		
	ENA_STEP_MODE,
	DIS_STEP_MODE,
	ENA_SYNC_MODE,
	DIS_SYNC_MODE,	
	GET_POSITION, 		
	SET_MM_PER_STEP,
	SET_STEPS_PER_REVOLUTION,
	ZERO,
	ZERO_INTERLOCK_STOP,
	MOVE_TO,	
	MOVE,
	CIRCLE_MOVE,
	STOP,
	RECEIVE,
	SETUP_CIRCLE_MOVE,
	SET_CIRCLE_OPS,
	GET_CIRCLE_STEPS,
	GET_CIRCLE_TIMES,
	GET_CIRCLE_DIRS,
	PRINT_CIRCLE_INFO,
	MOTOR_READY,
	MOTOR_NOT_READY
};

struct motorParameters {
	int pin_num;		//WiringPi pin number of axis device select
	int SPR;			//Steps per revolution
	double mm_per_step;	//Millimeters traveled in one step
};	

class MotorController : public QWidget {
	Q_OBJECT;
public:
	MotorController(QWidget *parent = 0);
	~MotorController() {sem_unlink("sem_ready"); std::cout << "MotorController destroyed\n";}
	
	void setup_spi();
	void setup_gpio();
	
	static void release_sem_interrupt() {
		std::cout << "Semaphore Unlocked\n";
		sem_post(sem);
	}
	
	void setup_x_axis(motorParameters &params);
	void setup_y_axis(motorParameters &params);
	
	//Send a message to a particular device
	void send(int device_pin);
	void send_data(int device_pin, std::vector<int> &data, SPI_FUNCTION_CODE type);
	void send_circle_ops(int device_pin, int ops);
	void print_circle_info(int device_pin) {
		sendbuf[0] = PRINT_CIRCLE_INFO;
		send(device_pin);
	}
	
	//Functions that execute on all axes
	void enable_jog_mode(bool enable);
	void enable_step_mode(bool enable);
	void enable_sync_mode(bool enable);
	void set_jog_speed_steps(int steps);
	void set_jog_speed_mm(double mm);
	void get_position(double &x_steps, double &y_steps);
	
	void sync_move();
	void circle_move();
	
	//Axis-specific functions	
	void x_move();
	void y_move();
	
	void x_zero();
	void y_zero();
	
	void x_set_dir(bool dir);
	void y_set_dir(bool dir);
	
	void x_set_speed(int rpm);
	void y_set_speed(int rpm);
	
	void x_set_steps_to_move(int steps);
	void y_set_steps_to_move(int steps);
	
	double x_get_position();
	double y_get_position();
	
	//Set the in_motion bool
	void set_motion(bool m) {
		in_motion = m;
	}
	
	void copy_circle_ops(curve::motor_ops_t &ops);
	
public slots:
	void calc_circle() {
		curve::calculate_circle(circle_params, circle_ops);
	}
	
	void send_circle() {
		send_circle_ops(x_params.pin_num, circle_ops.num_ops);
		send_circle_ops(y_params.pin_num, circle_ops.num_ops);
		
		send_data(x_params.pin_num, circle_ops.x_steps, GET_CIRCLE_STEPS);
		send_data(x_params.pin_num, circle_ops.x_times, GET_CIRCLE_TIMES);
		send_data(x_params.pin_num, circle_ops.x_dirs, GET_CIRCLE_DIRS);
		
		send_data(y_params.pin_num, circle_ops.y_steps, GET_CIRCLE_STEPS);
		send_data(y_params.pin_num, circle_ops.y_times, GET_CIRCLE_TIMES);
		send_data(y_params.pin_num, circle_ops.y_dirs, GET_CIRCLE_DIRS);
		
		print_circle_info(x_params.pin_num);
		print_circle_info(y_params.pin_num);
	}
		
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
	bool in_motion = false;
	
	motorParameters x_params;
	motorParameters y_params;
	
	curve::motor_ops_t circle_ops;
	curve::circle_params_t circle_params;
};

#endif
