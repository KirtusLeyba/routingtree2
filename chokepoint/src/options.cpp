#include "options.hpp"

Options parse_args(int argc, char** argv) {
	Options o;

	//set defaults
	o.input_file = "NONE";

	for(int i = 1; i < argc; i++) {
		std::string argument = argv[i];
		std::string indicator = "--";
		if(argument.find(indicator) != std::string::npos) {
			if(argument == "--input_file") {
				o.input_file = argv[i+1];
			}
		}
	}
	return o;
}

int check_args(Options opt) {
	int result = 1;
	if(opt.input_file == "NONE") {
		printf("Please provide an input_file (--input_file)\n");
		result = 0;
	}
	return result;
}