#ifndef _IO_H
#define _IO_H

	#include <mpi.h>

	void io_MPI(short ** matrix, int proc_rank, int printing_proc, int size_x, int size_y, int num_guard_cells, int proc_size_x, int proc_size_y, MPI_Comm topology);



#endif