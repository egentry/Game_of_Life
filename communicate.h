#ifndef _COMMUNICATE_H
#define _COMMUNICATE_H

	#include <mpi.h>
	struct topology
	{
		int coords[2];
		MPI_Comm overall;
		MPI_Comm row;
		MPI_Comm column;
	};

	enum {TOP, BOTTOM, LEFT, RIGHT};

	struct topology create_topology(const int proc_size_x, const int proc_size_y, const int proc_size);

	void swap_columns_with_topology(short ** matrix, struct topology my_topology, 
		const int size_x, const int size_y, const int proc_rank, const int proc_size, const int num_guard_cells);
	void swap_rows_with_topology(short ** matrix, struct topology my_topology, 
		const int size_x, const int size_y, const int proc_rank, const int proc_size, const int num_guard_cells);


	int wrap_ranks(int proc_id, const int proc_size);
	void swap_rows(short ** matrix, int size_x, int size_y, int proc_rank, int proc_size);

#endif