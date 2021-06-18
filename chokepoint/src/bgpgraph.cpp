#include "bgpgraph.hpp"

Options parse_args(int argc,char** argv) {

	if(argc != 9){
		printf("USAGE: ./chokepoint --inputfile <inputfile> --outputfile <outputfile> --bordermode <bordermode> --storencp <1=true,0=false>\n");
		exit(0);
	}

	Options opt;
	opt.input_file = argv[2];
	opt.output_file = argv[4];
	opt.border_mode = argv[6];
	opt.store_ncp = atoi(argv[8]);
	return opt;
}

BGPNode::BGPNode(std::string _asn, std::string _community_label) {
	asn = _asn;
	community_label = _community_label;
}

inline bool check_in(std::string a, std::unordered_map<std::string,BGPNode*> G){
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

void mod_bfs(std::unordered_map<std::string,BGPNode*> &G,
			std::string source;
			std::unordered_map<std::string, double> &ito_paths,
			std::unordered_map<std::string, double> &ito_community_paths,
			std::unordered_map<std::string, double> &oti_paths,
			std::unordered_map<std::string, double> &oti_community_paths,
			Options opt) {
	//modified BFS for BGPSAS
	std::squeue<std::string> frontier;
	std::unordered_set<std::string> explored;
	explored.insert(source);

	std::vector< std::vector<std::string> > paths;

	//stage 1: explore as far as possible with c2p links
	std::string current_node = source;
	for( auto it : G[current_node]->neighbors ) {
		std::string n = it.first;
		std::string rel = it.second;
		if( rel == "c2p" ) {
			frontier.push(n);
			explored.insert(n);
			std::vector<std::string> p;
			p.push_back(source);
			p.push_back(n);
			paths.push_back(p);
		}
	}
	while(!frontier.empty()) {
		current_node = frontier.front();
		frontier.pop();
		for(auto it : G[current_node]->neighbors ) {
			std::string n = it.first;
			std::string rel = it.second;
			if( rel == "c2p" ) {
				if( explored.find(n) == explored.end() ) {
					frontier.push(n)
					std::vector<std::vector<std::string>> new_paths;
					// not a good solution!!!! <--- TODO HERE
					for(std::vector<std::string> p : paths) {
						if(p.size() > 0) {
							if(p[p.size() - 1] == current_node) {
								std::vector<std::string> new_p;
								for(std::string s : p) {
									new_p.push_back(s);
								}
								new_p.push_back(n);
								new_paths.push_back(new_p);
							}
						}
					}
					for(std::vector<std::string> p : new_paths) {
						paths.push_back(p);
					}
				}
				explored.insert(n)
			}
		}
	}

	//debug: print paths
	for(auto it : paths) {
		for(unsigned int i = 0; i < it.size(); i++) {
			printf("%s\n", it[i].c_str());
		}
		printf("\n");
	}

}