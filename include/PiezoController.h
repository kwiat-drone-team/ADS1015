#pragma once
#include "serialib.h"

class PiezoController {
  public:
  	PiezoController(const char* device, unsigned baudrate);
  	void Step(int steps);
  	void MoveTo(int position);
  	int GetPosition();
  	bool Running();
  private:
  	serialib ser;
};
