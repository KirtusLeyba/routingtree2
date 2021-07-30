#pragma once
#include <stdlib.h>
#include <string>

struct Options {
	std::string input_file;
};

Options parse_args(int argc, char** argv);
int check_args(Options opt);