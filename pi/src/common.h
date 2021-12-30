#ifndef COMMON_H
#define COMMON_H

#include <QWidget>
#include <QAction>
#include <QTimer>
#include <QTimerEvent>
#include <QEvent>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTextEdit>

#include <vector>
#include <cmath>
#include <iostream>

#include "program/codeBlock.h"

namespace CNC
{

	enum MODE
	{
		JOG,
		HOME,
		AUTO,
		EDIT,
		MDI,
		NOP
	};

	enum AXIS
	{
		X,
		Y,
		Z,
	};

	struct position_t
	{
		double x;
		double y;
		double z;
	};

}//CNC namespace

#endif