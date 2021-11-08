#include "ConfigureUtility.h"

ConfigureUtility::ConfigureUtility() {
	std::fstream file;
	file.open(".config");
	std::string line;
	file_contents.clear();
	
	if(file.good()) {
		file.seekg(0);
		while(!file.eof()){
			std::getline(file, line);
			file_contents.push_back(line);
		}
	}
	file.close();
}

void ConfigureUtility::print() {
	if(file_contents.size() > 0) {
		for(auto it = file_contents.begin(); it != file_contents.end(); it++)
			std::cout << *it << '\n';
	}
	else
		std::cout << "File does not exist or is empty\n";
}

void ConfigureUtility::write_out() {
	std::ofstream file;
	file.open(".config", std::ofstream::out | std::ofstream::trunc);
	for(auto it = file_contents.begin(); it != file_contents.end(); it++)
		file << *it << '\n';
}

std::string ConfigureUtility::get_first_word(const std::string &line) {
	auto pos = line.find(':');
	if(pos != std::string::npos)
		return line.substr(0, pos);
	pos = line.find('{');
	if(pos != std::string::npos)
		return line.substr(0, pos);
	return " ";	
}

const std::string ConfigureUtility::get_param_value(const std::string &line) {
	auto pos = line.find(':');
	if(pos == std::string::npos)
		return " ";
		
	while(line[++pos] == ' ' || line[pos] == '\t');
		
	return line.substr(pos, line.size());
}

void ConfigureUtility::get_axis_params(SPI::AXIS a, motor::params_t &p, int start_line) {
	if(file_contents.size() < 2)
	{
		p.a = a;
		return;
	}
	
	int cur_line = start_line;
	auto line = file_contents.begin() + start_line;
	char axis_name;
	switch(a) {
		case SPI::X_AXIS:
			axis_name = 'x';
			p.a = SPI::X_AXIS;
			break;
		case SPI::Y_AXIS:
			axis_name = 'y';
			p.a = SPI::Y_AXIS;
			break;
		case SPI::Z_AXIS:
			axis_name = 'z';
			p.a = SPI::Z_AXIS;
			break;
		default:
			axis_name = ' ';
	}
	
	while(get_first_word(*line++) != "axis_params") {
		cur_line++;
		if(line == file_contents.end())
			return;
	}
	
	if(get_param_value(*line)[0] == axis_name)
		get_axis_params(p, ++line);
	else 
		get_axis_params(a, p, ++cur_line);
}

void ConfigureUtility::set_axis_params(motor::params_t &p, int start_line) {
	int cur_line = start_line;
	auto line = file_contents.begin() + start_line;
	char axis_name;
	switch(p.a) {
		case SPI::X_AXIS:
			axis_name = 'x';
			break;
		case SPI::Y_AXIS:
			axis_name = 'y';
			break;
		case SPI::Z_AXIS:
			axis_name = 'z';
			break;
		default:
			return;
	}
	
	while(get_first_word(*line++) != "axis_params") {
		cur_line++;
		if(line == file_contents.end()) {
			file_contents.push_back("axis_params{");
			file_contents.push_back("axis: ");
			file_contents[file_contents.size()-1].push_back(axis_name);
			file_contents.push_back("pin: " + std::to_string(p.pin_num));
			file_contents.push_back("spmm: " + std::to_string(p.spmm));
			file_contents.push_back("max: " + std::to_string((double)p.max_steps/p.spmm));
			file_contents.push_back("back: " + std::to_string((double)p.backlash/p.spmm));
			file_contents.push_back("}");
			write_out();
			return;
		}
	}	
	if(get_param_value(*line)[0] == axis_name) {
		*(++line) = "pin: " + std::to_string(p.pin_num);
		*(++line) = "spmm: " + std::to_string(p.spmm);
		*(++line) = "max: " + std::to_string((double)p.max_steps/p.spmm);
		*(++line) = "back: " + std::to_string((double)p.backlash/p.spmm);
		write_out();
	}
	else
		set_axis_params(p, ++cur_line);
}

void ConfigureUtility::get_axis_params(motor::params_t &p, std::vector<std::string>::iterator line) {
	p.pin_num = std::stoi(get_param_value(*line++));
	p.spmm = std::stoi(get_param_value(*line++));
	p.max_steps = p.spmm*std::stod(get_param_value(*line++));
	p.backlash = p.spmm*std::stod(get_param_value(*line++));
}
