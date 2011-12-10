#include "collision.h"

/* Private function prototypes */
static int ccw(Point p0, Point p1, Point p2);
static int intersect(Line l1, Line l2);
static int parallel(Line l1, Line l2);
static Point intersection(Line l1, Line l2);
static float length(Line l);
static float magnitude(Vector v);
static Vector normalize(Vector v);
static float dot(Vector v1, Vector v2);
static Collision *newCollision(Bound *a, Bound *b, Object *other_obj, Point *tile_pos, int point_collided);
static Point closestPointOnLine(Point p, Line l);
static int getLineCollision(Line mov_line, Line stat_line, Velocity v, Time dt);
static int testRectOverlap(Rect r1, Rect r2);
static Collision *findSolidCollision(Object *obj, Collision *c);


/* ccw
 * Takes three ordered points, determines if motion from p0 to p1 to p2 is
 * counterclockwise.  Returns 1 if counterclockwise, -1 if clockwise. If the
 * points are collinear, it returns -1 if p0 is between p2 and p1, 0 if p2 is
 * between p0 and p1, and 1 if p1 is between p0 and p2.
 * (From Sedgewick, "Algorithms in C++", p. 350)
 */
 
int ccw(Point p0, Point p1, Point p2)
{
  int dx1, dx2, dy1, dy2;
 
  // Find dx and dy for comparing slopes:
  dx1 = p1.x - p0.x; dy1 = p1.y - p0.y;
  dx2 = p2.x - p0.x; dy2 = p2.y - p0.y;
                                                                                
  // Compare the slopes.  This is equivalent to division, but eliminates the
  // possibility of a divide by 0 error.  Also, slopes need not be positive
  // for this to work correctly:
  if (dx1*dy2 > dy1*dx2) return +1;
  if (dx1*dy2 < dy1*dx2) return -1;
  // The routine has not returned, so the points are collinear.
  if ((dx1*dx2 < 0) || (dy1*dy2 < 0)) return -1;
  if ((dx1*dx1+dy1*dy1) < (dx2*dx2+dy2*dy2)) return +1;
  return 0;
}
 
/* intersect
 * Takes two line segments, returns 1 if they intersect.
 * (From Sedgewick, "Algorithms in C++", p. 351)
 */
 
int intersect(Line l1, Line l2)
{
 
  // The following expression evaluates true if both endpoints of each line
  // are on different sides of the other:
  return ((ccw(l1.p1, l1.p2, l2.p1)*ccw(l1.p1, l1.p2, l2.p2)) <= 0)
    && ((ccw(l2.p1, l2.p2, l1.p1)*ccw(l2.p1, l2.p2, l1.p2)) <= 0);
}

/* parallel
   Returns true if two lines are parallel.
*/
int parallel(Line l1, Line l2)
{
  /* dy1/dx1 == dy2/dx2
     -> dy1 * dx2 == dy2 * dx1 
  */
  return ((l1.p2.y - l1.p1.y) * (l2.p2.x - l2.p1.x) == (l2.p2.y - l2.p1.y) * (l1.p2.x - l1.p1.x));
}

/* intersection
   Finds the point of intersection of two lines which are known to be
   intersecting.
*/

Point intersection(Line l1, Line l2)
{

  Point p;

  /* y = mx + b */
  float l1_dy, l1_dx, l2_dy, l2_dx, l1_m, l2_m, l1_b, l2_b;

  /* In the case that the lines are parallel, return the endpoint on l1 */
  if (parallel(l1, l2))
    {
      //printf("Finding intersection for parallel lines...\n");
      return ((l1.p2.x < l2.p1.x && l1.p2.x < l2.p2.x) ||
	      (l1.p2.x > l2.p2.x && l1.p2.x > l2.p1.x) ||
	      (l1.p2.y > l2.p1.y && l1.p2.y > l2.p2.y) ||
	      (l1.p2.y < l2.p1.y && l1.p2.y < l2.p2.y)) ? l1.p1 : l1.p2;
    }

  l1_dy = l1.p2.y - l1.p1.y;
  l1_dx = l1.p2.x - l1.p1.x;
  l2_dy = l2.p2.y - l2.p1.y;
  l2_dx = l2.p2.x - l2.p1.x;

  /* In case one of the lines is vertical, prevent divide by 0 errors: */
  if (l1_dx == 0)
    {
      /* If the other line is horizontal, solve quickly: */
      if (l2_dy == 0)
	{
	  p.x = l1.p1.x;
	  p.y = l2.p1.y;
	  return p;
	}
	
      p.x = l1.p2.x;

      l2_m = (l2.p2.y - l2.p1.y) / (l2.p2.x - l2.p1.x);
      l2_b = l2.p1.y - l2_m * l2.p1.x;

      p.y = rint(l2_m * p.x + l2_b);
      return p;
    }
  else if (l2_dx == 0)
    {
      /* If the other line is horizontal, solve quickly: */
      if (l1_dy == 0)
	{
	  p.x = l2.p1.x;
	  p.y = l1.p1.y;
	  return p;
	}

      p.x = l2.p2.x;

      l1_m = (l1.p2.y - l1.p1.y) / (l1.p2.x - l1.p1.x);
      l1_b = l1.p1.y - l1_m * l1.p1.x;

      p.y = rint(l1_m * p.x + l1_b);
      return p;
    }

  /* Same if one of the lines is horizontal: */
  if (l1_dy == 0)
    {
      /* If the other line is vertical, solve quickly: */
      if (l2_dx == 0) 
	{
	  p.x = l2.p1.x;
	  p.y = l1.p1.y;
	  return p;
	}

      p.y = l1.p2.y;

      l2_m = l2_dy / l2_dx;
      l2_b = l2.p1.y - l2_m * l2.p1.x;

      p.x = rint((p.y - l2_b) / l2_m);
      return p;
    }
  else if (l2_dy == 0)
    {

      /* If the other line is vertical, solve quickly: */
      if (l1_dx == 0) 
	{
	  p.x = l1.p1.x;
	  p.y = l2.p1.y;
	  return p;
	}
 
      p.y = l2.p2.y;

      l1_m = l1_dy / l1_dx;
      l1_b = l1.p1.y - l1_m * l1.p1.x;

      p.x = rint((p.y - l1_b) / l1_m);
      return p;
    }

  /* Neither line is horizontal or vertical, solve normally: */

  l1_m = l1_dy / l1_dx;
  l2_m = l2_dy / l2_dx;


  /* b = y - mx */
  l1_b = l1.p1.y - l1_m * l1.p1.x;
  l2_b = l2.p1.y - l2_m * l2.p1.x;

  p.x = rint((l2_b - l1_b) / (l1_m - l2_m));
  p.y = rint((l1_b - l2_b * (l1_m / l2_m)) / (1 - (l1_m / l2_m)));

  return p;

}

