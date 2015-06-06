#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "GoL.h"



int main(int argc, char *argv[] )
{
	const int verbose = 1;

	const int size_x = 1024;
	const int size_y = size_x;
	const int num_guard_cells = 1;  // one on each end
	short ** matrix_old;
	short ** matrix_new;

	int i,j;

	matrix_old = allocate_matrix(matrix_old, size_x, size_y);
	matrix_new = allocate_matrix(matrix_new, size_x, size_y);

	int seed = 0;
	init_matrix(matrix_old, size_x, size_y, num_guard_cells, seed);

	if (verbose)
	{
		printf("Printing initial matrix: \n");
		print_matrix(matrix_old, size_x, size_y);
	}

	dead_or_alive( matrix_old, matrix_new, size_x, size_y, num_guard_cells, verbose);
	enforce_boundary_conditions(matrix_new, size_x, size_y, num_guard_cells);
	swap((void*) &matrix_old, (void*) &matrix_new);

	if (verbose)
	{
		printf("Printing evolved matrix: \n");
		print_matrix(matrix_old, size_x, size_y);
	}


	free_matrix(matrix_old, size_x, size_y);
	free_matrix(matrix_new, size_x, size_y);

	return 0;

}

