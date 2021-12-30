#ifndef CODEBLOCK_H
#define CODEBLOCK_H

#include "common.h"

namespace CNC
{

class codeBlock : public QObject
{
	Q_OBJECT

public:

	codeBlock(QObject* parent = nullptr);
	~codeBlock() {}

	void clear();

public:

	bool addArg(char c, double d);

public:

	const std::map<char, double>& args() const {return m_args;}

public:


public:

	char	letterCode; //G,M,etc.
	int		numberCode;

	char	m_letterCode; //G,M,etc.
	int		m_numberCode;

	std::map<char, double>	m_args;

	double x, y, z; //Absolute positions
	double u, v, w; //Incremental positions
	double i, j, k; //Relative center location for circular move
	double r;		//Radius
	double p;		//Power
	double f;		//Feed rate

	double prev_x, prev_y, prev_z;	//Previous absolute positions

	bool abs_x = false;
	bool abs_y = false;
	bool abs_z = false;

};

	
	std::ostream& operator<<(std::ostream& out, const codeBlock* block);

}//CNC namespace

#endif