/* length
   Gets the length of a line.
*/
float
length(Line l)
{
  return sqrtf((l.p2.x - l.p1.x) * (l.p2.x - l.p1.x) + (l.p2.y - l.p1.y) * (l.p2.y - l.p1.y));
}

/* magnitude
   Returns the magnitude of a vector.
*/
float
magnitude(Vector v)
{
  return sqrtf(v.x * v.x + v.y * v.y);
}

/* normalize
   Normalizes a vector.
*/
Vector
normalize(Vector v)
{
  float m = magnitude(v);
  v.x = v.x / m;
  v.y = v.y / m;
  return v;
}

/* normal
   Returns the normal of a line.
*/
Vector
normal(Line l)
{
  Vector n;
  n.x = (l.p2.y - l.p1.y);
  n.y = (l.p1.x - l.p2.x);
  return (normalize(n));
}

/* dot
   Returns the dot product of two vectors:
*/
float
dot(Vector v1, Vector v2)
{
  return ((v1.x * v2.x) + (v1.y * v2.y));
}

/* newCollision
   Allocates and initializes a collision 
*/
Collision *
newCollision(Bound *a, Bound *b, Object *other_obj, Point *tile_pos, int point_collided)
{
  Collision *new_collision;
  MALLOC(new_collision, sizeof(Collision));

  new_collision->a = *a;
  new_collision->b = *b;

  /* Determine the type by what parameters were provided */
  new_collision->type = (other_obj == NULL) ? TILE_TYPE : OBJ_TYPE;
  if (new_collision->type == TILE_TYPE)
    {
      new_collision->other.tile_pos = *tile_pos;
    }
  else
    {
      new_collision->other.obj = other_obj;
    }

  new_collision->point_collided = point_collided;
  new_collision->next = NULL;

  return new_collision;
}

/* closestPointOnLine
   Given a point and a line segment, find the closest point on that line
   segment.

   Based on code at http://www.gamedev.net/reference/articles/article1026.asp
   and
   http://astronomy.swin.edu.au/~pbourke/geometry/pointline/source.c
*/
Point
closestPointOnLine(Point p, Line l)
{

  Point a;
  Vector c, V;
  float line_mag;
  float t;

  c.x = p.x - l.p1.x;
  c.y = p.y - l.p1.y;

  V.x = l.p2.x - l.p1.x;
  V.y = l.p2.y - l.p1.y;
  V = normalize(V);

  line_mag = length(l);
  t = dot(V, c);


  if (t < 0) return l.p1;
  if (t > line_mag) return l.p2;

  a.x = rint(l.p1.x + V.x * t);
  a.y = rint(l.p1.y + V.y * t);

  return a;
}

/* getLineCollision

   Given two lines, one stationary and one moving, check if they collided.
   This works by looking at the path travelled by each endpoint of the moving
   line and seeing if that path intersects with the stationary line.  If that
   doesn't detect a collision, do the same test for the endpoints of the
   stationary line.  I think this should detect all possible collisions between
   two lines of any length if one of them is moving.  The values returned
   indicate which point on which line was found to be colliding.

*/
int
getLineCollision(Line mov_line, Line stat_line, Velocity v, Time dt)
{

  Line path;
  
  /* Exit now if the dot product of the two lines' normals is >= 0 */
  if (dot(normal(mov_line), normal(stat_line)) >= 0) return -1;

  /* Create a line representing the path that p1 on the moving line
     travelled: */
  path.p2 = mov_line.p1;
  path.p1.x = path.p2.x - APPROX(v.x * dt);
  path.p1.y = path.p2.y - APPROX(v.y * dt);
  
  if (intersect(path, stat_line)) return L1P1;
  
  /* No collision for p1, try p2: */
  path.p2 = mov_line.p2;
  path.p1.x = path.p2.x - APPROX(v.x * dt);
  path.p1.y = path.p2.y - APPROX(v.y * dt);
  
  if (intersect(path, stat_line)) return L1P2;
  
  /* Pretend the moving line was stationary and the stationary line moved,
     and test for intersection with the stationary line's paths: */
  path.p1 = stat_line.p1;
  path.p2.x = path.p1.x + APPROX(v.x * dt);
  path.p2.y = path.p1.y + APPROX(v.y * dt);
  
  if (intersect(path, mov_line)) return L2P1;
  
  /* Try p2: */
  path.p1 = stat_line.p2;
  path.p2.x = path.p1.x + APPROX(v.x * dt);
  path.p2.y = path.p1.y + APPROX(v.y * dt);
  
  if (intersect(path, mov_line)) return L2P2;
  
  /* If we still haven't found a collision, return -1 for failure: */

  return -1;

}

