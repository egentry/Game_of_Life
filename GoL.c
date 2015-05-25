#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "GoL.h"

short ** allocate_matrix(short ** matrix, int size_x, int size_y)
{
	int i;
	matrix = (short**) malloc(size_x * sizeof(short*));
	for (i=0; i<size_x; ++i)
	{
		matrix[i] = (short*) malloc(size_y * sizeof(short));
	}

	return matrix;
}

void init_matrix(short ** matrix, int size_x, int size_y, int num_guard_cells, int seed)
{

	int i, j;

	srand(seed);
	for (i=num_guard_cells; i<size_x-num_guard_cells; ++i)
	{
		for (j=num_guard_cells; j<size_y-num_guard_cells; ++j)
		{
			matrix[i][j] = rand()%2;
		}
	}
	enforce_boundary_conditions(matrix, size_x, size_y, num_guard_cells);

	return;
}
	 

void print_matrix(short ** matrix, int size_x, int size_y)
{
	int i,j;
	for (i=0; i<size_x; ++i)
	{
		for (j=0; j<size_y; ++j)
		{
			printf("%d ", matrix[i][j]);
		}
		printf("\n");
	}	

	return;	
}

void dead_or_alive(short ** matrix_old, short ** matrix_new, 
	int size_x, int size_y, int num_guard_cells, int verbose)
{
	int i,j;

	for (i=num_guard_cells; i<size_x-num_guard_cells; ++i)
	{
		for (j=num_guard_cells; j<size_y-num_guard_cells; ++j)
		{
			matrix_new[i][j] = 0;

			// adjacent conditions
			matrix_new[i][j] += matrix_old[i-1][j];
			matrix_new[i][j] += matrix_old[i][j-1];
			matrix_new[i][j] += matrix_old[i+1][j];
			matrix_new[i][j] += matrix_old[i][j+1];


			// corner conditions
			matrix_new[i][j] += matrix_old[i-1][j-1];
			matrix_new[i][j] += matrix_old[i+1][j+1];
			matrix_new[i][j] += matrix_old[i-1][j+1];
			matrix_new[i][j] += matrix_old[i+1][j-1];

		}
	}

	if (verbose)
	{
		printf("neighbors matrix: \n");
		print_matrix(matrix_new, size_x, size_y);
	}

	//  update if cells are alive or dead
	for (i=0; i<size_x; ++i)
	{	
		for (j=0; j<size_y; ++j)
		{
			if (matrix_new[i][j] ==3)
			{
				matrix_new[i][j] = ALIVE;
			}
			else if (matrix_new[i][j]==2)
			{
				matrix_new[i][j] = matrix_old[i][j];
			}
			else
			{
				matrix_new[i][j] = DEAD;
			}
		}
	}

	return;
}

void enforce_boundary_conditions(short ** matrix, int size_x, int size_y, int num_guard_cells)
{
	// enforces PERIODIC boundary conditions, including across corners

	int i,j;
	const int inner_valid_edge_x = num_guard_cells;
	const int outer_valid_edge_x = size_x - num_guard_cells - 1;
	const int inner_valid_edge_y = num_guard_cells;
	const int outer_valid_edge_y = size_y - num_guard_cells - 1;


	// edges
	for (i=num_guard_cells; i<size_x-num_guard_cells; ++i)
	{
		matrix[i][inner_valid_edge_y-1] = matrix[i][outer_valid_edge_y];
		matrix[i][outer_valid_edge_y+1] = matrix[i][inner_valid_edge_y];
	}

	for (j=num_guard_cells; j<size_y-num_guard_cells; ++j)
	{
		matrix[inner_valid_edge_x-1][j] = matrix[outer_valid_edge_x][j];
		matrix[outer_valid_edge_x+1][j] = matrix[inner_valid_edge_x][j];
	}


	// corners
	matrix[inner_valid_edge_x-1][inner_valid_edge_y-1] = matrix[outer_valid_edge_x][outer_valid_edge_y];
	matrix[inner_valid_edge_x-1][outer_valid_edge_y+1] = matrix[outer_valid_edge_x][inner_valid_edge_y];
	matrix[outer_valid_edge_x+1][inner_valid_edge_y-1] = matrix[inner_valid_edge_x][outer_valid_edge_y];
	matrix[outer_valid_edge_x+1][outer_valid_edge_y+1] = matrix[inner_valid_edge_x][inner_valid_edge_y];

	return;
}


void swap(void **a, void **b)
{
    // must be dynamically allocated array
    void *tmp = *a;

    *a = *b;
    *b = tmp;

    return;
}