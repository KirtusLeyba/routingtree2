'''

The following is an implementation of
BGPSimPy, a Border Gateway Protocol
Simulator written in Python by Cynthia
Freeman. This implementation by Kirtus
Leyba includes the mpi4y library in order
to distribute the workload onto multiple nodes
in a cluster.


This and previous implementations of BGPSimPy
are released open source under the CRAP-L license.
See CRAPL-LICENSE.txt


Questions:
kleyba@asu.edu

'''

'''

GRT.py

GRT = Generate Routing Trees
This is the main crux of the BGP simulation.
A routing tree is a tree structure representing
all of the paths to get to a destination node (the root)
to another node (any node in the routing tree).

Arguments:
    0 - Relative Path to Adjacency Matrix File (required)
    1 - verbose key, v = verbose, f = noVerbose (required)
    2 - Relative Path to output directory (required)
    3 - Relative Path to nodelist file (required)
'''


### This is an MPI based parallel Application ###

### Imports ###
from mpi4py import MPI
from scipy import sparse
import scipy.io
import sys
import numpy as np
import time
import itertools
from itertools import izip
import copy
import sys

### Routing Tree Functions ###
def checkPreviousLevelsAlt(BFS,node,level):
    '''
    check if node is in BFS at given level or any previous level
    '''
    while level >= 0:
        if node in BFS[level][1]:
            return True
        level -= 1
    return False

def customerToProviderBFS(destinationNode,routingTree,graph):
    '''
    input: 
        destinationNode (the root of routing tree)
        empty routing tree which is sparse also
    output:
        routing tree after step 1 of routing tree algorithm
        nodes added this step as a dictionary where key = level and value = list of nodes
    what it does:
        perform a bfs from destinationNode and only add relationship = 3 
    '''
    BFS = [(0,[destinationNode])]
    levels = {} #Dictionary returning the highest level of each key
    allNodes = set(np.append(graph.nonzero()[1], graph.nonzero()[0]))
    for node in allNodes:
        levels[node] = -1

    levels[destinationNode] = 0

    for pair in BFS:
        level = pair[0]
        vertices = pair[1]
        for vertex in vertices:
            for node,relationship in izip(fullGraph[vertex].nonzero()[1],fullGraph[vertex].data):
                if (relationship == 3) and (routingTree[node, vertex] == 0 and routingTree[vertex, node] == 0) and ((not levels[node] <= level) or (levels[node] == -1)): 
                    routingTree[node,vertex] = 1
                    if BFS[-1][0] == level:
                        BFS.append((level+1,[node]))
                        levels[node] = level+1
                    else:
                        BFS[-1][1].append(node)
                        levels[node] = BFS[-1][0]
    return routingTree,BFS,levels

def peerToPeer(routingTree,BFS,graph,levels):
    '''
    input:
        routing tree which is sparse also
        nodes from step 1 of RT algorithm in bfs order
    output:
        routing tree after step 2 of routing tree algorithm
        nodes added from this step and previous step as a dictionary where key = level and value = list of nodes
    purpose:
        connect new nodes to nodes added in step 1 with relationship = 1
    '''
    oldNodes = []
    old = {}
    allNodes = set(np.append(graph.nonzero()[1], graph.nonzero()[0]))
    for node in allNodes:
        old[node] = 0

    for pair in BFS:
        oldNodes.extend(pair[1])
        for node in pair[1]:
            old[node] = 1
    newBFS = copy.deepcopy(BFS)
    newLevels = levels
    for pair in BFS:
        level = pair[0]
        #print "---level---: ",level
        vertices = pair[1]
        for vertex in vertices:
            for node,relationship in izip(fullGraph[vertex].nonzero()[1],fullGraph[vertex].data):
                if (relationship == 1) and (old[node] == 0): 
                    routingTree[node,vertex] = 1
                    if newBFS[-1][0] == level:
                        newBFS.append((level+1,[node]))
                        newLevels[node] = level+1
                    else:
                        newBFS[-1][1].append(node)
                        newLevels[node] = newBFS[-1][0]
    return routingTree,newBFS,newLevels

