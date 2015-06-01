#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>
#include "GoL.h"
#include "communicate.h"

#include "io.h"


void io_MPI(short ** matrix, int proc_rank, int printing_proc, int size_x, int size_y, int num_guard_cells, int proc_size_x, int proc_size_y, struct topology my_topology)
{

	int ierr; 

	short  sub_matrix[size_x-2*num_guard_cells][(size_y-2*num_guard_cells)*proc_size_y];
	short full_matrix[(size_x-2*num_guard_cells)*proc_size_x][(size_y-2*num_guard_cells)*proc_size_y];

	int i,j;

	// Gather across rows
	for (i=1*num_guard_cells; i<size_x-1*num_guard_cells; ++i)
	{
		if(my_topology.coords[1] == 0)
		{

			ierr = MPI_Gather(&(matrix[i][1]), size_y-2*num_guard_cells, MPI_SHORT, 
							  &(sub_matrix[i-1]), size_y-2*num_guard_cells, MPI_SHORT, 0, my_topology.row);
		}
		else
		{
			void * blank_ptr;
			ierr = MPI_Gather(&(matrix[i][1]), size_y-2*num_guard_cells, MPI_SHORT, 
							  blank_ptr, size_y-2*num_guard_cells, MPI_SHORT, 0, my_topology.row);
		}
	}

	// Gather across columns
	for (i=0; i<proc_size_x; ++i)
	{
		if( (my_topology.coords[0]==0) && (my_topology.coords[1]==0) )
		{
			ierr = MPI_Gather(sub_matrix, 
				proc_size_y*(size_y-2*num_guard_cells)*(size_x-2*num_guard_cells),
				MPI_SHORT, full_matrix, 
				proc_size_y*(size_y-2*num_guard_cells)*(size_x-2*num_guard_cells), 
				MPI_SHORT, 0, my_topology.column);
		}
		else if( (my_topology.coords[0]!=0) && (my_topology.coords[1]==0) )
		{
			void * blank_ptr;
			ierr = MPI_Gather(sub_matrix, 
				proc_size_y*(size_y-2*num_guard_cells)*(size_x-2*num_guard_cells),
				MPI_SHORT, blank_ptr, 
				proc_size_y*(size_y-2*num_guard_cells)*(size_x-2*num_guard_cells), 
				MPI_SHORT, 0, my_topology.column);
		}
	}

	// Print
	if (proc_rank==0)
	{
		printf("matrix: \n");
		for (j=0; j<(size_y-2)*proc_size_y; ++j)
		{
			int k;
			for( k=0; k<(size_x-2)*proc_size_x; ++k)
			{
				printf("%d ", full_matrix[j][k]);	
			}
		printf("\n");
		}
	}

	return;
}