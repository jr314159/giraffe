#include "objtypes.h"

extern struct obj_att_define player_def, baddie_def, bullet_def;

/* This is an array of pointers to object definition structures.  Each object
   .c file must have one of these structures. */

struct obj_att_define *obj_defs[] =
{
  &player_def,
  &baddie_def,
  &bullet_def
};