/* testRectOverlap
   Returns true if two rectangles are overlapping:
*/
int
testRectOverlap(Rect r1, Rect r2)
{
  return (!(r1.p2.x < r2.p1.x || r1.p2.y < r2.p1.y || r1.p1.x > r2.p2.x || r1.p1.y > r2.p2.y));

}

/* col_GetCollision
   Given an object and a tile or an object, find a collision.

   Returns NULL if no collisions were found.
*/
Collision *
col_getCollision(Object *obj, Object *other_obj, Point *tile_pos, Time dt) 
{

  Bound *a_b, *a_bounds, *b_bounds;
  Point a_top_left, b_top_left;
  int point_collided = -1;
  int type;
  Velocity a_v = obj_getObjVel(obj);

  a_top_left = obj_getObjTopLeft(obj);
  a_bounds = obj_getObjBounds(obj);

  /* Collision with a tile or an object? */
  if (other_obj != NULL)
    {
      type = OBJ_TYPE;
      /* If the collision is between two moving objects, 
	 simplify the problem to
	 a collision between a moving and a 
	 static object by using relative
	 velocity: */
      a_v.x -= obj_getObjVel(other_obj).x;
      a_v.y -= obj_getObjVel(other_obj).y;
      
      b_bounds = obj_getObjBounds(other_obj);
      b_top_left = obj_getObjTopLeft(other_obj);
    }
  else
    {
      type = TILE_TYPE;
      /* The tile is necessarily in the same layer as the object.  No
	 collisions with things in other layers. */
      b_bounds = map_getTileBounds(obj_getObjLayer(obj), tile_pos->x, tile_pos->y);
      b_top_left.x = map_mapToRealX(tile_pos->x);
      b_top_left.y = map_mapToRealY(tile_pos->y);
    }

  /* Loop through a's boundaries: */
  a_b = a_bounds;
  while (a_b != NULL)
    {

      /* Loop through b's boundaries: */
      Bound *b_b = b_bounds;
      while (b_b != NULL)
	{
	  
	  /* If b's boundary is a line and a is moving in a direction which
	     can collide with the line: */
	  if (b_b->type == LINE &&
	      (dot(a_v, normal(b_b->b.line)) < 0))
	    {

	      /* Map the line into real coordinates: */
	      Line b_line;
	      b_line.p1.x = b_b->b.line.p1.x + b_top_left.x;
	      b_line.p1.y = b_b->b.line.p1.y + b_top_left.y;
	      b_line.p2.x = b_b->b.line.p2.x + b_top_left.x;
	      b_line.p2.y = b_b->b.line.p2.y + b_top_left.y;
	      
	      /* If a's boundary is a line and this line's normal is within
		 90 degrees of a's velocity: */
	      if (a_b->type == LINE &&
		  (dot(a_v, normal(a_b->b.line)) > 0))
		{
		  
		  /* Map the line into real coordinates: */
		  Line a_line;
		  a_line.p1.x = a_b->b.line.p1.x + a_top_left.x;
		  a_line.p1.y = a_b->b.line.p1.y + a_top_left.y;
		  a_line.p2.x = a_b->b.line.p2.x + a_top_left.x;
		  a_line.p2.y = a_b->b.line.p2.y + a_top_left.y;

		  /* See if there was a collision between the two lines,
		     and return if there was one: */
		  point_collided = getLineCollision(a_line, b_line, a_v, dt);
		  if (point_collided != -1)
		    {
		      Bound a, b;
		      a.type = LINE;
		      a.b.line = a_line;
		      b.type = LINE;
		      b.b.line = b_line;
		      return (newCollision(&a, &b, other_obj, tile_pos, point_collided));
		    }
		}
	      /* If a's boundary is a rect: */
	      else if (a_b->type == RECT)
		{
		  Line a_line;

		  /* Check for collisions as if the rectangle was composed
		     of 4 lines: */

		  /* Check top and bottom before sides, because I think that
		     there will be collisions with the objects' bottoms more
		     frequently than the sides because of gravity: */

		  /* If a has vertical velocity check its top or
		     bottom: */
		  if (a_v.y != 0) 
		    {
		      /* If a is moving down, check its bottom: */
		      if (a_v.y > 0)
			{
			  a_line.p1.x = a_b->b.rect.p2.x + a_top_left.x;
			  a_line.p1.y = a_b->b.rect.p2.y + a_top_left.y;
			  a_line.p2.x = a_b->b.rect.p1.x + a_top_left.x;
			  a_line.p2.y = a_line.p1.y;
			  
			}
		      else if (a_v.y < 0)
			{
			  a_line.p1.x = a_b->b.rect.p1.x + a_top_left.x;
			  a_line.p1.y = a_b->b.rect.p1.y + a_top_left.y;
			  a_line.p2.x = a_b->b.rect.p2.x + a_top_left.x;
			  a_line.p2.y = a_line.p1.y;
			}
		      /* Check for collision: */
		      point_collided = getLineCollision(a_line, b_line, a_v, dt);
		      if (point_collided != -1)
			{
			  Bound a, b;
			  a.type = LINE;
			  a.b.line = a_line;
			  b.type = LINE;
			  b.b.line = b_line;
			  return (newCollision(&a, &b, other_obj, tile_pos, point_collided));
			}
		    }
		  /* If a has horizontal velocity, check its sides: */
		  if (a_v.x != 0)
		    {
		      /* If a is moving right, check its right side:*/
		      if (a_v.x > 0)
			{
			  a_line.p1.x = a_b->b.rect.p2.x + a_top_left.x;
			  a_line.p1.y = a_b->b.rect.p1.y + a_top_left.y;
			  a_line.p2.x = a_line.p1.x;
			  a_line.p2.y = a_b->b.rect.p2.y + a_top_left.y;
			}
		      /* If it's moving left, check its left side: */
		      else if (a_v.x < 0)
			{
			  a_line.p1.x = a_b->b.rect.p1.x + a_top_left.x;
			  a_line.p1.y = a_b->b.rect.p2.y + a_top_left.y;
			  a_line.p2.x = a_line.p1.x;
			  a_line.p2.y = a_b->b.rect.p1.y + a_top_left.y;
			}
		      /* Check for collision: */
		      point_collided = getLineCollision(a_line, b_line, a_v, dt);
		      if (point_collided != -1)
			{
			  Bound a, b;
			  a.type = LINE;
			  a.b.line = a_line;
			  b.type = LINE;
			  b.b.line = b_line;
			  return (newCollision(&a, &b, other_obj, tile_pos, point_collided));
			}
		    }
		}
	      /* Else if a's bound == circle */
	      else if (a_b->type == CIRCLE)
		{
		  printf("Sorry, no circle collisions yet.\n");
		  
		}
	    }

	  /* If b's boundary is a rectangle: */
	  else if (b_b->type == RECT)
	    {

	      /* If a's boundary is a rectangle too: */
	      if (a_b->type == RECT)
		{
		  Rect a_rect, b_rect;

		  /* Map the rectangles into real coordinates for comparison:
		   */
		  b_rect.p1.x = b_b->b.rect.p1.x + b_top_left.x;
		  b_rect.p1.y = b_b->b.rect.p1.y + b_top_left.y;
		  b_rect.p2.x = b_b->b.rect.p2.x + b_top_left.x;
		  b_rect.p2.y = b_b->b.rect.p2.y + b_top_left.y;

		  a_rect.p1.x = a_b->b.rect.p1.x + a_top_left.x;
		  a_rect.p1.y = a_b->b.rect.p1.y + a_top_left.y;
		  a_rect.p2.x = a_b->b.rect.p2.x + a_top_left.x;
		  a_rect.p2.y = a_b->b.rect.p2.y + a_top_left.y;

		  /* If the two boundaries are rectangles, return a
		     collision if they simply overlap.  Collisions are
		     simpler this way, but two small rectangles are likely to
		     pass through each other at high speeds, so we just assume
		     that the rectangles are fairly big: */
		  if (testRectOverlap(a_rect, b_rect))
		    {
		      Bound a, b;
		      a.type = RECT;
		      a.b.rect = a_rect;
		      b.type = RECT;
		      b.b.rect = b_rect;
		      return (newCollision(&a, &b, other_obj, tile_pos, 0));
		    }
		}

	      /* If a's boundary is a line and its normal is within 90 degrees
		 of a's velocity: */
	      else if (a_b->type == LINE &&
		       (dot(a_v, normal(a_b->b.line)) > 0))
		{

		  Line b_line;

		  /* Map this object boundary into real coordinates */
		  Line a_line;
		  a_line.p1.x = a_b->b.line.p1.x + a_top_left.x;
		  a_line.p1.y = a_b->b.line.p1.y + a_top_left.y;
		  a_line.p2.x = a_b->b.line.p2.x + a_top_left.x;
		  a_line.p2.y = a_b->b.line.p2.y + a_top_left.y;

		  /* Check for collisions as if the b's rectangle is made of
		     of 4 lines: */

		  /* Check top and bottom before sides, because I think that
		     there will be collisions with the tiles' tops more
		     frequently than the sides because of gravity: */

		  /* If a has vertical velocity, check b's top
		     or bottom: */
		  if (a_v.y != 0) {
		  /* If a is moving down, check b's top: */
		    if (a_v.y > 0)
		      {
			b_line.p1.x = b_b->b.rect.p1.x + b_top_left.x;
			b_line.p1.y = b_b->b.rect.p1.y + b_top_left.y;
			b_line.p2.x = b_b->b.rect.p2.x + b_top_left.x;
			b_line.p2.y = b_line.p1.y;
			
		      }
		    /* If a is moving up, check b's bottom: */
		    else if (a_v.y < 0)
		      {
			b_line.p1.x = b_b->b.rect.p2.x + b_top_left.x;
			b_line.p1.y = b_b->b.rect.p2.y + b_top_left.y;
			b_line.p2.x = b_b->b.rect.p1.x + b_top_left.x;
			b_line.p2.y = b_line.p1.y;
		      }

		    /* Check for collision: */
		    point_collided = getLineCollision(a_line, b_line, a_v, dt);
		    if (point_collided != -1)
		      {
			Bound a, b;
			a.type = LINE;
			a.b.line = a_line;
			b.type = LINE;
			b.b.line = b_line;
			return (newCollision(&a, &b, other_obj, tile_pos, point_collided));
		      }
		    
		  }

		  /* If a has horizontal velocity, check the sides: */
		  if (a_v.x != 0) {
		    /* If a is moving right, check b's left: */
		    if (a_v.x > 0)
		      {
			b_line.p1.x = b_b->b.rect.p1.x + b_top_left.x;
			b_line.p1.y = b_b->b.rect.p2.y + b_top_left.y;
			b_line.p2.x = b_line.p1.x;
			b_line.p2.y = b_b->b.rect.p1.y + b_top_left.y;
		      }
		    /* If a is moving left, check b's right: */
		    else if (a_v.x < 0)
		      {
			b_line.p1.x = b_b->b.rect.p2.x + b_top_left.x;
			b_line.p1.y = b_b->b.rect.p1.y + b_top_left.y;
			b_line.p2.x = b_line.p1.x;
			b_line.p2.y = b_b->b.rect.p2.y + b_top_left.y;
		      }
		    /* Check for collision: */
		    point_collided = getLineCollision(a_line, b_line, a_v, dt);
		    if (point_collided != -1)
		      {
			Bound a, b;
			a.type = LINE;
			a.b.line = a_line;
			b.type = LINE;
			b.b.line = b_line;
			return (newCollision(&a, &b, other_obj, tile_pos, point_collided));
		      }
		  }

		}

	      /* else if a's bound == circle */
	      else if (a_b->type == CIRCLE)
		{
		  printf("Sorry, no circle collisions yet.\n");

		}
	    }
	  /* No collisions yet, see if b has any more boundaries: */
	  b_b = b_b->next;
	}
      /* No collisions yet, see if a has any more boundaries: */
      a_b = a_b->next;
    }

  /* No collision found, return NULL */
  return NULL;
}

