#ifndef TUNIXERRORMANAGER_H
#define TUNIXERRORMANAGER_H

#include <Arduino.h>
#include "Constants.h"

class TunixErrorManager 
{
	private :
	
	public:
		TunixErrorManager();
		void errorHandler(int errorCode);
};

extern TunixErrorManager errorManager;

#endif