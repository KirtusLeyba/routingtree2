### Converts a p2p/p2c text file into a usable matrix ###

import numpy as np
from scipy import sparse
import scipy.io
import time
import itertools
from itertools import izip
import copy
import sys

inFile = str(sys.argv[1])
outFile = str(sys.argv[2])

def determineNodeCount(fileName, outName):
    nodeList = []
    with open(fileName, 'r') as f:
        content = f.readlines()
    for line in content:
        if(line[0] != '#'):
            splitLine = line.split("\t", 2)
            if( int(splitLine[0]) not in nodeList ):
                nodeList.append(int(splitLine[0]))
            if( int(splitLine[1]) not in nodeList ):
                nodeList.append(int(splitLine[1]))
    print("Node Count: " + str(len(nodeList)))
    print("Max Node ID: " + str(max(nodeList)))
    ### Saving the node list ###
    outFileList = outName + ".nodeList"
    ouf = open(outFileList, 'w')
    for node in nodeList:
        ouf.write(str(node) + "\n")
    ouf.close()
    return max(nodeList)


def fileToSparse(fileName, outName):
    '''
    reads the full AS graph in as a text file of relationships,
    converts it to a sparse matrix (note that row x or column x is for AS x)
    saves the sparse matrix 
    loads the sparse matrix and times the loading
    usage: fileToSparse("Cyclops_caida_cons.txt")
    '''

    numNodes = determineNodeCount(fileName, outName)

    with open(fileName,'r') as f:
        content = f.readlines()
    empMatrix = sparse.lil_matrix((numNodes+1,numNodes+1), dtype=np.int8)
    i = 1
    total = len(content)
    for line in content:
        if i%1000 == 0:
            print("completed: " + str((float(i)/float(total))*100.0))
        i += 1
        splitLine = line.split("\t",2)
        node1 = int(splitLine[0])
        node2 = int(splitLine[1])
        relationship = splitLine[2][:3]
        if relationship == "p2p":
            empMatrix[node1,node2] = 1
            empMatrix[node2,node1] = 1
        if relationship == "p2c":
            empMatrix[node1,node2] = 2
            empMatrix[node2,node1] = 3
    empMatrix = empMatrix.tocsr()
    scipy.io.mmwrite(outName,empMatrix)
    start = time.time()
    test = scipy.io.mmread(outName).tolil()  #5.4MB to save sparse matrix
    end = time.time()
    print end-start, " seconds to load" #2.3 seconds

fileToSparse(inFile, outFile)
