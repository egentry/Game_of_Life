#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>

#include "communicate.h"

MPI_Comm create_topology(const int proc_size_x, const int proc_size_y)
{
	// assumes 2 dimensions. If you want more than 2 dimenions, overload this function
	MPI_Comm topology;

	int ndims = 2;

	int periodic[2];
	int dims[2];
	int reorder = true;

	// periodicic boundary conditions
	periodic[0] = true;
	periodic[1] = true;

	dims[0] = proc_size_x;
	dims[1] = proc_size_y; 

	MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periodic, reorder, &topology);

	return topology;
}

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



void swap_columns_with_topology(short ** matrix, MPI_Comm topology, const int size_x, const int size_y, const int proc_rank, const int proc_size)
{
	int ierr;
	MPI_Request  all_requests[4];
	MPI_Status   all_statuses[4];

	int rank_left, rank_right;
	int send_tag_left, send_tag_right;
	int recv_tag_left, recv_tag_right;

	int direction = 1;
	int disp = 1; // i.e. what would the ranks be for a shift RIGHT (for disp > 0)
	MPI_Cart_shift(topology, direction, disp, &rank_left, &rank_right);


	// send + receive buffers, since columns are the slow axis in C
	short left_send[size_x];  
	short right_send[size_x]; 
	short left_recv[size_x];  
	short right_recv[size_x]; 

	int i;
	for (i=0; i<size_x; ++i)
	{
		left_send[i]  = matrix[i][1];
		right_send[i] = matrix[i][size_x - 2];
	}

	send_tag_left = proc_rank + 0*proc_size;
	ierr = MPI_Isend(left_send, size_y, MPI_SHORT, rank_left, send_tag_left,
						MPI_COMM_WORLD, &all_requests[0]);

	send_tag_right 	= proc_rank + 1*proc_size;
	ierr = MPI_Isend(right_send, size_y, MPI_SHORT, rank_right, send_tag_right,
						MPI_COMM_WORLD, &all_requests[1]);


	recv_tag_right = rank_left + 0*proc_size;
	ierr = MPI_Irecv(right_recv, size_y, MPI_SHORT, rank_right, recv_tag_right,
				MPI_COMM_WORLD, &all_requests[2]);

	recv_tag_left = rank_left + 1*proc_size;
	ierr = MPI_Irecv(left_recv, size_y, MPI_SHORT, rank_left, recv_tag_left,
				MPI_COMM_WORLD, &all_requests[3]);


	int num_requests = 4;
	ierr = MPI_Waitall(num_requests, all_requests, all_statuses );	

	for(i=0; i<size_x; ++i)
	{
		matrix[i][0] 			= left_recv[ i];
		matrix[i][size_x - 1] 	= right_recv[i];
	}

	return;
}

void swap_rows_with_topology(short ** matrix, MPI_Comm topology, const int size_x, const int size_y, const int proc_rank, const int proc_size)
{
	int ierr;
	MPI_Request  all_requests[4];
	MPI_Status   all_statuses[4];

	int rank_down, rank_up;
	int send_tag_down, send_tag_up;
	int recv_tag_down, recv_tag_up;

	int direction = 0;
	int disp = 1; // i.e. what would the ranks be for a shift UP (for disp > 0)
	MPI_Cart_shift(topology, direction, disp, &rank_down, &rank_up);

	send_tag_down = proc_rank + 0*proc_size;
	ierr = MPI_Isend(matrix[1], size_y, MPI_SHORT, rank_down, send_tag_down,
						MPI_COMM_WORLD, &all_requests[0]);

	send_tag_up = proc_rank + 1*proc_size;
	ierr = MPI_Isend(matrix[size_x - 2], size_y, MPI_SHORT, rank_up, send_tag_up,
						MPI_COMM_WORLD, &all_requests[1]);


	recv_tag_up = rank_down + 0*proc_size;
	ierr = MPI_Irecv(matrix[size_x - 1], size_y, MPI_SHORT, rank_up, recv_tag_up,
				MPI_COMM_WORLD, &all_requests[2]);

	recv_tag_down = rank_down + 1*proc_size;
	ierr = MPI_Irecv(matrix[0], size_y, MPI_SHORT, rank_down, recv_tag_down,
				MPI_COMM_WORLD, &all_requests[3]);

	int num_requests = 4;
	ierr = MPI_Waitall(num_requests, all_requests, all_statuses );	

	return;
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

