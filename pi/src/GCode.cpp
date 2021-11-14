#include "GCode.h"

namespace gcode {
	
void translate_gcode_to_params(const char* filename, std::vector<params_t> &params) {
	std::string g_line {""};
	std::ifstream file;
	file.open(filename);
	params_t p;
	
	while(!file.eof()) {
		std::getline(file, g_line);
		g_line.push_back(' ');
		std::cout << "Got line: " << g_line << '\n';
		if(g_line[1] == '1' || g_line[1] == '2' || g_line[1] == '3') {
			gcode_line_to_params(g_line, p);
			params.push_back(p);			
		}
	}
	
	prev_x = 0;
	prev_y = 0;
}//translate_gcode_to_params

void gcode_line_to_params(std::string &g_line, params_t &params) {
	//if(g_line[0] == 'L')

	params.type = (MOVE_TYPE)((int)(g_line[1])%48);
	
	params.x_i = prev_x;
	params.y_i = prev_y;
	params.i = 0;
	params.j = 0;
	
	std::string::iterator it = g_line.begin();
	it++;
	
	std::cout << "Translating line: " << &g_line[0] << '\n';
	
	while(it != g_line.end()) {
		switch(*it) {
			case 'X':	params.x_f = get_double(++it);	break;
			case 'Y':	params.y_f = get_double(++it);	break;
			case 'I':	params.i = get_double(++it);	break;
			case 'J':	params.j = get_double(++it);	break;
			case 'F':	feed_rate = get_double(++it);	break;
			default:	it++;
		}
	}
	
	prev_x = params.x_f;
	prev_y = params.y_f;
	
	params.feed_rate = feed_rate;
	
}//gcode_line_to_params

void get_program(std::vector<params_t> &params, std::vector<motor::move_t> &moves) {
	motor::move_t move;
	
	line::ops_t line_op;
	line::params_t line_params;
	
	curve::params_t curve_op;
	curve::esp_params_t curve_esp_op;
	
	line_params.x_spmm = x_spmm;
	line_params.y_spmm = y_spmm;
	
	curve_op.x_spmm = x_spmm;
	curve_op.y_spmm = y_spmm;
	
	for(params_t p : params) {
		if(p.type == LINEAR_INTERPOLATION) {
			line_params.x_i = p.x_i;
			line_params.x_f = p.x_f;
			line_params.y_i = p.y_i;
			line_params.y_f = p.y_f;
			line_params.speed = p.feed_rate;
			line::get_line(line_params, line_op);
			move.line_op = 1;
			move.l = line_op;
			moves.push_back(move);
		}
		
		else if(p.type == CIRCULAR_INTERPOLATION_CW) {
			curve_op.x_i = p.x_i;
			curve_op.x_f = p.x_f;
			curve_op.y_i = p.y_i;
			curve_op.y_f = p.y_f;
			curve_op.i = p.i;
			curve_op.j = p.j;
			curve_op.feed_rate = p.feed_rate;
			curve_op.dir = 1;
			curve::get_curve(curve_op, curve_esp_op);
			move.line_op = 0;
			move.c = curve_esp_op;
			moves.push_back(move);
		}
		
		else if(p.type == CIRCULAR_INTERPOLATION_CCW) {
			curve_op.x_i = p.x_i;
			curve_op.x_f = p.x_f;
			curve_op.y_i = p.y_i;
			curve_op.y_f = p.y_f;
			curve_op.i = p.i;
			curve_op.j = p.j;
			curve_op.feed_rate = p.feed_rate;
			curve_op.dir = 0;
			curve::get_curve(curve_op, curve_esp_op);
			move.line_op = 0;
			move.c = curve_esp_op;
			moves.push_back(move);
		}
	}
}

void get_program(const char* filename, std::vector<motor::move_t> &moves) {
	std::vector<params_t> p;
	translate_gcode_to_params(filename, p);
	std::cout << "GCode Params Size: " << p.size() << '\n';
	get_program(p, moves);
}

double get_double(std::string::iterator &it) {
	std::string val {""};
	while(*it != ' ') {
		val += *it;
		it++;
	}
	
	std::cout << "Translating " << val << " to double\n";
	return std::stod(val);
}//get_double

const std::string mtts(MOVE_TYPE type) {
	switch(type) {
		case RAPID_POSITION:				return "Rapid Positioning";
		case LINEAR_INTERPOLATION:			return "Linear Move";
		case CIRCULAR_INTERPOLATION_CW:		return "CW Circle Move";
		case CIRCULAR_INTERPOLATION_CCW:	return "CCW Circle Move";
		default:							return "Not a supported move type";
	}
}

}//gcode namespace
