#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "GoL.h"
#include "communicate.h"

#include "get_performance_constants.h"

#include <mpi.h>


int main(int argc, char *argv[])
{

	int proc_rank, proc_size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &proc_rank);
	MPI_Comm_size( MPI_COMM_WORLD, &proc_size);
	const int root_proc = 0;

	// char filename[] = "dat/performance_constants.dat";
	// FILE *file_pointer = fopen(filename, 'a');

	if (argc > 1)
	{
		if (strcmp(argv[1], "comp")==0)
		{
			int num_timesteps = 1024;
			if (argc > 2)
			{
				char *end;
				num_timesteps = strtol(argv[2], &end, 10);
			}
			double comp_time = get_comp_time(proc_rank, num_timesteps);
			double comp_time_avg = get_filtered_average(comp_time, proc_rank, proc_size, root_proc);

			if (proc_rank == root_proc)
			{
				char filename[] = "data/performance_constants.dat";
				FILE *file_pointer = fopen(filename, "a");
				fprintf(file_pointer, "Computation: t = %e [s] / true grid point / timestep \n", comp_time_avg );
				fclose(file_pointer);
			}

		}
		if (strcmp(argv[1], "comm")==0)
		{
			long size;
			size = 1;
			double comm_time_single = get_comm_time(proc_rank, proc_size, size);
			double comm_time_single_avg = get_filtered_average(comm_time_single, proc_rank, proc_size, root_proc);



			size = 4194304;
			if (argc > 2)
			{
				char *end;
				size = strtol(argv[2], &end, 10);
			}
			double comm_time_long = get_comm_time(proc_rank, proc_size, size);
			double comm_time_long_avg = get_filtered_average(comm_time_long, proc_rank, proc_size, root_proc);
			comm_time_long_avg = comm_time_long_avg / size;

			if (proc_rank == root_proc)
			{
				char filename[] = "data/performance_constants.dat";
				FILE *file_pointer = fopen(filename, "a");
				fprintf(file_pointer, "Communication: t_start = %e [s], t_length = %e [s] / SHORT variable \n", comm_time_single_avg, comm_time_long_avg );
				fclose(file_pointer);
			}

		}
	}

	MPI_Finalize();
	return 0;
}

double get_comp_time(const int proc_rank, const int num_timesteps)
{
	double start_time, end_time;

	const int size_x = 1024;
	const int size_y = 1024;
	const int num_guard_cells = 1;  // one on each end

	const verbose = 0;

	short ** matrix_old;
	short ** matrix_new;

	int i,j,k;


	// // // // // Set up system // // // // //

	matrix_old = allocate_matrix(matrix_old, size_x, size_y);
	matrix_new = allocate_matrix(matrix_new, size_x, size_y);

	int seed = proc_rank + 5; // avoid seeding one proc with 1 and another with 0 -- both will have the same seed
	init_matrix(matrix_old, size_x, size_y, num_guard_cells, seed);

	// // // // // Evolve system // // // // //

	start_time = MPI_Wtime();
	for (k=0; k<num_timesteps; ++k)
	{
		// can complete `num_guard_cells' of updates before need to communicate again
		dead_or_alive( matrix_old, matrix_new, size_x, size_y, num_guard_cells, verbose);
		swap((void*) &matrix_old, (void*) &matrix_new);
	}
	end_time = MPI_Wtime();


	double delta_time = end_time - start_time;
	long real_cells = (size_x - num_guard_cells) * (size_y - num_guard_cells); // can't be int because I'll be multiplying it by a number that might overflow
	double time_comp =  delta_time / (real_cells * num_timesteps);
	printf("time = %e on proc %d, for %ld cells, %d timesteps \n",time_comp, proc_rank, real_cells, num_timesteps);

	free_matrix(matrix_old, size_x, size_y);
	free_matrix(matrix_new, size_x, size_y);

	return time_comp;
}

double get_comm_time(const int proc_rank, const int proc_size, const long size)
{
	double start_time, end_time;

	short *array_send, *array_recv;
	array_send = malloc(size * sizeof(short));
	array_recv = malloc(size * sizeof(short));

	const verbose = 0;

	int i,j,k;


	// // // // // Set up system // // // // //
	int ierr;
	int num_requests = 2;
	MPI_Request  requests[num_requests];
	MPI_Status   statuses[num_requests];

	int rank_send, rank_recv;
	rank_send = proc_rank + 1;
	rank_recv = proc_rank - 1;
	rank_send = wrap_ranks(rank_send, proc_size);
	rank_recv = wrap_ranks(rank_recv, proc_size);

	int tag_send, tag_recv;
	tag_send = proc_rank; // create tags based on sender
	tag_recv = rank_recv;

	// // // // // // Evolve system // // // // //

	MPI_Barrier(MPI_COMM_WORLD);
	start_time = MPI_Wtime();
	ierr = MPI_Isend(array_send, size, MPI_SHORT, rank_send, tag_send, MPI_COMM_WORLD, &requests[0]);
	ierr = MPI_Irecv(array_recv, size, MPI_SHORT, rank_recv, tag_recv, MPI_COMM_WORLD, &requests[1]);
	ierr = MPI_Waitall(num_requests, requests, statuses);
	end_time = MPI_Wtime();


	double delta_time = end_time - start_time;
	printf("took time = %e  on proc = %d for size %ld \n", delta_time, proc_rank, size);

	free(array_send);
	free(array_recv);

	return delta_time;
}

double get_filtered_average(const double time, const int proc_rank, const int proc_size, const int root_proc)
{
	double times[proc_size];

	int ierr;
	int count = 1;
	int recv_count = proc_size;

	ierr = MPI_Gather(&time, count, MPI_DOUBLE, 
					  times, count, MPI_DOUBLE, root_proc, MPI_COMM_WORLD);

	int i;

	if (proc_rank == root_proc)
	{
		if (proc_size > 2)
		{
			// filter out min and max times
			int max_ind  = 0;
			int max_elem = times[0];
			int min_ind  = 0;
			int min_elem = times[0];

			for (i=0; i<proc_size; ++i)
			{
				if (times[i] > max_elem)
				{
					max_ind  = i;
					max_elem = times[i];
				}
				if (times[i] < min_elem)
				{
					min_ind  = i;
					min_elem = times[i];
				}
			}

			times[min_ind] = -1; // any times below 0 will be ignored.
			times[max_ind] = -1; // any times below 0 will be ignored.
		}

		int valid_points = 0;
		double time_sum  = 0;
		for(i=0; i<proc_size; ++i)
		{
			if (times[i] > 0)
			{	
				time_sum += times[i];
				++valid_points;
			}
		}
		double time_averaged = time_sum / valid_points;

		return time_averaged;
	}
	else
	{
		return 0.;
	}
		
}









