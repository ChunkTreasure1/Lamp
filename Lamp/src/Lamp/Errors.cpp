#include "Errors.h"

#include <cstdlib>
#include <iostream>

void Lamp::FatalError(std::string errorString)
{
	std::cout << errorString << std::endl;
	std::cout << "Enter any key to quit...";

	int tmp;
	std::cin >> tmp;

	exit(-1);
}