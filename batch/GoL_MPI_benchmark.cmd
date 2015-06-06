#PBS -S /bin/bash
#PBS -u egentry
#PBS -N GoL_MPI

#PBS -l nodes=8:ppn=8
#PBS -l walltime=00:03:00

#PBS -V
#PBS -q newest

cd $PBS_O_WORKDIR
cd ..

export PRINTING_PROC=0

export NUM_TIMESTEPS=100
export NUM_GUARD_CELLS=1

# make sure function is cached + ready to go
 mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS
 mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS



rm -f data/benchmark_results.dat
echo "Procs    Time     Real Cells   Time Steps" >> data/benchmark_results.dat

mpirun  -n  1 ./GoL_MPI  1  1 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS
mpirun  -n  2 ./GoL_MPI  2  1 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS
mpirun  -n  4 ./GoL_MPI  2  2 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS
mpirun  -n  8 ./GoL_MPI  4  2 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS
mpirun  -n 16 ./GoL_MPI  4  4 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS
mpirun  -n 32 ./GoL_MPI  8  4 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS
mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS


mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS 2
mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS 4
mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS 8
mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS 16
mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS 32
mpirun  -n 64 ./GoL_MPI  8  8 $PRINTING_PROC $NUM_TIMESTEPS 64

