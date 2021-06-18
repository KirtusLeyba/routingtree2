#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <omp.h>
#include "bgpgraph.hpp"


int main(int argc, char** argv) {

	Options opt = parse_args(argc, argv);

	printf("### BGPSAS ###\n");
	printf("# input_file = %s\n", opt.input_file.c_str());
	printf("# output_file = %s\n", opt.output_file.c_str());
	printf("# border_mode = %s\n", opt.border_mode.c_str());
	printf("# store_ncp = %d\n", opt.store_ncp);
	if(opt.store_ncp) {
		printf("# storing ncp at: ncp_%s\n", opt.output_file.c_str());
	}

	//load network from data
	printf("Loading network...\n");
	double t0 = omp_get_wtime();
	std::unordered_map<std::string,BGPNode*> full_network = read_input_file(opt);
	std::vector<std::string> node_list;

	//for tabulating results
	std::unordered_map<std::string, double> ito_paths;
	std::unordered_map<std::string, double> ito_community_paths;
	std::unordered_map<std::string, double> oti_paths;
	std::unordered_map<std::string, double> oti_community_paths;


	for(auto it : full_network) {
		auto it_v = std::find(node_list.begin(), node_list.end(), it.first);
		if(it_v == node_list.end()){
			node_list.push_back(it.first);
			ito_paths.insert(std::make_pair( it.first, 0.0 ));
			oti_paths.insert(std::make_pair( it.first, 0.0 ));
			ito_community_paths.insert(std::make_pair( it.second->community_label, 0.0 ));
			oti_community_paths.insert(std::make_pair( it.second->community_label, 0.0 ));
		}
		for(auto it2 : it.second->neighbors) {

			it_v = std::find(node_list.begin(), node_list.end(), it2.first);
			if(it_v == node_list.end()){
				node_list.push_back(it2.first);
				ito_paths.insert(std::make_pair( it2.first, 0.0 ));
				oti_paths.insert(std::make_pair( it2.first, 0.0 ));
				ito_community_paths.insert(std::make_pair( full_network[it2.first]->community_label, 0.0 ));
				oti_community_paths.insert(std::make_pair( full_network[it2.first]->community_label, 0.0 ));
			}

		}
	}
	double t1 = omp_get_wtime();
	printf("Finished Loading network in %f seconds...\n", t1-t0);


	//clear memory
	printf("Cleaning memory...\n");
	for(auto it : full_network) {
		delete it.second;
	}

	return 0;
}