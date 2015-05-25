#ifndef _COMMUNICATE_H
#define _COMMUNICATE_H

	enum {TOP, BOTTOM, LEFT, RIGHT};

	int wrap_ranks(int proc_id, int proc_size);
	void swap_rows(short ** matrix, int size_x, int size_y, int proc_rank, int proc_size);

#endif