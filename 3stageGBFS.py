from time import time
import networkx as nx
import pygraphviz
from networkx.drawing.nx_agraph import write_dot

class SearchNode:
	def __init__(self, asn, fromType, parent, children):
		self.asn = asn
		self.fromType = fromType
		self.parent = parent
		self.children = children

	def __eq__(self, other):
		return self.asn == other.asn

	def __hash__(self):
		return hash(self.asn)

def expand(node, adjtable, frontier, visited):
	children = []
	if(node.fromType is None):
		possibleChildren = adjtable[node.asn]
		for p in possibleChildren:
			sn = SearchNode(p[1], p[0], node, [])
			if p[0] == "c2p" and sn.asn not in visited:
				children.append(sn)
				frontier.append(sn)
				visited.add(sn.asn)
	elif(node.fromType == "c2p"):
		possibleChildren = adjtable[node.asn]
		for p in possibleChildren:
			sn = SearchNode(p[1], p[0], node, [])
			if(p[0] == "c2p" or p[0] == "p2p" and sn.asn not in visited):
				children.append(sn)
				frontier.append(sn)
				visited.add(sn.asn)
	elif(node.fromType == "p2c"):
		possibleChildren = adjtable[node.asn]
		for p in possibleChildren:
			sn = SearchNode(p[1], p[0], node, [])
			if p[0] == "p2c" and sn.asn not in visited:
				children.append(sn)
				frontier.append(sn)
				visited.add(sn.asn)
	node.children = children
	return children

def ThreeStageBFS(startnode, adjtable):
	frontier = []
	visited = set()
	frontier.append(startnode)
	visited.add(startnode.asn)
	while(len(frontier) > 0):
		node = frontier.pop(0)
		expand(node, adjtable, frontier, visited)

	return startnode

def loadAdjTable(relfile):
	adjtable = {}
	with open(relfile) as fp:
		line = fp.readline()
		while(line):
			if(line[0]!="#"):
				s = line.split("|")
				asnA = int(s[0])
				asnB = int(s[1])
				if asnA not in adjtable:
					adjtable[asnA] = []
				if asnB not in adjtable:
					adjtable[asnB] = []
				rel = int(s[2])
				if rel == 0:
					adjtable[asnA].append(("p2p",asnB))
					adjtable[asnB].append(("p2p",asnA))
				elif rel == -1:
					adjtable[asnA].append(("p2c",asnB))
					adjtable[asnB].append(("c2p",asnA))
			line = fp.readline()

	return adjtable


print("Setting up...")
adjtable = loadAdjTable("20191101.as-rel.txt")
startnode = SearchNode(265138, None, None, [])

print("Running search...")
starttime = time()
result = ThreeStageBFS(startnode, adjtable)
endtime = time()

print("Generated the routing tree for asn {} in {} seconds".format(startnode.asn, endtime - starttime))



def addEdges(G, node, depth=0, maxDepth=4):
	if(depth > maxDepth):
		return
	for c in node.children:
		if(c.fromType == "p2p"):
			color = "blue"
		elif(c.fromType == "p2c"):
			color = "red"
		else:
			color = "green"
		G.add_edge(c.asn, node.asn, edgeType=c.fromType, color=color)
		addEdges(G, c, depth+1, maxDepth)
G = nx.DiGraph()
addEdges(G,startnode,depth=0,maxDepth=1)

print("Graph has {} nodes and {} edges".format(len(G.nodes), len(G.edges)))

write_dot(G, "grid.dot")