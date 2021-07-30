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