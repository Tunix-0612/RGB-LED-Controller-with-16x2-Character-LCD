#ifndef TUNIXERRORMANAGER_H
#define TUNIXERRORMANAGER_H

#include <Arduino.h>
#include "Constants.h"

class TunixErrorManager 
{
	private :
	
	public:
		TunixErrorManager();
		void errorHandler(ErrorCode code);
};

extern TunixErrorManager errorManager;

#endif