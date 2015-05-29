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



	const int size_x = 5;
	const int size_y = 5;
	const int size_x_overall = size_x * proc_size;
	const int size_y_overall = size_y * proc_size;
	int num_guard_cells = 1;  // one on each end
	short ** matrix_old;
	short ** matrix_new;

	int num_timesteps = 1;

	int i,j,k;

	int verbose = 1;

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

	// // // // // Set up system // // // // //

	MPI_Comm topology = create_topology(proc_size_x, proc_size_y);


	if (proc_rank!=printing_proc)
	{
		verbose = 0;
	}
	if ((verbose) && proc_rank==printing_proc)
	{
		printf("rank = %d of %d \n", proc_rank, proc_size);
		printf("Processer %d is printing \n", printing_proc);		
	}

	matrix_old = allocate_matrix(matrix_old, size_x, size_y);
	matrix_new = allocate_matrix(matrix_new, size_x, size_y);

	srand(proc_rank + 5); // avoid seeding with 1
	int seed = rand();
	init_matrix(matrix_old, size_x, size_y, num_guard_cells, seed);

	swap_rows_with_topology(    matrix_old, topology, size_x,  size_y,  proc_rank,  proc_size);
	swap_columns_with_topology( matrix_old, topology, size_x,  size_y,  proc_rank,  proc_size);

	if ((verbose) && (proc_rank==printing_proc))
	{
		printf("Printing initial matrix: \n");
		print_matrix(matrix_old, size_x, size_y);
	}



	// // // // // Evolve system // // // // //


	// dead_or_alive( matrix_old, matrix_new, size_x, size_y, num_guard_cells, verbose);
	// if ((verbose) && (proc_rank==printing_proc))
	// {
	// 	printf("Printing after dead_or_alive: \n");
	// 	print_matrix(matrix_new, size_x, size_y);
	// }
	// enforce_boundary_conditions_leftright(matrix_new, size_x, size_y, num_guard_cells);
	// swap_rows(matrix_new, size_x, size_y, proc_rank, proc_size);
	// if ((verbose) && (proc_rank==printing_proc))
	// {
	// 	printf("Printing after full time step: \n");
	// 	print_matrix(matrix_new, size_x, size_y);
	// }
	// swap((void*) &matrix_old, (void*) &matrix_new);



	for (k=0; k<num_timesteps; ++k)
	{
		dead_or_alive( matrix_old, matrix_new, size_x, size_y, num_guard_cells, verbose);
		swap_rows_with_topology(    matrix_new, topology, size_x,  size_y,  proc_rank,  proc_size);
		swap_columns_with_topology( matrix_new, topology, size_x,  size_y,  proc_rank,  proc_size);
		swap((void*) &matrix_old, (void*) &matrix_new);

		if (proc_rank==printing_proc)
		{
			printf("k=%d: \n", k);
			print_matrix(matrix_old, size_x, size_y);
		}
	}


	if (proc_rank==printing_proc)
	{
		printf("Printing after full evolution step: \n");
		print_matrix(matrix_new, size_x, size_y);
	}

	io_MPI(matrix_new, proc_rank, printing_proc, size_x, size_y, num_guard_cells, proc_size_x, proc_size_y, topology);



	free(matrix_old);
	free(matrix_new);

	MPI_Finalize();

	return 0;

}

