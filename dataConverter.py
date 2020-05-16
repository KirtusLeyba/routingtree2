### Converts the AS relationships into a form usable by BGPSimPy ###

import sys

inFile = str(sys.argv[1])
outFile = str(sys.argv[2])

of = open(outFile, 'w')

with open(inFile) as fp:
    line = fp.readline()
    cnt = 1
    nodeList = []
    while line:
        if(line[0] != '#'):
            data = line.split('|')
            outString = str(data[0]) + "\t" + str(data[1])
            
            if( int(data[0]) not in nodeList ):
                nodeList.append(int(data[0]))
            if( int(data[1]) not in nodeList ):
                nodeList.append(int(data[1]))

            if(data[2] == "0\n" or data[2] == "0"):
                outString += "\t" + "p2p\n" ## add endline here for saving
            else:
                outString += "\t" + "p2c\n"
            of.write(outString)
            line = fp.readline()
            cnt += 1
        else:
            line = fp.readline()

    print("number of unique nodes: " + str(len(nodeList)))

of.close()

### Saving the node list ###
outFileList = outFile + ".nodeList"
ouf = open(outFileList, 'w')
for node in nodeList:
    ouf.write(str(node) + "\n")
ouf.close()
