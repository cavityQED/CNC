#ifndef ACTION_H
#define ACTION_H

#include <pigpio.h>

#include <cmath>

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

	double prev_x, prev_y, prev_z;	//Previous absolute positions

	bool abs_x = false;
	bool abs_y = false;
	bool abs_z = false;
};

class Action : public QWidget
{
	Q_OBJECT

public:
	
	Action(CNC::codeBlock block, QWidget* parent = nullptr);
	Action(QWidget* parent = nullptr);
	~Action() {}

	virtual void enable_sync_mode(bool enable)	{return;}
	virtual void wait_for_ready()				{return;}

	void setCodeBlock(CNC::codeBlock block)		{m_block = block;}

	const CNC::codeBlock& getCodeBlock() const {return m_block;}

public slots:

	virtual void execute() = 0;

protected:

	CNC::codeBlock		m_block;
};

}//CNC namespace

#endif