#include "Log.hpp"
#include <iostream>

void Log::Error(const std::string &message)
{
	std::cerr << message << '\n';
}
void Log::Info(const std::string &message)
{
	std::cout << message << '\n';
}