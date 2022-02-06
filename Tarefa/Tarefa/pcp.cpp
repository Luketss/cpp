#include <iostream>
#include <cctype>
#include <random>
#include <iomanip>
#include <sstream>
#include <string>
#include <list>
#include <iomanip> 
#include <Windows.h>
#include <cstdlib>
#include <ctime>

#include "common.h"


using namespace std;





string GeneratePCPMessage()
{
	const int spTipo = 5;
	int spNSEQ = 0;
	string spDate = CreateTimestamp();
	string type = RandomString(8);
	//string arrText[2] = {type, spDate};
	//return ShapeStringResult(arrText[2]);
	return type;
}