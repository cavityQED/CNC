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

#define READY_PIN 5

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
	GET_POSITION, 		
	SET_MM_PER_STEP,
	SET_STEPS_PER_REVOLUTION,
	ZERO,
	ZERO_INTERLOCK_STOP,
	MOVE_TO,	
	MOVE,
	STOP,
	RECEIVE
};

struct motorParameters {
	int pin_num;		//WiringPi pin number of axis device select
	int SPR;			//Steps per revolution
	double mm_per_step;	//Millimeters traveled in one step
};	

class MotorController {
public:
	MotorController();
	~MotorController() {sem_unlink("sem_ready");}
	
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
	
	//Functions that execute on all axes
	void enable_jog_mode();
	void disable_jog_mode();
	void enable_step_mode();
	void set_jog_speed_steps(int steps);
	void set_jog_speed_mm(double mm);
	void get_position(double &x_steps, double &y_steps);
	
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
		
private:
	//SPI
	int spi_cs_fd;
	unsigned char sendbuf[8] {0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char recvbuf[8] {0, 0, 0, 0, 0, 0, 0, 0};
	struct spi_ioc_transfer spi;
	
	//Semaphore
	static sem_t* sem;
	
	//Axis Info
	bool x_connected = false;
	bool y_connected = false;
	
	motorParameters x_params;
	motorParameters y_params;
};

#endif