/* col_listObjCollisions
   Return a list of all collisions with objects for an object.
*/
Collision *
col_listObjCollisions(Object *obj, Time dt)
{

  /* The linked list of collisions */
  Collision *obj_collisions = NULL;

  /* The sector the object is in */  
  Point obj_sector;
  /* The sector we're looking at: */
  Point curr_sector;
  /* The layer we're in */
  int l = obj_getObjLayer(obj);

  obj_sector.x = obj_realToSectorX(obj_getObjPos(obj).x);
  obj_sector.y = obj_realToSectorY(obj_getObjPos(obj).y);

  /* Loop through the sectors in the object's layer within a certain range */
  for (curr_sector.y = (obj_sector.y - SECTOR_Y_RANGE >= 0) ? obj_sector.y - SECTOR_Y_RANGE : 0;
       curr_sector.y <= obj_sector.y + SECTOR_Y_RANGE && curr_sector.y < obj_getLayerHeight(l);
       curr_sector.y++
       )
    {
      for (curr_sector.x = (obj_sector.x - SECTOR_X_RANGE >= 0) ? obj_sector.x - SECTOR_X_RANGE : 0;
	   curr_sector.x <= obj_sector.x + SECTOR_X_RANGE && curr_sector.x < obj_getLayerWidth(l);
	   curr_sector.x++
	   )
	
	{
	  /* Loop through all of the objects in the sector */
	  
	  Object *this_object = obj_getObjList(l, curr_sector.x, curr_sector.y);
	  while (this_object != NULL)
	    {

	      /* Make sure we're not detecting collisions between 1 object: */
	      if (this_object != obj) { 
		Collision *this_obj_collision = col_getCollision(obj, this_object, NULL, dt);
		
		/* If we found a collision with this object, add it to our
		   list: */
		if (this_obj_collision != NULL)
		  {
		    
		    /* If this is the first collision we've found,
		       add this collision as the first node in the list
		    */
		    if (obj_collisions == NULL)
		      {
			obj_collisions = this_obj_collision;
		      }
		    /* Otherwise, add this collision to the top of the
		       list: */
		    else
		      {
			this_obj_collision->next = obj_collisions;
			obj_collisions = this_obj_collision;
		      }
		  }
	      }
	      // get the next object
	      this_object = obj_getNextObj(this_object);

	    }
	}
    }
  return obj_collisions;
}
/* col_listTileCollisions

   Return a list of all collisions with tiles for an object.

   Searches all of the tiles that the object is currently overlapping.  For
   more accuracy I might want to find the bounding box of the object before
   it moved and after it moved, and find the bounding box of those bounding
   boxes, and search the tiles which that bounding box is overlapping.
*/

