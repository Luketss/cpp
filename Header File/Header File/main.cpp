#include <iostream>
#include "Logger.h"

void cpointer() 
{
	// first pointer
	void* ptr = nullptr;
}

void findAMemorySpace()
{
	int var = 8;
	void* ptr = &var; //this & before a created variable ask for the variable address
}

void getMemoryValue()
{
	//this will return an error, void pointer can not get the memory size
	//int var = 8;
	//void* ptr = &var;
	//*ptr = 10;
	int var = 8;
	int* ptr = &var;
	*ptr = 10;
}

void createAReference()
{
	//note the difference between pointer (int* a = &b) and reference (int& ref = a)
	// ref is not a variable, it is a reference, an alias, ref is a
	int a = 5;
	int& ref = a;
}

//essa pode ser usada no trabalho quando pega a ordem 000 e incrementa
void Increment(int& value)
{
	value++;
}

void doublePointer()
{
	char* buffer = new char[8];
	memset(buffer, 0, 8);

	char** ptr = &buffer;

	delete[] buffer;
}


int main() 
{
	int a = 5;
	Increment(a);
	InitLog();
	Log("my message");
	std::cin.get();
}