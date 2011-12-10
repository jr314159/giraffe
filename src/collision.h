#ifndef __DEFINED_COLLISION_H
#define __DEFINED_COLLISION_H

#include "defs.h"
#include "map.h"
#include "object.h"
#include "timer.h"
#include "signal.h"
#include <math.h>

enum Rectangle_Sides {RECT_TOP, RECT_RIGHT, RECT_BOTTOM, RECT_LEFT};
enum Point_Collided_Values {L1P1, L1P2, L2P1, L2P2};

/* Coordinates are in integer values, but velocities and distances travelled
   are floating point values.  APPROX is a rounding method which adds a 
   threshold value, so that collision tests do not fail and allow objects to
   pass through boundaries because of rounding errors. */
#define EPSILON 1.0
#define APPROX(x) (rint(((x) > 0) ? ((x) + EPSILON) : (((x) < 0) ? ((x) - EPSILON) : (x))))

/* The maximum number of times the collision code will be allowed to loop: */
#define COLLISION_MAX_LOOPS 15

/* A structure containing basic information about a collision. */
typedef struct collision_struct
{

  /* When a collision is detected, it is detected between only two boundaries.
     The boundaries are then mapped into real coordinates (rather than relative
     to the object or tile positions) and stored here. */
  Bound a, b;

  /* Collision with an object or a tile */
  int type;

  /* Either a pointer to an object or the position of a tile */
  union
  {
    Object *obj;
    Point tile_pos;
  } other;

  /* Line collisions are detected by drawing paths that the endpoints travelled
     and checking for intersections.  If the collision was between two lines,
     this variable will remember which point on which line was detected to have
     collided. */
  int point_collided;

  /* We will collect these in a list: */
  struct collision_struct *next;

} Collision;

extern void col_doObjCollisions(Object *obj, Time dt);
extern void col_doTileCollisions(Object *obj, Time dt);
extern void col_doCollisions(Object *obj, Time dt);
extern Collision *col_getCollision(Object *obj, Object *other_obj, Point *tile_pos, Time dt);
extern Collision *col_listCollisions(Object *obj, Time dt);
extern Collision *col_listTileCollisions(Object *obj, Time dt);
extern Collision *col_listObjCollisions(Object *obj, Time dt);
extern void col_collisionResponse(Object *obj, Collision *coll_info, Time dt);
extern Vector normal(Line l);


#endif /* __DEFINED_COLLISION_H */