Collision *
col_listTileCollisions(Object *obj, Time dt)
{

  Point map_pos;
  Point obj_top_left = obj_getObjTopLeft(obj);
  Point obj_bot_right = obj_getObjBotRight(obj);
  /* The layer we're in */
  int l = obj_getObjLayer(obj);

  /* A linked list of collisions with tiles: */
  Collision *tile_collisions = NULL;
  
  
  /* Create a list of all tiles the object is colliding with*/
  
  /* For each tile which the object's bounding box overlaps: */
  for (map_pos.y = (map_realToMapY(obj_top_left.y) >= 0) ? map_realToMapY(obj_top_left.y) : 0;
       map_pos.y <= map_realToMapY(obj_bot_right.y) &&
	 map_pos.y < map_getLayerHeight(l);
       map_pos.y++)
    {
      for (map_pos.x = (map_realToMapX(obj_top_left.x) >= 0) ? map_realToMapX(obj_top_left.x) : 0;
	   map_pos.x <= map_realToMapX(obj_bot_right.x) &&
	     map_pos.x < map_getLayerWidth(l);
	   map_pos.x++)
	{

	  Collision *this_tile_collision = col_getCollision(obj, NULL, &map_pos, dt);
	  
	  /* If we found a collision with this tile, add it to our
	     list: */
	  if (this_tile_collision != NULL)
	    {
	      
	      /* If this is the first collision we've found,
		 add this collision as the first node in the list
	      */
	      if (tile_collisions == NULL)
		{
		  tile_collisions = this_tile_collision;
		}
	      /* Otherwise, add this collision to the top of the
		 list: */
	      else
		{
		  this_tile_collision->next = tile_collisions;
		  tile_collisions = this_tile_collision;
		}
	    }
	}
    } /* End of for loops, found all the tiles */

  return tile_collisions;

}

/* col_listCollisions
   Gets lists of obj collisions and tile collisions and concatenates them.
*/
Collision *col_listCollisions(Object *obj, Time dt)
{

  Collision *collisions = col_listTileCollisions(obj, dt);
  Collision *temp;
  temp = collisions;

  if (temp == NULL)
    collisions = col_listObjCollisions(obj, dt);
  else
    {
      while (temp->next != NULL)
	temp = temp->next;
      temp->next = col_listObjCollisions(obj, dt);
    }
  return collisions;
}

