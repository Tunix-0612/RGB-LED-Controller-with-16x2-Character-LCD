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
		ErrorCode selfTest();
};

extern TunixSelfTest deviceTest;

#endif