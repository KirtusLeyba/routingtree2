#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include "options.hpp"

class BGPNode {
	public:
		std::string asn;
		std::unordered_map<std::string, std::string> neighbors;
		std::string community_label;
		bool is_border;

		BGPNode(std::string _asn, std::string _community_label);
};

std::unordered_map<std::string,BGPNode*> read_input_file(Options opt);

std::unordered_map<std::string, int> count_paths(std::unordered_map<std::string, BGPNode*> &G,
												std::string source);

void count_paths_bgp(std::unordered_map<std::string, BGPNode*> &G,
												std::unordered_map<std::string, double> &path_counts,
												std::unordered_map<std::string, double> &path_cc_counts,
												std::string source);

void traverse_paths(std::string node,
	std::unordered_map<std::string, std::vector<std::string>> &parents,
	std::unordered_map<std::string, double> &path_counts,
	std::unordered_map<std::string, double> &path_cc_counts,
	std::unordered_map<std::string, BGPNode*> &G);