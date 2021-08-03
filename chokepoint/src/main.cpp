#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <omp.h>
#include "options.hpp"
#include "bgpgraph.hpp"

int main(int argc, char** argv) {

	Options opt = parse_args(argc, argv);
	if(!check_args(opt)){
		exit(0);
	}

	// for tabulating results
	int num_threads = 0;
	#pragma omp parallel
	{
		#pragma omp critical
		{
			num_threads += 1;
		}
	}
	printf("#Running with openmp using %d threads!\n", num_threads);
	std::vector< std::unordered_map< std::string, double > > all_path_counts(num_threads);
	std::vector< std::unordered_map< std::string, double > > all_path_counts_cc(num_threads);

	#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();
		//load network from data
		printf("#Thread %d Loading network...\n", thread_id);
		double t0 = omp_get_wtime();
		std::unordered_map<std::string,BGPNode*> full_network = read_input_file(opt);
		std::vector<std::string> node_list;
		std::unordered_set<std::string> node_set;


		for(auto it : full_network) {
			auto it_v = node_set.find(it.first);
			if(it_v == node_set.end()){
				node_list.push_back(it.first);
				node_set.insert(it.first);
			}
			for(auto it2 : it.second->neighbors) {

				it_v = node_set.find(it2.first);
				if(it_v == node_set.end()){
					node_list.push_back(it2.first);
					node_set.insert(it.first);
				}

			}
		}
		double t1 = omp_get_wtime();
		printf("#Thread %d Finished Loading network in %f seconds...\n",thread_id, t1-t0);

		//run BFS
		int num_nodes = node_list.size();
		#pragma omp barrier
		t0 = omp_get_wtime();
		#pragma omp for schedule(dynamic, 1000)
		for(int i = 0; i < node_list.size(); i++) {
			t1 = omp_get_wtime();
			printf("# Working on node %d\n", i);
			printf("# Progress: %f in %f seconds", (double)i/(double)num_nodes, t1-t0);
			std::string source = node_list[i];
			count_paths_bgp(full_network,
							all_path_counts[thread_id],
							all_path_counts_cc[thread_id],
							source);
		}
		t1 = omp_get_wtime();
		printf("#Thread %d Finished work in %f seconds...\n",thread_id, t1-t0);

		//clear memory
		printf("#Thread %d Cleaning memory...\n", thread_id);
		for(auto it : full_network) {
			delete it.second;
		}

	}

	// merge results
	std::unordered_map<std::string, double> merged_path_counts;
	for(int i = 0; i < num_threads; i++) {
		for(auto it : all_path_counts[i]) {
			if( merged_path_counts.find(it.first) == merged_path_counts.end()) {
				merged_path_counts[it.first] = it.second;
			} else {
				merged_path_counts[it.first] += it.second;
			}
		}
	}

	std::unordered_map<std::string, double> merged_path_counts_cc;
	for(int i = 0; i < num_threads; i++) {
		for(auto it : all_path_counts_cc[i]) {
			if( merged_path_counts_cc.find(it.first) == merged_path_counts_cc.end()) {
				merged_path_counts_cc[it.first] = it.second;
			} else {
				merged_path_counts_cc[it.first] += it.second;
			}
		}
	}

	// print results
	printf("#ASN,PathCount\n");
	for(auto it : merged_path_counts) {
		printf("%s,%f\n", it.first.c_str(), it.second);
	}
	printf("#skip\n");
	printf("#CC,PathCount\n");
	for(auto it : merged_path_counts_cc) {
		printf("%s,%f\n", it.first.c_str(), it.second);
	}


	return 0;
}