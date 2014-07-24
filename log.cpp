#include "log.h"
#include <iostream>
void logw(char* str)
{
	std::cout << "WARNING: \t" << str << std::endl;
}
void loge(char* str)
{
	std::cout << "ERROR: \t" << str << std::endl;
}
void logi(char* str)
{
	std::cout << "INFO: \t" << str << std::endl;
}
void logi(int str)
{
	std::cout << "INFO: \t" << str << std::endl;
}
void logw(int str)
{
	std::cout << "WARNING: \t" << str << std::endl;
}