#ifndef CODEBLOCK_H
#define CODEBLOCK_H

#include "common.h"

namespace CNC
{

class codeBlock : public QObject
{
	Q_OBJECT

public:

	codeBlock(QObject* parent = nullptr) : QObject(parent) {}
	~codeBlock() {}

};

}//CNC namespace

#endif