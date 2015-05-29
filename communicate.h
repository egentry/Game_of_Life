#ifndef _COMMUNICATE_H
#define _COMMUNICATE_H

	#include <mpi.h>

	enum {TOP, BOTTOM, LEFT, RIGHT};

	MPI_Comm create_topology(const int proc_size_x, const int proc_size_y);
	void swap_columns_with_topology(short ** matrix, MPI_Comm topology, 
		const int size_x, const int size_y, const int proc_rank, const int proc_size);
	void swap_rows_with_topology(short ** matrix, MPI_Comm topology, 
		const int size_x, const int size_y, const int proc_rank, const int proc_size);


	int wrap_ranks(int proc_id, int proc_size);
	void swap_rows(short ** matrix, int size_x, int size_y, int proc_rank, int proc_size);

#endif