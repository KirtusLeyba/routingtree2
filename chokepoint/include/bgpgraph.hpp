#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>

//convenience struct
//for holding command line args
struct Options {
	std::string input_file;
	std::string output_file;
	std::string border_mode;
	int store_ncp;
};

Options parse_args(int argc,char** argv);

class BGPNode {
	public:
		std::string asn;
		std::unordered_map<std::string, std::string> neighbors;
		std::string community_label;
		bool is_border;

		BGPNode(std::string _asn, std::string _community_label);
};

std::unordered_map<std::string,BGPNode*> read_input_file(Options opt);