def providerToCustomer(routingTree,BFS,graph,levels):
    '''
    input:
        routing tree which is sparse also
        nodes from step 1 and 2 of RT algorithm
    output:
        routing tree after step 3 of routing tree algorithm
        nodes added from this step and previous two steps as a dictionary where key = level and value = list of nodes
    purpose:
        breadth first search of tree, add nodes with relationship 2
    '''
    edgesCount = 0
    oldNodes = []
    old = {}
    allNodes = set(np.append(graph.nonzero()[1], graph.nonzero()[0]))
    for node in allNodes:
        old[node] = 0
    for pair in BFS:
        oldNodes.extend(pair[1])

    for node in oldNodes:
        old[node] = 1

    for pair in BFS:
        level = pair[0]
        vertices = pair[1]
        for vertex in vertices:
            for node,relationship in izip(fullGraph[vertex].nonzero()[1],fullGraph[vertex].data):
                if (relationship == 2) and (routingTree[vertex,node] == 0 and routingTree[node,vertex] == 0) and old[node] == 0 and (( not(levels[node] <= level)) or (levels[node] == -1)): 
                    routingTree[node,vertex] = 1
                    if BFS[-1][0] == level:
                        BFS.append((level+1,[node]))
                        levels[node] = level+1
                    else:
                        BFS[-1][1].append(node)
                        levels[node] = BFS[-1][0]
    return routingTree

def saveAsNPZ(fileName, matrix):
    matrixCOO = matrix.tocoo()
    row = matrixCOO.row
    col = matrixCOO.col
    data = matrixCOO.data
    shape = matrixCOO.shape
    np.savez(fileName, row = row, col = col, data = data, shape = shape)

def makeRoutingTree(destinationNode):
    '''
    input: 
        destination AS
    output:
        routing tree of destination AS in sparse matrix format
    '''
    print("=================" + str(destinationNode) + "=======================")
    routingTree = sparse.dok_matrix((numNodes+1,numNodes+1), dtype=np.int8)
    stepOneRT,stepOneNodes,lvls = customerToProviderBFS(destinationNode,routingTree,fullGraph)
    stepTwoRT,stepTwoNodes,lvlsTwo = peerToPeer(stepOneRT,stepOneNodes,fullGraph,lvls)
    stepThreeRT = providerToCustomer(stepTwoRT,stepTwoNodes,fullGraph,lvlsTwo)
    saveAsNPZ(str(args[3]) + "dcomplete"+str(destinationNode), stepThreeRT)
    return stepThreeRT


### Helper Functions ###

### getRankBounds
### Input:
###   nodeListFile, a file holding list of ASNode IDs
### Output:
###   bounds, a dictionary describing the bounds of the rank
def getRankBounds(nodeList):
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    numRanks = comm.Get_size()

    dataSize = len(nodeList)

    if(verbose):
        print("Node Count: " + str(dataSize))

    dataSizePerRank = dataSize/numRanks
    leftOver = dataSize%numRanks
    startIndex = dataSizePerRank*rank
    lastIndex = (dataSizePerRank*(rank+1))-1
    if(rank < leftOver):
        dataSizePerRank += 1
        if(rank != 0):
            startIndex += 1
            lastIndex += 2
        else:
            lastIndex += 1
    else:
        startIndex += leftOver
        lastIndex += leftOver

    return startIndex, lastIndex






# Interpret User Input
args = sys.argv
verbose = False
if(args[2] == 'v'):
    verbose = True
#interpretArgs(args) #TODO

### initialization phase ###
fullGraph = scipy.io.mmread(str(args[1])).tocsr() #read the graph on all ranks


comm = MPI.COMM_WORLD
rank = comm.Get_rank()
numRanks = comm.Get_size()

### Get the node count and node list
if(rank  == 0):
    nodeListFile = str(args[4])
    nodeList = []
    with open(nodeListFile) as fp:
        line = fp.readline()
        while line:
            if(line[-1] == '\n'):
                line = line[:-1]
            nodeList.append(int(line))
            line = fp.readline()
    if(verbose):
        print("Max ASNode ID: " + str(max(nodeList)))
else:
    nodeList = None

nodeList = comm.bcast(nodeList, root = 0)
numNodes = int(args[5])#max(nodeList)
firstIndex, lastIndex = getRankBounds(nodeList)

if(verbose): ### Printing MPI Status for debugging purposes
    print("MPI STATUS... rank " + str(rank) + " reporting... working on nodes " + str(firstIndex) + " to " + str(lastIndex))

comm.Barrier() ## Synchronize here, then continue ##

timer = {'start':0.0, 'end':0.0}
timer['start'] = time.time()

### Primary Loop, executed distrubitively in parallel
for index in range(firstIndex, lastIndex + 1):
    destinationNode = nodeList[index]
    routingTree = makeRoutingTree(destinationNode) ### Calculate the routing tree for this node
    
### wait for all ranks to check time
comm.Barrier()
timer['end'] = time.time()
if(rank == 0):
    print("All Routing Trees Completed. Elapsed Time: " + str((timer['end'] - timer['start'])))
