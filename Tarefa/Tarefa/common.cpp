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

using namespace std;


string CreateTimestamp()
{
	auto current_time = std::time(nullptr);
	tm time_info{};
	const auto local_time_error = localtime_s(&time_info, &current_time);
	if (local_time_error != 0)
	{
		throw std::runtime_error("localtime_s() failed: " + std::to_string(local_time_error));
	}
	std::ostringstream output_stream;
	output_stream << std::put_time(&time_info, "%H:%M:%S");
	string timestamp(output_stream.str());
	return timestamp;
}

string RandomString(size_t length)
{
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}