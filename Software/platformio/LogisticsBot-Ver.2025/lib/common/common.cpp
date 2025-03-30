
#include "common.h"

bool timePassed(unsigned long &t, int32_t diff)
{
  unsigned long t1 = millis();
  if (t1 - t >= diff)
  {
    t = t1;
    return true;
  }
  return false;
}