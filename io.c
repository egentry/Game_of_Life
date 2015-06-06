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

	short  sub_matrix[ size_x-2*num_guard_cells ][(size_y-2*num_guard_cells)*proc_size_y];

	int i,j;

	// Gather across rows
	for (i=num_guard_cells; i<size_x-num_guard_cells; ++i)
	{
		if(my_topology.coords[1] == 0)
		{
			ierr = MPI_Gather(    &(matrix[i][num_guard_cells]),size_y-2*num_guard_cells, MPI_SHORT, 
							  &(sub_matrix[i-num_guard_cells]), size_y-2*num_guard_cells, MPI_SHORT, 0, my_topology.row);
		}
		else
		{
			void * blank_ptr;
			ierr = MPI_Gather(&(matrix[i][num_guard_cells]), size_y-2*num_guard_cells, MPI_SHORT, 
							  blank_ptr, size_y-2*num_guard_cells, MPI_SHORT, 0, my_topology.row);
		}
	}

	// Gather across columns
	if( (my_topology.coords[0]==0) && (my_topology.coords[1]==0) )
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

		for (j=0; j<(size_x-2*num_guard_cells); ++j)
		{
			int k;
			for( k=0; k<(size_y-2*num_guard_cells)*proc_size_y; ++k)
			{
				fprintf(file_pointer, "%d ", sub_matrix[j][k]);	
			}
			fprintf(file_pointer, "\n");
		}

		for(i=1; i<proc_size_x; ++i)
		{
			MPI_Status recv_status;
			int count 	 = proc_size_y*(size_y-2*num_guard_cells)*(size_x-2*num_guard_cells);
			int source	 = i;
			int tag 	 = i;

			ierr = MPI_Recv(sub_matrix, count, MPI_SHORT, source, tag, my_topology.column, &recv_status );

			for (j=0; j<(size_x-2*num_guard_cells); ++j)
			{
				int k;
				for( k=0; k<(size_y-2*num_guard_cells)*proc_size_y; ++k)
				{
					fprintf(file_pointer, "%d ", sub_matrix[j][k]);	
				}
				fprintf(file_pointer, "\n");
			}
		}
		fclose(file_pointer);
	}
	else if( (my_topology.coords[0]!=0) && (my_topology.coords[1]==0) )
	{
		int count 	= proc_size_y*(size_y-2*num_guard_cells)*(size_x-2*num_guard_cells);
		int dest 	= 0;
		int tag 	= my_topology.coords[0];

		ierr = MPI_Send(sub_matrix, count, MPI_SHORT, dest, tag, my_topology.column);
	}




	return;
}

void add_benchmark_result(int proc_size, double delta_time, int real_cells, int num_timesteps)
{
	// // Ultimately we'll want something like 'tau' for our performance model:
	// double tau = delta_time / (real_cells * num_timesteps);

	char filename[] = "data/benchmark_results.dat";
	FILE *file_pointer = fopen(filename, "a");
	fprintf(file_pointer, "%d         %e   %d        %d \n", proc_size, delta_time, real_cells, num_timesteps);
	fclose(file_pointer);

	return;

}
