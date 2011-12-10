#ifndef __DEFINED_TILETYPES_H
#define __DEFINED_TILETYPES_H

#include "../map.h"

enum tile_types
{
  NONE_T = 0     /* The tile does nothing */
};

struct tile_att_define
{
  int solid;
  float elasticity;
  float friction;
  int active;
  void *(*init_atts)(void);
  void (*go)(int, int, int);
  void (*free_atts)(int, int, int);
};


#endif /* __DEFINED_TILETYPES_H */
