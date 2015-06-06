#PBS -S /bin/bash
#PBS -u egentry
#PBS -N GoL_serial

#PBS -l nodes=1:ppn=1
#PBS -l walltime=00:00:30

#PBS -V
#PBS -q newest

cd $PBS_O_WORKDIR
cd ..

./GoL_serial