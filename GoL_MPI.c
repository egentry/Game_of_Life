#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "GoL.h"
#include "communicate.h"
#include "io.h"

#include <mpi.h>

int main(int argc, char *argv[])
{
	int proc_rank, proc_size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &proc_rank);
	MPI_Comm_size( MPI_COMM_WORLD, &proc_size);

	double start_time, end_time;


	const int size_x = 1024;
	const int size_y = 1024;
	const int size_x_overall = size_x * proc_size;
	const int size_y_overall = size_y * proc_size;
	      int num_guard_cells = 1;  // one on each end

	const int verbose = 0;
	      int num_timesteps = 25;

	short ** matrix_old;
	short ** matrix_new;

	int i,j,k;


	// // // // // Parse inputs // // // // //

	int proc_size_x = 2;
	int proc_size_y = 2;
	int printing_proc = 0;
	if (argc > 2)
	{
		char *end;
		proc_size_x = strtol(argv[1], &end, 10);
		proc_size_y = strtol(argv[2], &end, 10);

		if ((proc_size_x * proc_size_y) != proc_size)
		{
			printf("ERROR: proc_size_x (arg 1) and proc_size_y (arg 2) must equal total proc_size! \n");
			printf("proc_size_x = %d \n", proc_size_x);
			printf("proc_size_y = %d \n", proc_size_y);
			printf("proc_size   = %d \n", proc_size);
			return -1;
		}
	}
	if (argc > 3)
	{
		char *end;
		printing_proc = strtol(argv[3], &end, 10);
	}
	if (argc > 4)
	{
		char *end;
		num_timesteps = strtol(argv[4], &end, 10);
	}
	if (argc > 5)
	{
		char *end;
		num_guard_cells = strtol(argv[5], &end, 10);
	}

	// // // // // Set up system // // // // //

	struct topology my_topology = create_topology(proc_size_x, proc_size_y, proc_rank);

	if ((verbose) && proc_rank==printing_proc)
	{
		printf("rank = %d of %d \n", proc_rank, proc_size);
		printf("Processer %d is printing \n", printing_proc);		
	}

	matrix_old = allocate_matrix(matrix_old, size_x, size_y);
	matrix_new = allocate_matrix(matrix_new, size_x, size_y);

	int seed = proc_rank + 5; // avoid seeding one proc with 1 and another with 0 -- both will have the same seed
	init_matrix(matrix_old, size_x, size_y, num_guard_cells, seed);

	swap_rows_with_topology(    matrix_old, my_topology, size_x,  size_y,  proc_rank,  proc_size, num_guard_cells);
	swap_columns_with_topology( matrix_old, my_topology, size_x,  size_y,  proc_rank,  proc_size, num_guard_cells);


	// // // // // Evolve system // // // // //

	start_time = MPI_Wtime();
	int l = 0; // tracks timesteps since guard cells last updated
	for (k=0; k<num_timesteps; ++k)
	{
		if (verbose)
		{
			if (proc_rank==printing_proc)
			{
				printf("k=%d: \n", k);
			}

			io_MPI(matrix_old, proc_rank, printing_proc, size_x, size_y, num_guard_cells, proc_size_x, proc_size_y, my_topology, k);
		}

		// can complete `num_guard_cells' of updates before need to communicate again
		int tmp_guard_cells = l+1;  // boundaries slowly creep inwards
		dead_or_alive( matrix_old, matrix_new, size_x, size_y, tmp_guard_cells, verbose);
		swap((void*) &matrix_old, (void*) &matrix_new);

		++l;
		if (l==num_guard_cells)
		{
			swap_rows_with_topology(    matrix_old, my_topology, size_x, size_y, proc_rank, proc_size, num_guard_cells);
			swap_columns_with_topology( matrix_old, my_topology, size_x, size_y, proc_rank, proc_size, num_guard_cells);
			l = 0;
		}
	}
	end_time = MPI_Wtime();
	if (proc_rank==printing_proc)
	{
		double delta_time = end_time - start_time;
		int real_cells = (size_x-num_guard_cells)*(size_y-num_guard_cells);
		printf("total time: %e [s] \n", delta_time);
		printf("had %d real cells for %d timesteps \n", real_cells, num_timesteps);
		printf("average time per real cell per timestep: %e [s] \n", delta_time / (real_cells * num_timesteps));
		add_benchmark_result(proc_size, delta_time, real_cells, num_timesteps);
	}

	if (verbose)
	{
		if (proc_rank == printing_proc)
		{
			printf("Printing final matrix: \n");
		}
		io_MPI(matrix_old, proc_rank, printing_proc, size_x, size_y, num_guard_cells, proc_size_x, proc_size_y, my_topology, -1);	
	}


	free_matrix(matrix_old, size_x, size_y);
	free_matrix(matrix_new, size_x, size_y);

	MPI_Finalize();

	return 0;
}

