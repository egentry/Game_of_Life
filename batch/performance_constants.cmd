#PBS -S /bin/bash
#PBS -u egentry
#PBS -N performance_constants

#PBS -l nodes=1:ppn=8
#PBS -l walltime=00:02:00

#PBS -V
#PBS -q newest

cd $PBS_O_WORKDIR
cd ..

# run the program to make sure it's in the cache
mpirun -n 8 get_performance_constants comm
mpirun -n 8 get_performance_constants comp

rm -f data/performance_constants.dat

# now run the tests for real
mpirun -n 8 get_performance_constants comm
mpirun -n 8 get_performance_constants comp