/* col_collisionResponse
   Given an object which is colliding and rough info about the collision,
   change the position and velocity of the object accordingly.  If the object
   is colliding with another object, modify the other object's velocity and
   position as well.

   Also, send impulse signals to objects whose velocities changed.
*/
void
col_collisionResponse(Object *obj, Collision *coll_info, Time dt)
{

  Signal sig_a, sig_b;
  Vector impulse_a, impulse_b;
  Velocity v_a, v_b, rel_v_a;
  Point pos_a, new_pos_a, pos_b, new_pos_b;
  int mass_a, mass_b;

  float k_a, k_b, k;    /* Coefficients of elasticity */

  /* Get object a's position, velocity, mass, and elasticity: */
  v_a = obj_getObjVel(obj);
  new_pos_a = pos_a = obj_getObjPos(obj);
  mass_a = obj_getObjMass(obj);
  k_a = obj_getObjElasticity(obj);

  /* If b is an object, get its position, velocity, mass and elasticity*/
  if (coll_info->type == OBJ_TYPE)
    {
      v_b = obj_getObjVel(coll_info->other.obj);
      new_pos_b = pos_b = obj_getObjPos(coll_info->other.obj);
      mass_b = obj_getObjMass(coll_info->other.obj);
      k_b = obj_getObjElasticity(coll_info->other.obj);
    }
  else
    {
      v_b.x = 0;
      v_b.y = 0;
      k_b = 1;
    }

  /* The elasticity for the collision is the product of each object's
     elasticity */
  k = k_a * k_b;

  /* Do these detections using a's relative velocity to b */
  rel_v_a.x = v_a.x - v_b.x;
  rel_v_a.y = v_a.y - v_b.y;


  /* If a's boundary is a rectangle: */
  if (coll_info->a.type == RECT)
    {

      /* If both boundaries are rectangles, all we know about the collision 
	 is that they are overlapping, so we'll start from there: */

      if (coll_info->b.type == RECT)
	{

	  Vector n = {0, 0};
	  int x_depth = 0, y_depth = 0;

	  /* Find the overlap depths of the rectangles.  If there is an overlap
	     in one dimension and the overlap is less than the distance 
	     travelled in that dimension, there might have been a collision
	     in that dimension.  If so, set one component of a normal vector
	     of the colliding surface to remember. */
	  if (rel_v_a.x > 0)
	    {
	      x_depth = coll_info->a.b.rect.p2.x - coll_info->b.b.rect.p1.x;
	      if (APPROX(rel_v_a.x * dt) >= x_depth) n.x = -1;
	    }
	  else if (rel_v_a.x < 0)
	    {
	      x_depth = coll_info->a.b.rect.p1.x - coll_info->b.b.rect.p2.x;
	      if (APPROX(rel_v_a.x * dt) <= x_depth) n.x = 1;
	    }

	  if (rel_v_a.y > 0)
	    {
	      y_depth = coll_info->a.b.rect.p2.y - coll_info->b.b.rect.p1.y;
	      if (APPROX(rel_v_a.y * dt) >= y_depth) n.y = -1;
	    }
	  else if (rel_v_a.y < 0)
	    {
	      y_depth = coll_info->a.b.rect.p1.y - coll_info->b.b.rect.p2.y;
	      if (APPROX(rel_v_a.y * dt) <= y_depth) n.y = 1;
	    }


	  /* If we have both an x and y overlap, it is not immediately obvious
	     whether the collision was on a side or a top or bottom.  So what
	     we do is, move backwards along the velocity vector and see if
	     there is still y overlap at that point.  If there is, we know
	     it was an x-collision. */
	  if (n.x != 0 && n.y != 0)
	    {

	      /* The y-distance travelled at the point where there is no
		 x-overlap: */
	      int dy;
	      dy = APPROX((APPROX(rel_v_a.x * dt) - x_depth) * APPROX(rel_v_a.y * dt) / APPROX(rel_v_a.x * dt));

	      if (n.y == -1)
		{
		  if (y_depth > dy) n.y = 0;
		  else n.x = 0;
		}
	      else 
		{
		  if (y_depth < dy) n.y = 0;
		  else n.x = 0;
		}

	    }


	  /* Right, now we know which side the collision occurred on, and
	     we know how deeply the rectangles overlap, so we can set the
	     position and velocity of the object accordingly: */


	  /* If both things are objects: */
	  if (coll_info->type == OBJ_TYPE)
	    {
	      float a_a, a_b, optimizedP;
	      
	      /* We are using n as the normal for the surface of impact between
		 the two objects */
	      
	      a_a = dot(v_a, n);
	      a_b = dot(v_b, n);
	      
	      /* Perfectly inelastic when k = 0, perfectly elastic when k = 1.
		 Really bouncy when k > 1. */
	      optimizedP = ((1 + k) * (a_a - a_b)) / (mass_a + mass_b);
	      
	      
	      impulse_a.x = -optimizedP * mass_b * n.x;
	      impulse_a.y = -optimizedP * mass_b * n.y;
	      
	      impulse_b.x = optimizedP * mass_a * n.x;
	      impulse_b.y = optimizedP * mass_a * n.y;
	      
	      /* Move both of the objects out of collision: */

	      new_pos_a.x = pos_a.x + rint(x_depth / 2) * n.x + n.x;
	      new_pos_a.y = pos_a.y + rint(y_depth / 2) * n.y + n.y;
	      new_pos_b.x = pos_b.x - rint(x_depth / 2) * n.x - n.x;
	      new_pos_b.y = pos_b.y - rint(y_depth / 2) * n.y - n.y;
	      
	      	      
	    }
	  /* Otherwise, just change the position and velocity of the one
	     object. */
	  else
	    {
	      impulse_a.x = -n.x * n.x * v_a.x * (1 + k);
	      impulse_a.y = -n.y * n.y * v_a.y * (1 + k);
	      new_pos_a.x = pos_a.x + x_depth * n.x + n.x;
	      new_pos_a.y = pos_a.y + y_depth * n.y + n.y;
	    }

	}
    }
  /* If a's boundary is a line: */
  else if (coll_info->a.type == LINE)
    {

      /* If b's boundary is also a line: */
      if (coll_info->b.type == LINE)
	{

	  /* Given that we know which point on which line intersected with the
	     other line, we can now find the exact point where that happened:
	  */
	  Point p;
	  Line l1, l2;

	  int x_depth = 0, y_depth = 0;
	  Vector n;

	  switch (coll_info->point_collided)
	    {
	    case L1P1:
	      l1.p2 = coll_info->a.b.line.p1;
	      l1.p1.x = l1.p2.x - APPROX(rel_v_a.x * dt);
	      l1.p1.y = l1.p2.y - APPROX(rel_v_a.y * dt);

	      l2 = coll_info->b.b.line;

	      p = intersection(l1, l2);
	      x_depth = l1.p2.x - p.x;
	      y_depth = l1.p2.y - p.y;
	      

	      break;
	    case L1P2:
	      l1.p2 = coll_info->a.b.line.p2;
	      l1.p1.x = l1.p2.x - APPROX(rel_v_a.x * dt);
	      l1.p1.y = l1.p2.y - APPROX(rel_v_a.y * dt);

	      l2 = coll_info->b.b.line;

	      
	      p = intersection(l1, l2);
	      x_depth = l1.p2.x - p.x;
	      y_depth = l1.p2.y - p.y;

	      break;
	    case L2P1:
	      l1.p1 = coll_info->b.b.line.p1;
	      l1.p2.x = l1.p1.x + APPROX(rel_v_a.x * dt);
	      l1.p2.y = l1.p1.y + APPROX(rel_v_a.y * dt);

	      l2 = coll_info->a.b.line;

	      p = intersection(l1, l2);
	      x_depth = p.x - l1.p1.x;
	      y_depth = p.y - l1.p1.y;

	      break;
	    case L2P2:
	      l1.p1 = coll_info->b.b.line.p2;
	      l1.p2.x = l1.p1.x + APPROX(rel_v_a.x * dt);
	      l1.p2.y = l1.p1.y + APPROX(rel_v_a.y * dt);

	      l2 = coll_info->a.b.line;

	      p = intersection(l1, l2);
	      x_depth = p.x - l1.p1.x;
	      x_depth = p.y - l1.p1.y;

	      break;
	    default:
	      printf("Error:  You suck\n");
	      break;
	    }

	  /* Get the normal of the b's line boundary: */
	  n = normal(coll_info->b.b.line);

	  /* If the collision is between two objects, 
	     do some fancy pants
	     momentum physics, which I found on
	     http://www.gamasutra.com/features/20020118/vandenhuevel_03.htm */
	  if (coll_info->type == OBJ_TYPE)
	    {
	      float a_a, a_b, optimizedP;

	      /* We are using n as the normal for the surface of impact between
		 the two objects */

	      a_a = dot(v_a, n);
	      a_b = dot(v_b, n);

	      /* Perfectly inelastic when k = 0, perfectly elastic when k = 1.
		 Really bouncy when k > 1. */
	      optimizedP = ((1 + k) * (a_a - a_b)) / (mass_a + mass_b);


	      impulse_a.x = -optimizedP * mass_b * n.x;
	      impulse_a.y = -optimizedP * mass_b * n.y;

	      impulse_b.x = optimizedP * mass_a * n.x;
	      impulse_b.y = optimizedP * mass_a * n.y;


	      /* Move both of the objects out of collision: */
	      new_pos_a.x = pos_a.x - rint(x_depth / 2) + rint(n.x);
	      new_pos_a.y = pos_a.y - rint(y_depth / 2) + rint(n.y);
	      new_pos_b.x = pos_b.x + rint(x_depth / 2) - rint(n.x);
	      new_pos_b.y = pos_b.y + rint(y_depth / 2) - rint(n.y);
	      
	    }

	  /* The tile is not movable, so just do some simple
	     physics: */
	  else
	    {
	      float v_dot_n;

	      /* Move a back to the point of intersection, plus a little
		 bit more in the direction of the line's normal: */
	      
	      new_pos_a.x = pos_a.x - x_depth + rint(n.x);
	      new_pos_a.y = pos_a.y - y_depth + rint(n.y);


	      v_dot_n = dot(v_a, n);
	      
	      v_dot_n *= 1 + k;
	      
	      impulse_a.x = -v_dot_n * n.x;
	      impulse_a.y = -v_dot_n * n.y;
	    }



	}

    }
  /* If a's boundary is a circle: */
  else if (coll_info->a.type == CIRCLE)
    {
      printf("Sorry, no circle collisions yet.\n");
    }


  /* Update the position and velocity: */
  v_a.x += impulse_a.x;
  v_a.y += impulse_a.y;
  obj_setObjVel(obj, v_a);
  obj_setObjPos(obj, new_pos_a);

  /* Send an impulse signal */
  sig_a.type = IMPULSE_SIG;
  sig_a.sig.imp.vec = impulse_a;
  sig_a.sig.imp.hit.type = coll_info->type;
  if (coll_info->type == OBJ_TYPE)
    sig_a.sig.imp.hit.u.obj_type = obj_getObjType(coll_info->other.obj);
  else
    sig_a.sig.imp.hit.u.tile_type = map_getTileType(obj_getObjLayer(obj), coll_info->other.tile_pos.x, coll_info->other.tile_pos.y);

  obj_sendObjSignal(obj, &sig_a);

  /* Update the 2nd object (if it's an object) */
  if (coll_info->type == OBJ_TYPE)
    {
      v_b.x += impulse_b.x;
      v_b.y += impulse_b.y;
      obj_setObjVel(coll_info->other.obj, v_b);
      obj_setObjPos(coll_info->other.obj, new_pos_b);

      /* Send an impulse signal */
      sig_b.type = IMPULSE_SIG;
      sig_b.sig.imp.vec = impulse_b;
      sig_b.sig.imp.hit.type = OBJ_TYPE;
      sig_b.sig.imp.hit.u.obj_type = obj_getObjType(obj);

      obj_sendObjSignal(coll_info->other.obj, &sig_b);
    }
  /* If it's a tile, create an impulse to send it: */
  else
    {
      sig_b.type = IMPULSE_SIG;
      sig_b.sig.imp.vec.x = -impulse_a.x;
      sig_b.sig.imp.vec.y = -impulse_a.y;
      sig_b.sig.imp.hit.type = OBJ_TYPE;
      sig_b.sig.imp.hit.u.obj_type = obj_getObjType(obj);

      map_sendTileSignal(obj_getObjLayer(obj), coll_info->other.tile_pos.x, coll_info->other.tile_pos.y, &sig_b);
    }
}

