#ifndef __DEFINED_DYNARRAY_H
#define __DEFINED_DYNARRAY_H

#include <stdlib.h>
#include <stdio.h>


extern void **dyn_arrayAlloc(int nrows, int ncols, size_t cell_size);
extern void dyn_arrayFree(void **dyn_array, int nrows);


extern void *dyn_1dArrayAlloc(int nrows, size_t cell_size);
extern void *dyn_1dArrayInsertRow(void *array, int old_nrows, int row_num, size_t cell_size);
extern void *dyn_1dArrayDeleteRow(void *array, int old_nrows, int row_num, size_t cell_size);
#define dyn_1dArrayFree(x) free(x)


#endif // __DEFINED_DYNARRAY_H
