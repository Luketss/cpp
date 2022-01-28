#include <iostream>
#include <algorithm>
#include <iterator>
#include <format>
#include "Logger.h"

void InitLog()
{
	Log("Initialize log");
}

void Log(const char* message)
{
	std::cout << message << std::endl;
}

int text() {
    std::cout << std::format("Hello {}!\n", "world");
}