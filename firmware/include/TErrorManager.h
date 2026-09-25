#ifndef TERRORMANAGER_H
#define TERRORMANAGER_H

#include <Arduino.h>

#include "Constants.h"

class TErrorManager 
{
  private :

  public:
    TErrorManager();
    void errorHandler(StatusCode code);
};

extern TErrorManager errorManager;

#endif
