/* defs.h
 *
 * Definitions known by all modules.
 *
 */

#ifndef __DEFINED_DEFS_H
#define __DEFINED_DEFS_H

/* Allocate a certain amount of memory to ptr, exit on failure: */
#define MALLOC(ptr, size) do { \
if(((ptr) = (void *) malloc(size)) == NULL) \
{ \
  fprintf(stderr, "Unable to allocate memory.\n"); \
  exit(0); \
} \
} while(0)

/* Limits a signed number to under a certain absolute value: */
#define LIMIT(x, lim) (((x) >= 0) ? (((x) > (lim)) ? (lim) : (x)) : (((x) < (-lim)) ? (-lim) : (x)))

/* Decrease the absolute value of a number by a certain amount, but do not
   cross zero: */
#define DECREASE(x, d) (((x) >= 0) ? (((x) >= (d)) ? ((x) - (d)) : 0) : (((x) <= (-d)) ? ((x) + (d)) : 0))

enum bound_types {LINE, RECT, CIRCLE};

typedef struct {
  float x, y;
} Vector;

typedef Vector Velocity;

typedef struct {
  int x, y;
} Point;

typedef struct {
  Point p1, p2;
} Line;

/* Note that p2 must be a point below and to the right of p1, otherwise
   the rectangle will be "inside out" and produce strange results: */
typedef struct {
  Point p1, p2;
} Rect;

typedef struct {
  Point p;
  int r;
} Circle;

typedef struct {
  int r;
  int g;
  int b;
} Color;

typedef struct bound_struct
{
  int type; // what type of boundary
  union
  {
    Line line;
    Rect rect;
    Circle circle;
  } b;
  struct bound_struct *next; // pointer to next boundary in the list
} Bound;

enum thing_types {OBJ_TYPE, TILE_TYPE};

#endif // __DEFINED_DEFS_H
