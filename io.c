#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>
#include "GoL.h"

#include "io.h"


void io_MPI(short ** matrix, int proc_rank, int printing_proc, int size_x, int size_y, int num_guard_cells, int proc_size_x, int proc_size_y, MPI_Comm topology)
{

	int i,j;
	const int num_dims = 2;
	int my_coords[num_dims];

	MPI_Cart_coords(topology, proc_rank, num_dims, my_coords);

	short *line;

	int ierr; 
	const int remain_dims[num_dims] = {false, true}; // slice along the TRUE axis
	MPI_Comm my_row;
	ierr = MPI_Cart_sub(topology, remain_dims, &my_row);

	if (my_coords[1]==0)
	{
		// if I'm the 0th rank in my row:
		int tmp_coords[1];
		MPI_Cart_coords(my_row, proc_rank, 1, tmp_coords);

		printf("my rank =%d, my coords = [%d, %d] \n", proc_rank, my_coords[0], my_coords[1]);
		printf("my rank =%d, my row coords = [%d] \n", proc_rank, tmp_coords[0]);

		line = malloc((size_y-2)*proc_size_y * sizeof(short));
	}

	for (i=1; i<size_x-1; ++i)
	{
		ierr = MPI_Gather(&(matrix[i][1]), size_y-2, MPI_SHORT, 
						  line, size_y-2, MPI_SHORT, 0, my_row);

		if (proc_rank==0)
		{
			printf("line %d: ", i);
			for (j=0; j<(size_y-2)*proc_size_y; ++j)
			{
				printf("%d ", line[j]);
			}
			printf("\n");
		}
	}



	// now I need to collect these all to the io processor, and print into a file. Almost done!


	if (my_coords[1]==0)
	{
		free(line);
	}

	return;
}