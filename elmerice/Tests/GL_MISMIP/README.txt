Require two calls of the FreeSurfaceSolver. 
It is then needed to make a copy of the object file FreeSurfaceSolver.so 
(or FreeSurfaceSolver.dylib for mac): 
cp $ELMER_HOME/share/elmersolver/lib/FreeSurfaceSolver.so MyFreeSurfaceSolver.so

To execute:
ElmerSolver mismip.sif

All command-lines are written in  Makefile
