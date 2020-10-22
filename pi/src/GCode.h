#ifndef GCODE_H
#define GCODE_H

#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>

#include "Line.h"
#include "Curve.h"
#include "Types.h"

namespace gcode {
	static double prev_x = 0;
	static double prev_y = 0;
	static double feed_rate = 0;
	
	static int x_spmm = 100;
	static int y_spmm = 200;

	void translate_gcode_to_params(const char* filename, std::vector<params_t> &params);

	void gcode_line_to_params(std::string &g_line, params_t &params);
	
	void get_program(std::vector<params_t> &params, std::vector<motor::move_t> &moves);
	void get_program(const char* filename, std::vector<motor::move_t> &moves);
	
	double get_double(std::string::iterator &it);
	
	const std::string mtts(MOVE_TYPE type);
	
	inline void set_x_spmm(int s) {x_spmm = s;}
	inline void set_y_spmm(int s) {y_spmm = s;}

}//gcode namespace

#endif
