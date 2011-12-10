#include "dynarray.h"

/* dyn_arrayAlloc

   dynamically allocate an array of given dimensions with cells of given
   size.

   For reference, while I understand clearly, here's a little explanation of
   how dynamic arrays work:  First, you create a column of pointers and get
   a pointer to the beginning of that column.  Each pointer in this column then
   can be referenced by p[i] where p is the pointer to the beginning of the
   column (p[0] = p).  Then for each pointer in this column, you create a 
   row of whatever items you want your array to contain, and get a pointer
   to the beginning of that row.  So, any item in this array can now be found
   by first looking up the appropriate row, and then moving down that row
   to the appropriate column: p[row][col].  Here's a little diagram of what
   I picture it looking like:

       **a ----> o                
                 |       a[row][col]  
                 |      /
                 |     L
        *a ----> o----o---
                 |

   Yay for ascii diagrams.  Anyway, this accurately simulates a real array in
   that the lookup is done by [row][col] rather than [x][y].  Ultimately this
   doesn't matter; if what you want is a coordinate map that you can lookup
   with [x][y], when you create the array you can just give it dimensions of
   x rows and y columns.  To visualize what that will look like, you'll
   essentially be turning your map 90 degrees so that it is sitting on its side
   but once the array is allocated this way you can just think of it in terms 
   of x,y coordinates.

   When this function is called, you should cast the return value into the
   correct pointer type.  For example, if your array is going to contain
   pointers to type Tile:

   Tile ***map_array = (Tile ***) dyn_arrayAlloc(NROWS, NCOLS, sizeof(Tile *));

   Then when you free it, cast the type back to void pointers:

   dyn_arrayFree((void **) map_array);

*/
void **
dyn_arrayAlloc(int nrows, int ncols, size_t cell_size)
{
  
  int i;

  void **dyn_array;


  // Allocate a column of void pointers, returning a void pointer to the
  // column:

  dyn_array = (void **) dyn_1dArrayAlloc(nrows, sizeof(void *));

  // For each row, allocate a row of cells which can contain objects of
  // the size cell_size, and return a pointer to the row:
  for (i = 0; i < nrows; i++)
    {
      dyn_array[i] = (void *) dyn_1dArrayAlloc(ncols, cell_size);
    }

  return dyn_array;
}

/* dyn_arrayFree

   the inverse of dyn_arrayAlloc

*/
void
dyn_arrayFree(void **dyn_array, int nrows)
{

  int i;

  for (i = nrows - 1; i >= 0; i--)
    {
      dyn_1dArrayFree(dyn_array[i]);
    }
  dyn_1dArrayFree(dyn_array);
}

/* dyn_1dArrayAlloc

   dynamically allocates a 1d array

*/
void *
dyn_1dArrayAlloc(int nrows, size_t cell_size)
{
  void *dyn_1d_array;

  // Use calloc here because it sets the array values to NULL, whereas they
  // contain garbage if just malloc is used:
  
  if ((dyn_1d_array = calloc(nrows, cell_size)) == NULL)
    {
      fprintf(stderr, "Unable to allocate memory.\n");
      exit(0);
    }

  return dyn_1d_array;
}

/* dyn_1dArrayInsertRow

   Reallocate a 1 dimensional array, inserting a new row at a given slot.
   It is the responsibility of the programmer to know the current number of
   rows of the array and the cell size.
*/
void *
dyn_1dArrayInsertRow(void *array, int nrows, int row_num, size_t cell_size)
{
  
  int row;

  /* Append one more row */
  if ((array = realloc(array, (nrows + 1) * cell_size)) == NULL)
    {
      fprintf(stderr, "Unable to allocate memory.\n");
      exit(0);
    }
  
  /* Start at the last row, shifting the rows forward until we find the
     row that we wanted to insert a new row at */
  /* This is some sort of complicated pointer arithmetic.  Pointer arithmetic
     doesn't work on void pointers, but if we cast the array to (char *) we
     can find the right addresses.  */
  for (row = nrows; row > row_num; row--)
    {
      void *src, *dest;
      dest = (char *)array + row * (cell_size / sizeof(char));
      src = (char *)array + (row - 1) * (cell_size / sizeof(char));
      memcpy(dest, src, cell_size);
    }

  /* Initialize this new row to zero */
  memset((char *)array + row_num * (cell_size / sizeof(char)), 0, cell_size);

  return array;
}

/* dyn_1dArrayDeleteRow
   The reverse of insert row.
 */
void *
dyn_1dArrayDeleteRow(void *array, int nrows, int row_num, size_t cell_size)
{

  /* Only do this if there is more than 1 row. */
  if (nrows > 1)
    {
      int row;
      
  /* Shift the rows back */
      for (row = row_num; row < nrows - 1; row++)
	{
	  void *src, *dest;
	  src = (char *)array + (row + 1) * (cell_size / sizeof(char));
	  dest = (char *)array + row * (cell_size / sizeof(char));
	  memcpy(dest, src, cell_size);
	}
      
      /* Reallocate the array */
      if ((array = realloc(array, (nrows - 1) * cell_size)) == NULL)
	{
	  fprintf(stderr, "Unable to allocate memory.\n");
      exit(0);
	}
      return array;
    }

  else
    {
      free(array);
      return NULL;
    }
}

/* dyn_1dArrayFree

   This is a macro defined in the header file!

*/
