#ifndef _GET_PERFORMANCE_CONSTANTS_H
#define _GET_PERFORMANCE_CONSTANTS_H

	double get_comp_time(const int proc_rank, const int num_timesteps);
	double get_comm_time(const int proc_rank, const int proc_size, const long size);
	double get_filtered_average(const double time, const int proc_rank, const int proc_size, const int root_proc);



#endif
