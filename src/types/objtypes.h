#ifndef __DEFINED_OBJTYPES_H
#define __DEFINED_OBJTYPES_H

#include "../object.h"

/* Object types */
enum obj_types
{
  PLAYER_TYPE = 0,
  BADDIE_TYPE,
  BULLET_TYPE
};

/* To make an object, create a new .c file in the objects/ directory.
   This file may contain private functions for the object's behavior, but it
   must end with an obj_att_define structure: */
struct obj_att_define
{
  int mass;                    /* The object's mass. */
  int w;                       /* Width */
  int h;                       /* Height */
  float elasticity;            /* Coefficient of elasticity (0 - 1) */
  float friction;              /* Coefficient of friction (not yet done) */
  int solid;                   /* Can the object pass through things? */
  char *sprite;                /* Sprite name */
  char *animation;             /* Initial animation name */
  Bound *(*bounds)(void);      /* Pointer to a function which creates the 
				  object's bounds */
  void *(*init_atts)(void);    /* Pointer a function which allocates the
				  type-specific attributes */
  void (*go)(Object *, Time);  /* Pointer to a function which the object 
				  does once every game cycle */
  void (*free_atts)(Object *); /* Pointer to a function which frees the
				  type-specific attributes */
};

extern struct obj_att_define *obj_defs[];

#endif /* __DEFINED_OBJTYPES_H */
