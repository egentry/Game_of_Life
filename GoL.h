#ifndef _GOL_H
#define _GOL_H

enum {DEAD, ALIVE};


short ** allocate_matrix(short ** matrix, int size_x, int size_y);
void         free_matrix(short ** matrix, int size_x, int size_y);
void         init_matrix(short ** matrix, int size_x, int size_y, int num_guard_cells, int seed);
void        print_matrix(short ** matrix, int size_x, int size_y);

void enforce_boundary_conditions(short ** matrix, int size_x, int size_y, int num_guard_cells);
void enforce_boundary_conditions_leftright(short ** matrix, int size_x, int size_y, int num_guard_cells);
void dead_or_alive(short ** matrix, short ** matrix_new, int size_x, int size_y, int num_guard_cells, int verbose);

void swap(void **a, void **b);


#endif 
