#ifndef ACTION_H
#define ACTION_H

#include <pigpio.h>

#include <QWidget>

namespace CNC
{

struct codeBlock
{
	char	letterCode; //G,M,etc.
	int		numberCode;

	double x, y, z; //Absolute positions
	double u, v, w; //Incremental positions
	double i, j, k; //Relative center location for circular move
	double r;		//Radius
	double p;		//Power
	double f;		//Feed rate

	std::map<char, double>	vars;
};

class Action : public QWidget
{
	Q_OBJECT

public:
	
	Action(CNC::codeBlock block, QWidget* parent = nullptr);
	~Action() {}

public slots:

	virtual void execute() = 0;

protected:

	CNC::codeBlock		m_block;
};

}//CNC namespace

#endif