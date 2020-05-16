# BGP-SAS
BGP-SAS (Border Gateway Protocol - Simulation, Analysis, and Storage) is a suite of tools for utilizing cluster computing (with MPI and Python via MPI4Py) to find routing-trees according to the Gao-Rexford model of routing. The algorithms used and an application of BGP-SAS are are detailed in the 2019 ACM SIGCAS COMPASS conference paper "Borders and Gateways: Measuring and Analyzing National AS Chokepoints". Please cite this work if you use BGP-SAS code for your project:

```
@inproceedings{leyba2019borders,
  title={Borders and gateways: measuring and analyzing national as chokepoints},
  author={Leyba, Kirtus G and Edwards, Benjamin and Freeman, Cynthia and Crandall, Jedidiah R and Forrest, Stephanie},
  booktitle={Proceedings of the 2nd ACM SIGCAS Conference on Computing and Sustainable Societies},
  pages={184--194},
  year={2019}
}
```

The primary tool in BGP-SAS is the routing tree generator, which is designed to be used in a cluster computing environment with MPI due to the heavy computation costs. A sample slurm script for running the routing tree algorithm (contained in speedyGRT.py) is listed below:

```
#!/bin/bash
#SBATCH -n 100
#SBATCH -J routingTrees
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=kleyba@asu.edu
#SBATCH --time=72:00:00

module load openmpi/3.0.0-gnu-4.9.4

mpirun -np 100 --map-by node --rank-by node -display-map python ./PythonCode/speedyGRT.py ./DataSets/20180101/20180101.as-rel.txtGraph.mtx v ./DataSets/20180101/routingTrees/ ./DataSets/20180101/20180101.as-rel.txt.bgp.nodeList 397206


```