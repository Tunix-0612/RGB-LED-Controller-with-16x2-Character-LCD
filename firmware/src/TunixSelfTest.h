#ifndef TUNIXSELFTEST_H
#define TUNIXSELFTEST_H

#include <Arduino.h>
#include "Constants.h"

class TunixSelfTest
{
	private:
		bool runRAMTest();
		
	public:
		TunixSelfTest();
	
		int  getFreeRAM();
		byte selfTest();
};

extern TunixSelfTest deviceTest;

#endif