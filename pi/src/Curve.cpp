#include "Curve.h"

namespace curve {

void get_curve(params_t &p, ops_t &ops, int backlash) {
	if(p.x_spmm != p.y_spmm) {
		std::cout << "Different steps per mm of axes unsupported\n";
		return;
	}
	
	double sq = sqrt(p.i*p.i + p.j*p.j);
	int r = round(sq * (double)p.x_spmm);
	
	int x1 = p.i * p.x_spmm;
	int y1 = p.j * p.y_spmm;
	
	int x2 = x1;
	int y2 = y1;
	
	int x3 = (p.x_f - (p.x_i + p.i))*p.x_spmm;
	int y3 = (p.y_f - (p.y_i + p.j))*p.y_spmm;
	
	int fxy;
	
	bool f;
	bool a;
	bool b;
	bool d = p.dir;
	
	int mask = 0;
	
	int xo;
	int yo;
	
	ops.x_steps.clear();
	ops.y_steps.clear();
	
	do{
		fxy = x2*x2 + y2*y2 - r*r;
		
		f = (fxy < 0)? 0 : 1;
		a = (x2 < 0)? 0 : 1;
		b = (y2 < 0)? 0 : 1;
		
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
		
		if(x2 == 0 && ops.x_steps.size() != 0) {
			ops.y_steps.insert(ops.y_steps.end(), backlash, (y2 < 0)? 1 : -1);
			ops.x_steps.insert(ops.x_steps.end(), backlash, 0);
		}
		else if(y2 == 0 && ops.x_steps.size() != 0) {
			ops.x_steps.insert(ops.x_steps.end(), backlash, (x2 < 0)? 1 : -1);
			ops.y_steps.insert(ops.y_steps.end(), backlash, 0);
		}			
		
		ops.x_steps.push_back(xo);
		ops.y_steps.push_back(yo);
		
		x2 += xo;
		y2 += yo;
				
	}while(x2 != x3 || y2 != y3);
	
	ops.wait_time = 1/p.feed_rate/p.x_spmm;
	
}//get_curve

void get_curve(params_t &p, esp_params_t &ep) {
	ep.r = round(sqrt(p.i*p.i + p.j*p.j) * p.x_spmm);
	std::cout << "R: " << ep.r << '\n';
	
	ep.x_i = round(-p.i*p.x_spmm);
	std::cout << "Xi: " << ep.x_i << '\n';
	
	ep.y_i = round(-p.j*p.y_spmm);
	std::cout << "Yi: " << ep.y_i << '\n';
	
	ep.x_f = round((p.x_f - (p.x_i + p.i))*p.x_spmm);
	std::cout << "Xf: " << ep.x_f << '\n';
	
	ep.y_f = round((p.y_f - (p.y_i + p.j))*p.y_spmm);
	std::cout << "Yf: " << ep.y_f << '\n';
	
	ep.feed_rate = p.feed_rate;
	std::cout << "feed_rate: " << ep.feed_rate << '\n';
	
	ep.dir = p.dir;
}//get_curve

}//curve namespace
