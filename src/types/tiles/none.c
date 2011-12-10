#include "../tiletypes.h"

/* This tile does nothing.  It may or may not have boundaries, but it does
   have an animation. It is the basic building block of a level. */
struct tile_att_define none_def =
{
  /* solid = */ 1,
  /* elasticity = */ 1,
  /* friction = */ 0,
  /* active = */ 0,
  /* init_atts = */ NULL,
  /* go = */ NULL,
  /* free_atts = */ NULL
};
