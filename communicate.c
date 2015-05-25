#include <mpi.h>

#include "communicate.h"




void swap_rows(short ** matrix, int size_x, int size_y, int proc_rank, int proc_size)
{
	// assumes that your left/right boundary conditions are already 

	// to do: add information about topology (i.e. how to swap in UP DOWN directions)
	// 			- check out virtual topologies
	//			- create MPI_derived type for 2D domain decomposition
	int ierr;

	MPI_Request sendrecv_requests[4];
	MPI_Status  sendrecv_statuses[4];

	int source, send_tag;
	int dest,   recv_tag;

	// Directions and tags will all be from the SENDER'S frame of reference
	dest     = proc_rank - 1;
	dest 	 = wrap_ranks(dest, proc_size);
	send_tag = (proc_rank*4) + TOP;
	ierr	 = MPI_Isend(matrix[1], size_y, MPI_SHORT, dest, send_tag,
				MPI_COMM_WORLD, &sendrecv_requests[0]);

	dest     = proc_rank + 1;
	dest 	 = wrap_ranks(dest, proc_size);
	send_tag = (proc_rank*4) + BOTTOM;
	ierr	 = MPI_Isend(matrix[size_x - 2], size_y, MPI_SHORT, dest, send_tag,
				MPI_COMM_WORLD, &sendrecv_requests[1]);

	source   = proc_rank + 1;
	source 	 = wrap_ranks(source, proc_size);
	recv_tag = (source   *4) + TOP;
	ierr   	 = MPI_Irecv(matrix[size_x - 1], size_y, MPI_SHORT, source, recv_tag,
				MPI_COMM_WORLD, &sendrecv_requests[2]);

	source   = proc_rank - 1;
	source 	 = wrap_ranks(source, proc_size);
	recv_tag = (source   *4) + BOTTOM;
	ierr   	 = MPI_Irecv(matrix[0], size_y, MPI_SHORT, source, recv_tag,
				MPI_COMM_WORLD, &sendrecv_requests[3]);

    
	int num_requests = 4;
	ierr = MPI_Waitall(num_requests, sendrecv_requests, sendrecv_statuses );
}

int wrap_ranks(int proc_id, int proc_size)
{
	if (proc_id == -1)
	{
		proc_id = proc_size-1;
	}
	else if (proc_id == proc_size)
	{
		proc_id = 0;
	}
	return proc_id;
}