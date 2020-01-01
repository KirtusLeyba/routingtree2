from time import time
import networkx as nx
import pygraphviz
from networkx.drawing.nx_agraph import write_dot
import sys

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

def expand(node, adjtable, frontier, visitedC2P, visitedP2P):

	children = []
	if(node.fromType is None):
		possibleChildren = adjtable[node.asn]
		for p in possibleChildren:
			sn = SearchNode(p[1], p[0], node, [])
			if (p[0] == "c2p"):
				children.append(sn)
				frontier.append(sn)
				visitedC2P.add(sn.asn)
			elif (p[0] == "p2p"):
				children.append(sn)
				frontier.append(sn)
				visitedP2P.add(sn.asn)
			elif (p[0] == "p2c"):
				children.append(sn)
				frontier.append(sn)
	elif(node.fromType == "c2p"):
		possibleChildren = adjtable[node.asn]
		for p in possibleChildren:
			sn = SearchNode(p[1], p[0], node, [])
			if (p[0] == "c2p"):
				children.append(sn)
				frontier.append(sn)
				visitedC2P.add(sn.asn)
			elif (p[0] == "p2p" and sn.asn not in visitedC2P):
				children.append(sn)
				frontier.append(sn)
				visitedP2P.add(sn.asn)
			elif (p[0] == "p2c" and sn.asn not in visitedC2P and sn.asn not in visitedP2P):
				children.append(sn)
				frontier.append(sn)
	elif(node.fromType == "p2p"):
		possibleChildren = adjtable[node.asn]
		for p in possibleChildren:
			sn = SearchNode(p[1], p[0], node, [])
			if(p[0] == "p2c" and sn.asn not in visitedP2P and sn.asn not in visitedC2P):
				children.append(sn)
				frontier.append(sn)
	elif(node.fromType == "p2c"):
		possibleChildren = adjtable[node.asn]
		for p in possibleChildren:
			sn = SearchNode(p[1], p[0], node, [])
			if p[0] == "p2c" and sn.asn not in visitedP2P and sn.asn not in visitedC2P:
				children.append(sn)
				frontier.append(sn)
	node.children = children

	return children

def ThreeStageBFS(startnode, adjtable):
	expanded = 0
	frontier = []
	visitedC2P = set()
	visitedP2P = set()
	frontier.append(startnode)
	visitedC2P.add(startnode.asn)
	while(len(frontier) > 0):
		node = frontier.pop(0)
		expand(node, adjtable, frontier, visitedC2P, visitedP2P)
		expanded += 1

	print("Expanded {} nodes".format(expanded))

	return startnode

def ThreeStageDFS(startnode, adjtable):
	expanded = 0
	frontier = []
	visitedC2P = set()
	visitedP2P = set()
	frontier.append(startnode)
	visitedC2P.add(startnode.asn)
	while(len(frontier) > 0):
		node = frontier.pop(len(frontier)-1)
		expand(node, adjtable, frontier, visitedC2P, visitedP2P)
		expanded += 1

	print("Expanded {} nodes".format(expanded))


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
					adjtable[asnA].append(("c2p",asnB))
					adjtable[asnB].append(("p2c",asnA))
			line = fp.readline()

	return adjtable

def addEdges(G, node, depth=0, maxDepth=4):
	if(depth > maxDepth):
		return
	for c in node.children:
		if(c.fromType == "p2p"):
			color = "#f0e800"
		elif(c.fromType == "p2c"):
			color = "#6c2dba"
		else:
			color = "#337a4d"
		G.add_edge(c.asn, node.asn, edgeType=c.fromType, color=color, penwidth=3)
		addEdges(G, c, depth+1, maxDepth)


print("Setting up...")
adjtable = loadAdjTable(sys.argv[1])
startnode = SearchNode(int(sys.argv[2]), None, None, [])

print("Running 3 stage BFS for asn {}".format(startnode.asn))
starttime = time()
result = ThreeStageBFS(startnode, adjtable)
endtime = time()

print("Generated the routing tree for asn {} in {} seconds".format(startnode.asn, endtime - starttime))

G = nx.DiGraph()
addEdges(G,startnode,depth=0,maxDepth=1000000000000000000000000000000)

print("Graph has {} nodes and {} edges".format(len(G.nodes), len(G.edges)))

write_dot(G, "bfs.dot")

####

print("Setting up...")
adjtable = loadAdjTable(sys.argv[1])
startnode = SearchNode(int(sys.argv[2]), None, None, [])

print("Running 3 stage DFS for asn {}".format(startnode.asn))
starttime = time()
result = ThreeStageDFS(startnode, adjtable)
endtime = time()


print("Generated the routing tree for asn {} in {} seconds".format(startnode.asn, endtime - starttime))

G = nx.DiGraph()
addEdges(G,startnode,depth=0,maxDepth=1000000000000000000000000000000)

print("Graph has {} nodes and {} edges".format(len(G.nodes), len(G.edges)))

write_dot(G, "dfs.dot")