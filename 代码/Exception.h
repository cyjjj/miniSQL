#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

class Exception
{
public:
	string msg;
	Exception(string s) :msg(s) {}
};

#endif  /* !_EXCEPTION_H_ */