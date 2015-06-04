#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "GoL.h"
#include "communicate.h"

#include "io.h"


void io_MPI(short ** matrix, int proc_rank, int printing_proc, int size_x, int size_y, int num_guard_cells, int proc_size_x, int proc_size_y, 
	struct topology my_topology, int timestep)
{
	// to do: 
	// 		adapt io routine so that the [0,0] processor doesn't need to hold all of the data from all the processors at the same time

	int ierr; 

	short  sub_matrix[size_x-2*num_guard_cells][(size_y-2*num_guard_cells)*proc_size_y];
	short full_matrix[(size_x-2*num_guard_cells)*proc_size_x][(size_y-2*num_guard_cells)*proc_size_y];

	int i,j;

	// Gather across rows
	for (i=1*num_guard_cells; i<size_x-1*num_guard_cells; ++i)
	{
		if(my_topology.coords[1] == 0)
		{
			ierr = MPI_Gather(    &(matrix[i][1*num_guard_cells]),size_y-2*num_guard_cells, MPI_SHORT, 
							  &(sub_matrix[i-1*num_guard_cells]), size_y-2*num_guard_cells, MPI_SHORT, 0, my_topology.row);
		}
		else
		{
			void * blank_ptr;
			ierr = MPI_Gather(&(matrix[i][1*num_guard_cells]), size_y-2*num_guard_cells, MPI_SHORT, 
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

	// // Print with print statements
	// if (proc_rank==0)
	// {
	// 	printf("matrix: \n");
	// 	for (j=0; j<(size_y-2*num_guard_cells)*proc_size_y; ++j)
	// 	{
	// 		int k;
	// 		for( k=0; k<(size_x-2*num_guard_cells)*proc_size_x; ++k)
	// 		{
	// 			printf("%d ", full_matrix[j][k]);	
	// 		}
	// 	printf("\n");
	// 	}
	// }

	// Print to file
	if (proc_rank==0)
	{
		char filename[256];
		if (timestep != -1)
		{
			sprintf(filename,"data/step_%04d.dat",timestep);
		}
		else
		{
			strcpy(filename, "data/final.dat");
		}
		FILE *file_pointer = fopen(filename, "w");


		printf("matrix: \n");
		for (j=0; j<(size_y-2*num_guard_cells)*proc_size_y; ++j)
		{
			int k;
			for( k=0; k<(size_x-2*num_guard_cells)*proc_size_x; ++k)
			{
				fprintf(file_pointer, "%d ", full_matrix[j][k]);	
			}
		fprintf(file_pointer, "\n");
		}

		fclose(file_pointer);
	}

	return;
}

void add_benchmark_result(int proc_size, double delta_time, int real_cells, int num_timesteps)
{
	// // Ultimately we'll want something like 'tau' for our performance model:
	// double tau = delta_time / (real_cells * num_timesteps);

	char filename[] = "data/benchmark_results.dat";
	FILE *file_pointer = fopen(filename, "a");
	fprintf(file_pointer, "%d %e %d %d \n", proc_size, delta_time, real_cells, num_timesteps);
	fclose(file_pointer);

	return;

}