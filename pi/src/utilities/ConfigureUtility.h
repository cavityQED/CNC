#ifndef CONFIGUREUTILITY_H
#define CONFIGUREUTILITY_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "../Types.h"

class ConfigureUtility {
public:
	ConfigureUtility();
	
	void print();
	void write_out();
	
	std::string get_first_word(const std::string &line);
	const std::string get_param_value(const std::string &line);
	void get_axis_params(SPI::AXIS a, motor::params_t &p, int start_line = 0);
	
	void set_axis_params(motor::params_t &p, int start_line = 0);
	
private:
	std::vector<std::string> file_contents;
	
	void get_axis_params(motor::params_t &p, std::vector<std::string>::iterator line);
};

#endif
