#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <omp.h>
#include "options.hpp"
#include "bgpgraph.hpp"
#include <upcxx/upcxx.hpp>


int main(int argc, char** argv) {

	//startup upc++
	upcxx::init();

	Options opt = parse_args(argc, argv);
	if(!check_args(opt)){
		exit(0);
	}

	//load network from data
	printf("Loading network...\n");
	double t0 = omp_get_wtime();
	std::unordered_map<std::string,BGPNode*> full_network = read_input_file(opt);
	std::vector<std::string> node_list;
	std::unordered_set<std::string> node_set;

	//for tabulating results
	std::unordered_map<std::string, double> ito_paths;
	std::unordered_map<std::string, double> ito_community_paths;
	std::unordered_map<std::string, double> oti_paths;
	std::unordered_map<std::string, double> oti_community_paths;


	for(auto it : full_network) {
		auto it_v = node_set.find(it.first);
		if(it_v == node_set.end()){
			node_list.push_back(it.first);
			node_set.insert(it.first);
			ito_paths.insert(std::make_pair( it.first, 0.0 ));
			oti_paths.insert(std::make_pair( it.first, 0.0 ));
			ito_community_paths.insert(std::make_pair( it.second->community_label, 0.0 ));
			oti_community_paths.insert(std::make_pair( it.second->community_label, 0.0 ));
		}
		for(auto it2 : it.second->neighbors) {

			it_v = node_set.find(it2.first);
			if(it_v == node_set.end()){
				node_list.push_back(it2.first);
				node_set.insert(it.first);
				ito_paths.insert(std::make_pair( it2.first, 0.0 ));
				oti_paths.insert(std::make_pair( it2.first, 0.0 ));
				ito_community_paths.insert(std::make_pair( full_network[it2.first]->community_label, 0.0 ));
				oti_community_paths.insert(std::make_pair( full_network[it2.first]->community_label, 0.0 ));
			}

		}
	}
	double t1 = omp_get_wtime();
	printf("Finished Loading network in %f seconds...\n", t1-t0);

	//run BFS
	t0 = omp_get_wtime();
	std::string source = "33891";
	std::unordered_map<std::string, int> path_counts = count_paths_bgp(full_network,source);
	t1 = omp_get_wtime();
	printf("Finished BFS in %f seconds...\n", t1-t0);

	for(auto it : path_counts) {
		printf("%s:%d\n",it.first.c_str(), it.second);
		std::cout << std::flush;
	}

	//clear memory
	printf("Cleaning memory...\n");
	for(auto it : full_network) {
		delete it.second;
	}

	return 0;
}