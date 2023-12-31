#pragma once

#ifndef Parsing_HPP
# define Parsing_HPP

#include <vector>
#include <iostream>

class Parsing
{
public:
	// static std::vector<std::string> parsing(char *buf);
	static std::vector<std::string> parsing(std::string command);
	static int checkCommand(std::string cmd);
};

#endif
