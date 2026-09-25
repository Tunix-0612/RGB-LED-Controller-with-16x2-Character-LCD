#ifndef TSELFTEST_H
#define TSELFTEST_H

#include <Arduino.h>

#include "Constants.h"

class TSelfTest
{
	private:
		bool runRAMTest();
		
	public:
		TSelfTest();
	
		int  getFreeRAM();
		StatusCode selfTest();
};

extern TSelfTest deviceTest;

#endif
