#PBS -S /bin/bash
#PBS -u egentry
#PBS -N GoL_MPI

#PBS -l nodes=2:ppn=8
#PBS -l walltime=00:00:30

#PBS -V
#PBS -q newest

cd $PBS_O_WORKDIR
cd ..

export SIZE=16
export SIZE_X=4
export SIZE_Y=4

export PRINTING_PROC=0

export NUM_TIMESTEPS=1
export NUM_GUARD_CELLS=1

echo mpirun -n $SIZE ./GoL_MPI $SIZE_X $SIZE_Y $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS

mpirun  -n $SIZE ./GoL_MPI $SIZE_X $SIZE_Y $PRINTING_PROC $NUM_TIMESTEPS $NUM_GUARD_CELLS