/* findSolidCollision
   Returns the first solid collision in a list
*/
Collision *findSolidCollision(Object *obj, Collision *c)
{
  Collision *temp = c;

  if (!obj_isSolid(obj)) return NULL;

  while (temp != NULL)
    {
      if (temp->type == OBJ_TYPE)
	{
	  if (obj_isSolid(temp->other.obj)) return temp;
	}
      else
	if (map_tileIsSolid(obj_getObjLayer(obj), temp->other.tile_pos.x, temp->other.tile_pos.y)) return temp;
      temp = temp->next;
    }
  return NULL;
}

/* col_doTileCollisions
   Find collisions with other tiles, send signals and do collision responses.
*/
void col_doTileCollisions(Object *obj, Time dt)
{

  /* Only check for collisions if the object is moving */
  Velocity v = obj_getObjVel(obj);
  if (v.x != 0 || v.y != 0)
    {  
      int loop_timeout = 0;
      
      /* Get tile collisions for this object */
      Collision *this, *collisions;
      this = collisions = col_listTileCollisions(obj, dt);
     
      /* Send hit signals to the object and tile for each collision*/
      while (this != NULL)
	{
	  Signal o_sig, t_sig;
	  o_sig.type = HIT_SIG;
	  o_sig.sig.hit.type = TILE_TYPE;
	  o_sig.sig.hit.u.tile_type = map_getTileType(obj_getObjLayer(obj), this->other.tile_pos.x, this->other.tile_pos.y);

	  t_sig.type = HIT_SIG;
	  t_sig.sig.hit.type = OBJ_TYPE;
	  t_sig.sig.hit.u.obj_type = obj_getObjType(obj);

	  obj_sendObjSignal(obj, &o_sig);
	  map_sendTileSignal(obj_getObjLayer(obj), this->other.tile_pos.x, this->other.tile_pos.y, &t_sig);

	  this = this->next;
	}


      /* Do a collision response for the first solid collision in the list, and
	 repeat while there are solid collisions detected. */
      while ((this = findSolidCollision(obj, collisions)) != NULL && ++loop_timeout < COLLISION_MAX_LOOPS)
	{

	  col_collisionResponse(obj, this, dt);
	  
	  while (collisions != NULL)
	    {
	      this = collisions;
	      collisions = this->next;
	      free(this);
	    }
	  collisions = col_listTileCollisions(obj, dt);
	  
	}
      
      /* Free the remaining collisions */
      while (collisions != NULL)
	{
	  this = collisions;
	  collisions = this->next;
	  free(this);
	}
    }
}

