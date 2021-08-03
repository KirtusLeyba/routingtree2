import sys
import pandas as pd

input_file = sys.argv[1]
output_file = sys.argv[2]

rel_frame = pd.read_csv(input_file, delimiter="|", names = ["AS_A", "AS_B", "REL"],
						comment="#")

edge_set =  set()
for index, row in rel_frame.iterrows():
	edge = sorted( [ row["AS_A"], row["AS_B"] ] )
	if(int(row["REL"]) == 0):
		edge.append("p2p")
	else:
		edge.append("p2c")
	edge = tuple(edge)
	edge_set.add(edge)

with open(output_file, "w") as fp:
	for edge in edge_set:
		fp.write("{} {} {} {} {}\n".format(edge[0], edge[1], edge[2], "a", "a"))