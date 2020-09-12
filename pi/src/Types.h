#ifndef TYPES_H
#define TYPES_H

#include <vector>

namespace SPI {

enum FUNCTION_CODE {
	SET_FEED_RATE, 			
	SET_DIRECTION,
	SET_STEP_TIME,
	SET_STEPS_TO_MOVE,
	SET_JOG_STEPS,
	SET_BACKLASH,
	SET_X_AXIS,
	SET_STEPS_PER_MM,
	SET_MAX_STEPS,
	SETUP_CURVE,
	ENA_JOG_MODE,		
	DIS_JOG_MODE,		
	ENA_LINE_MODE,
	DIS_LINE_MODE,
	ENA_CURV_MODE,
	DIS_CURV_MODE,
	ENA_SYNC_MODE,
	DIS_SYNC_MODE,
	FIND_ZERO,
	MOVE,
	STOP,
	RECEIVE,
};

enum AXIS {
	X_AXIS,
	Y_AXIS,
	Z_AXIS
};

}//SPI namespace

namespace curve {

typedef struct {
	double x_i;		//Initial x coordinate in mm
	double y_i;		//Initial y coordinate in mm
	double x_f;		//Final x coordinate in mm
	double y_f;		//Final y coordinate in mm
	double i;		//x distance of circle center from initial coordinates in mm
	double j;		//y distance of circle center from initial coordinates in mm
	double feed_rate;	//Feed rate in mm/s
	
	bool dir;		//1 for clockwise, 0 for counterclockwise
	
	int x_spmm;		//Steps per mm of x-axis
	int y_spmm;		//Steps per mm of y-axis
}params_t;

typedef struct {
	int x_i;
	int x_f;
	int y_i;
	int y_f;
	int r;
	int feed_rate;
	int x_axis;
	bool dir;
}esp_params_t;

typedef struct {
	int 				wait_time;	//Wait time between steps in microseconds
	
	std::vector<int>	x_steps;
	std::vector<int>	y_steps;
}ops_t;

}//curve namespace

namespace line {
	
typedef struct {
	double x_i;		//Initial x coordinate in mm
	double y_i;		//Initial y coordinate in mm
	double x_f;		//Final x coordinate in mm
	double y_f;		//Final y coordinate in mm
	double speed;	//Surface speed in mm/s
	
	int x_spmm;		//Steps per mm of x-axis
	int y_spmm;		//Steps per mm of y-axis
}params_t;

typedef struct {
	int x_steps;	//Number of x steps in the line operation
	int x_time;		//Time between x steps in microseconds
	int x_dir;		//Direction of x-axis
	
	int y_steps;	//Number of y steps in the line operation
	int y_time;		//Time between y steps in microseconds
	int y_dir;		//Direction of y-axis
}ops_t;

}//line namespace

namespace motor {
	
typedef struct {
	int pin_num;	//GPIO number of the device select pin
	int spmm;		//Steps per mm of the motor
	int max_steps;	//Max travel of the axis in steps
	int backlash;	//Amount of backlash in steps

	SPI::AXIS a;	//Axis identifier
}params_t;

typedef struct {
	bool line_op;	//True for line operation, false for curve operation
	
	line::ops_t l;	//Line operation
	
	curve::esp_params_t c;	//Curve operation
}move_t;
	
}//motor namespace

namespace gcode {
	
enum MOVE_TYPE {
	RAPID_POSITION,
	LINEAR_INTERPOLATION,
	CIRCULAR_INTERPOLATION_CW,
	CIRCULAR_INTERPOLATION_CCW
};

typedef struct {
	MOVE_TYPE type;
	
	double x_i;
	double y_i;
	double x_f;
	double y_f;
	double i;
	double j;
	double feed_rate;
} params_t;

}//gcode namespace
	
#endif
