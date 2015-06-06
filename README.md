# Game_of_Life
An MPI implementation of Conway's Game of Life (UCSC AMS250 HPC Final Project)
-------

Author: Eric Gentry   (gentry.e@gmail.com; egentry@ucsc.edu)   

Copyright (c) 2015
Licensed under the MIT License

-------

##Main Objectives
  - Implement a serial, and then parallel version of Conway's Game of Life
  - Create a theoretical performance model and evaluate it using empirical runtime data

##Overview
  - A write of the approach and results can be found in `writeup/report_egentry.pdf`
  - A sample animation of the evolution can be found in `writeup/plots/animation.mp4`

##Running
  - To compile, run `make all` in the main directory
  - To run, look for sample PBS batch script within the `batch` directory (you'll need to adapt these as appropriate for your system)
    - `GoL_serial.cmd` runs a sample serial version of the code
    - `GoL_MPI.cmd` runs a sample MPI version of the code
    - `GoL_MPI_benchmarks.cmd` runs a weak scaling test (1-64 processors) and a guard cell scaling test
    - `performance_constants.cmd` determines system constants (computation + communication) using a single node
  - To visualize results run one of the IPython notebooks:
    - `visualize.ipynb` animates recent results (both as an .mp4 and in an interactive GUI; requires running `GoL_MPI` with `verbose=1`)
    - `performance_analysis.ipynb` uses the results of `performance_constants.cmd` to predict runtime results, which are plotted against the empirical results of `GoL_MPI_benchmarks.cmd`
 

##Requires
  - gcc (makefile can be adapted to use other compilers)
  - mpicc (makefile assumes OpenMPI)
  - For visualization:
    - Python (tested for v.3, probably backwards compatible)
    - IPython 3
    - Matplotlib
    - Numpy
    - Pandas
    - Seaborn
