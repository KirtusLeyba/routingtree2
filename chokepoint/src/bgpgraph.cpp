#include "bgpgraph.hpp"

BGPNode::BGPNode(std::string _asn, std::string _community_label) {
	asn = _asn;
	community_label = _community_label;
}

inline bool check_in(std::string a, std::unordered_map<std::string,BGPNode*> &G){
	auto it = G.find(a);
	if(it == G.end()){
		return false;
	}
	return true;
}

std::unordered_map<std::string,BGPNode*> read_input_file(Options opt) {
	std::unordered_map<std::string,BGPNode*> G;

	std::ifstream fp(opt.input_file);
	std::string line;
	while(std::getline(fp, line)) {
		std::istringstream iss(line);
		std::string asn_a;
		std::string asn_b;
		std::string rel;
		std::string community_label_a;
		std::string community_label_b;

		iss >> asn_a >> asn_b >> rel >> community_label_a >> community_label_b;

		BGPNode* node_a;
		BGPNode* node_b;
		if(check_in(asn_a, G)) {
			node_a = G[asn_a];
		} else {
			node_a = new BGPNode(asn_a, community_label_a);
			G.insert( std::make_pair(node_a->asn, node_a) );
		}
		if(check_in(asn_b, G)) {
			node_b = G[asn_b];
		} else {
			node_b = new BGPNode(asn_b, community_label_b);
			G.insert( std::make_pair(node_b->asn, node_b) );
		}

		G[asn_a]->neighbors.insert( std::make_pair(node_b->asn, rel) );
		if(rel == "p2p") {
			G[asn_b]->neighbors.insert(std::make_pair(node_a->asn, rel));
		}
		else if(rel == "p2c") {
			G[asn_b]->neighbors.insert(std::make_pair(node_a->asn, "c2p"));
		}
		else if(rel == "c2p") {
			G[asn_b]->neighbors.insert(std::make_pair(node_a->asn, "p2c"));
		}

		if(community_label_a != community_label_b) {
			G[asn_a]->is_border = true;
			G[asn_b]->is_border = true;
		}

	}
	fp.close();

	return G;
}

void traverse_paths(std::string node,
	std::unordered_map<std::string, std::vector<std::string>> &parents,
	std::unordered_map<std::string, int> &path_counts){
	if(parents.find(node) != parents.end()) {
		for(unsigned int i = 0; i < parents[node].size(); i++) {
			path_counts[node] += 1;
			traverse_paths(parents[node][i], parents, path_counts);
		}
	} else {
		path_counts[node] += 1;
	}
}

std::unordered_map<std::string, int> count_paths(std::unordered_map<std::string, BGPNode*> &G,
												std::string source) {

	//for return value
	std::unordered_map<std::string, int> path_counts;
	if(!check_in(source, G)){
		return path_counts;
	}

	// local variables for tracking BFS
	std::unordered_map<std::string, std::vector<std::string>> parents;
	std::unordered_map<std::string, int> distances;
	std::unordered_set<std::string> visited;
	std::queue<std::string> frontier;

	//Set up the BFS starting from source
	std::string current_node = source;
	visited.insert(source);
	// parents.insert( std::make_pair(source, std::vector<std::string>()) );
	distances.insert( std::make_pair(source, 0) );
	for(auto it : G[current_node]->neighbors) {
		if(distances.find(it.first) != distances.end()) {
			if(distances[it.first] == distances[current_node] + 1) {
				parents[it.first].push_back(current_node);
			}
		} else {
			distances.insert( std::make_pair(it.first, distances[current_node] + 1) );
			parents.insert( std::make_pair(it.first, std::vector<std::string>()) );
			parents[it.first].push_back(current_node);
		}
		auto in_visited = visited.find(it.first);
		if(in_visited == visited.end()){
			frontier.push(it.first);
		}
	}

	//run BFS
	while(!frontier.empty()) {
		// fifo queue
		current_node = frontier.front();
		frontier.pop();
		visited.insert(current_node);
		for(auto it : G[current_node]->neighbors) {
			if(distances.find(it.first) != distances.end()) {
				if(distances[it.first] == distances[current_node] + 1) {
					parents[it.first].push_back(current_node);
				}
			} else {
				distances.insert( std::make_pair(it.first, distances[current_node] + 1) );
				parents.insert( std::make_pair(it.first, std::vector<std::string>()) );
				parents[it.first].push_back(current_node);
			}
			auto in_visited = visited.find(it.first);
			if(in_visited == visited.end()){
				frontier.push(it.first);
			}
		}
	}

	//iterate through found paths
	for(auto it : parents) {
		traverse_paths(it.first, parents, path_counts);
	}

	return path_counts;

}