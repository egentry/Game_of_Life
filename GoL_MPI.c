#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "GoL.h"
#include "communicate.h"

#include <mpi.h>

int main(int argc, char *argv[])
{
	int proc_rank, proc_size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &proc_rank);
	MPI_Comm_size( MPI_COMM_WORLD, &proc_size);


	const int size_x = 6;
	const int size_y = 2*6;
	const int size_x_overall = size_x * proc_size;
	const int size_y_overall = size_x * proc_size;
	int num_guard_cells = 1;  // one on each end
	short ** matrix_old;
	short ** matrix_new;

	int num_timesteps = 5;

	int i,j,k;

	int verbose = 1;
	int printing_proc = 0;
	if (argc ==2)
	{
		char *end;
		printing_proc = strtol(argv[1], &end, 10);
		if ( (printing_proc<0) || (printing_proc>proc_size-1) )
		{
			printf("ERROR -- bad command line value for printing_proc\n");
			printf("Must be between 0 and proc_size-1 \n");
			printf("Recieved: %d \n", printing_proc);
	
			printf("argc = %d \n", argc);
			printf("argv = ");
			for (i=0; i<argc; ++i)
			{
				printf("%s ", argv[i]);
			}
			printf("\n");
			printf("printing_proc = %d \n", printing_proc);

			return -1;
		}
	}


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

	srand(proc_rank + 4); // avoid seeding with 1
	int seed = rand();
	init_matrix(matrix_old, size_x, size_y, num_guard_cells, seed);
	if ((verbose) && (proc_rank==printing_proc))
	{
		printf("Printing initial matrix: \n");
		print_matrix(matrix_old, size_x, size_y);
	}
	enforce_boundary_conditions_leftright(matrix_old, size_x, size_y, num_guard_cells);
	if ((verbose) && (proc_rank==printing_proc))
	{
		printf("Printing initial matrix after L/R boundary conditions: \n");
		print_matrix(matrix_old, size_x, size_y);
	}
	swap_rows(matrix_old, size_x, size_y, proc_rank, proc_size);
	if ((verbose) && (proc_rank==printing_proc))
	{
		printf("Printing initial matrix after T/B row swap: \n");
		print_matrix(matrix_old, size_x, size_y);
	}

	dead_or_alive( matrix_old, matrix_new, size_x, size_y, num_guard_cells, verbose);
	if ((verbose) && (proc_rank==printing_proc))
	{
		printf("Printing after dead_or_alive: \n");
		print_matrix(matrix_new, size_x, size_y);
	}
	enforce_boundary_conditions_leftright(matrix_new, size_x, size_y, num_guard_cells);
	swap_rows(matrix_new, size_x, size_y, proc_rank, proc_size);
	if ((verbose) && (proc_rank==printing_proc))
	{
		printf("Printing after full time step: \n");
		print_matrix(matrix_new, size_x, size_y);
	}
	swap((void*) &matrix_old, (void*) &matrix_new);



	for (k=0; k<num_timesteps; ++k)
	{
		dead_or_alive( matrix_old, matrix_new, size_x, size_y, num_guard_cells, verbose);
		enforce_boundary_conditions_leftright(matrix_new, size_x, size_y, num_guard_cells);
		swap_rows(matrix_new, size_x, size_y, proc_rank, proc_size);
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



	free(matrix_old);
	free(matrix_new);

	MPI_Finalize();

	return 0;

}

