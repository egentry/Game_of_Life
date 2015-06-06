#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "communicate.h"



struct topology create_topology(const int proc_size_x, const int proc_size_y, const int proc_rank)
{
	// assumes 2 dimensions. If you want more than 2 dimenions, overload this function
	struct topology my_topology;

	MPI_Comm overall;

	int ndims = 2;

	int periodic[ndims];
	int dims[ndims];
	int reorder = true;
	// periodicic boundary conditions
	periodic[0] = true;
	periodic[1] = true;
	dims[0] = proc_size_x;
	dims[1] = proc_size_y; 
 	MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periodic, reorder, &overall);

	int my_coords[ndims];
 	MPI_Cart_coords(overall, proc_rank, ndims, my_coords);

	int ierr; 
	int remain_dims[ndims];
	remain_dims[0] = false;
	remain_dims[1] = true; // slice along the TRUE axis
	MPI_Comm row;
 	ierr = MPI_Cart_sub(overall, remain_dims, &row);

	remain_dims[0] = true;
	remain_dims[1] = false;
	MPI_Comm column;
	ierr = MPI_Cart_sub(overall, remain_dims, &column);

	my_topology.overall   = overall;
	my_topology.row 	  = row;
	my_topology.column    = column;
	my_topology.coords[0] = my_coords[0];
	my_topology.coords[1] = my_coords[1];

	return my_topology;
}

void swap_rows(short ** matrix, const int size_x, const int size_y, const int proc_rank, const int proc_size)
{
	// assumes that your left/right boundary conditions are already 

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



void swap_columns_with_topology(short ** matrix, struct topology my_topology, const int size_x, const int size_y, const int proc_rank, const int proc_size,
	const int num_guard_cells)
{
	int ierr;
	MPI_Request  all_requests[4];
	MPI_Status   all_statuses[4];

	int rank_left, rank_right;
	int send_tag_left, send_tag_right;
	int recv_tag_left, recv_tag_right;

	int direction = 1;
	int disp = 1; // i.e. what would the ranks be for a shift RIGHT (for disp > 0)
	MPI_Cart_shift(my_topology.overall, direction, disp, &rank_left, &rank_right);

	// send + receive buffers, since columns are the slow axis in C
	short left_send[size_x];  
	short right_send[size_x]; 
	short left_recv[size_x];  
	short right_recv[size_x]; 

	int i, j;
	for (j=0; j<num_guard_cells; ++j)
	{
		for (i=0; i<size_x; ++i)
		{
			 left_send[i] = matrix[i][num_guard_cells + j];
			right_send[i] = matrix[i][size_x - 1 - num_guard_cells - j];
		}

		send_tag_left = proc_rank + 0*proc_size;
		ierr = MPI_Isend(left_send, size_y, MPI_SHORT, rank_left, send_tag_left,
							MPI_COMM_WORLD, &all_requests[0]);
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_columns_with_toplogy with MPI_Isend (left) \n");
			printf("ierr = %d \n", ierr);
		}

		send_tag_right 	= proc_rank + 1*proc_size;
		ierr = MPI_Isend(right_send, size_y, MPI_SHORT, rank_right, send_tag_right,
							MPI_COMM_WORLD, &all_requests[1]);
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_columns_with_toplogy with MPI_Isend (right) \n");
			printf("ierr = %d \n", ierr);
		}


		recv_tag_right = rank_right + 0*proc_size;
		ierr = MPI_Irecv(right_recv, size_y, MPI_SHORT, rank_right, recv_tag_right,
					MPI_COMM_WORLD, &all_requests[2]);
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_columns_with_toplogy with MPI_Irecv (right) \n");
			printf("ierr = %d \n", ierr);
		}

		recv_tag_left = rank_left + 1*proc_size;
		ierr = MPI_Irecv(left_recv, size_y, MPI_SHORT, rank_left, recv_tag_left,
					MPI_COMM_WORLD, &all_requests[3]);
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_columns_with_toplogy with MPI_Irecv (left) \n");
			printf("ierr = %d \n", ierr);
		}


		int num_requests = 4;
		ierr = MPI_Waitall(num_requests, all_requests, all_statuses );	
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_columns_with_toplogy with MPI_Waitall \n");
			printf("ierr = %d \n", ierr);
		}
		for(i=0; i<size_x; ++i)
		{
			matrix[i][j] 			  = left_recv[ i];
			matrix[i][size_x - 1 - j] = right_recv[i];
		}
	}

	return;
}

void swap_rows_with_topology(short ** matrix, struct topology my_topology, const int size_x, const int size_y, const int proc_rank, const int proc_size,
	const int num_guard_cells)
{
	int ierr;
	MPI_Request  all_requests[4];
	MPI_Status   all_statuses[4];

	int rank_down, rank_up;
	int send_tag_down, send_tag_up;
	int recv_tag_down, recv_tag_up;

	int direction = 0;
	int disp = 1; // i.e. what would the ranks be for a shift UP (for disp > 0)
	MPI_Cart_shift(my_topology.overall, direction, disp, &rank_down, &rank_up);

	// printf("Down rank = %d, proc rank = %d, Up rank = %d \n", rank_down, proc_rank, rank_up);

	int j;
	for (j=0; j<num_guard_cells; ++j)
	{
		send_tag_down = proc_rank + 0*proc_size;
		ierr = MPI_Isend(matrix[num_guard_cells + j], size_y, MPI_SHORT, rank_down, send_tag_down,
							MPI_COMM_WORLD, &all_requests[0]);
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_rows_with_toplogy with MPI_Isend (down) \n");
			printf("ierr = %d \n", ierr);
		}

		send_tag_up = proc_rank + 1*proc_size;
		ierr = MPI_Isend(matrix[size_x - 1 - num_guard_cells - j], size_y, MPI_SHORT, rank_up, send_tag_up,
							MPI_COMM_WORLD, &all_requests[1]);
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_rows_with_toplogy with MPI_Isend (up) \n");
			printf("ierr = %d \n", ierr);
		}


		recv_tag_up = rank_up + 0*proc_size;
		ierr = MPI_Irecv(matrix[size_x - 1 - j], size_y, MPI_SHORT, rank_up, recv_tag_up,
					MPI_COMM_WORLD, &all_requests[2]);
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_rows_with_toplogy with MPI_Irecv (up) \n");
			printf("ierr = %d \n", ierr);
		}

		recv_tag_down = rank_down + 1*proc_size;
		ierr = MPI_Irecv(matrix[j], size_y, MPI_SHORT, rank_down, recv_tag_down,
					MPI_COMM_WORLD, &all_requests[3]);
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_rows_with_toplogy with MPI_Irecv (down) \n");
			printf("ierr = %d \n", ierr);
		}


		int num_requests = 4;
		ierr = MPI_Waitall(num_requests, all_requests, all_statuses );	
		if (ierr != MPI_SUCCESS)
		{
			printf("ERROR  in swap_rows_with_toplogy with MPI_Waitall \n");
			printf("ierr = %d \n", ierr);
		}
	}

	return;
}

int wrap_ranks(int proc_id, const int proc_size)
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

 