/* col_doObjCollisions
   Find collisions with other objects, send signals and do collision responses.
*/
void col_doObjCollisions(Object *obj, Time dt)
{


  /* Only check for collisions if the object is moving */
  Velocity v = obj_getObjVel(obj);
  if (v.x != 0 || v.y != 0)
    {
      int loop_timeout = 0;
      Collision *this, *collisions;
      
      this = collisions = col_listObjCollisions(obj, dt);

      /* Send hit signals to the objects in each collision detected. */
      while (this != NULL)
	{
	  Signal sig_a, sig_b;
	  sig_a.type = sig_b.type = HIT_SIG;
	  sig_a.sig.hit.type = sig_b.sig.hit.type = OBJ_TYPE;
	  sig_a.sig.hit.u.obj_type = obj_getObjType(this->other.obj);
	  sig_b.sig.hit.u.obj_type = obj_getObjType(obj);

	  obj_sendObjSignal(obj, &sig_a);
	  obj_sendObjSignal(this->other.obj, &sig_b);
	  this = this->next;
	}

      /* Do a collision response for the first 
	 solid collision in the list, and
	 repeat while there are solid collisions detected. */
       while ((this = findSolidCollision(obj, collisions)) != NULL && ++loop_timeout < COLLISION_MAX_LOOPS)
	{
	  col_collisionResponse(obj, this, dt);
	  while (collisions != NULL)
	    {
	      this = collisions;
	      collisions = this->next;
	      free(this);
	    }
	  collisions = col_listObjCollisions(obj, dt);
	  
	}

       /* Free the remaining collisions */
      while (collisions != NULL)
	{
	  this = collisions;
	  collisions = this->next;
	  free(this);
	}
    }

}

/* col_doCollisions
   Check for collisions for an object and do collision responses.
*/
void col_doCollisions(Object *obj, Time dt)
{



  col_doTileCollisions(obj, dt);

  col_doObjCollisions(obj, dt);

}
