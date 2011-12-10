#ifndef __DEFINED_OBJECT_H
#define __DEFINED_OBJECT_H
 
#include "defs.h"
#include "map.h"
#include "file.h"
#include "animation.h"
#include "dynarray.h"
#include "timer.h"
#include "signal.h"
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Dimensions in real coordinates of sectors:

   About sectors:  Sectors are not a physical thing defined within the map or
   level data, they are just something that the object module provides to
   organize the objects so that access to information about objects is more
   efficient.  By only having to scan through objects within a limited range
   of sectors, we can drastically reduce the amount of wasted operations
   for objects that are not on screen or near where the game action is.
*/
#define SECTOR_W 300
#define SECTOR_H 150

/* What range of sectors should our various operations scan through? */
#define SECTOR_X_RANGE 2
#define SECTOR_Y_RANGE 2

/* Convert real coordinates to sector coordinates */
#define obj_realToSectorX(x) ((x) / SECTOR_W)
#define obj_realToSectorY(y) ((y) / SECTOR_H)

/* A note on some terminology:
   Objects are the things in our world which move and do stuff.  Objects
   have sprites, which refer to preloaded sprite data, which is no more than
   groups of animation data.
*/
typedef struct sprdata_struct
{
  char *name;         /* The name of the sprite */
  int n_animations;   /* The number of animations in the sprite */
  AnimData *data;     /* A 1d array of AnimData */
} SprData;

typedef struct spriteset_struct
{
  int n_sprites;    /* The number of sprites */
  SprData *data;     /* A 1d array of SprData */
} SpriteSet;

typedef struct sprite_struct
{
  int spr_id;        /* This corresponds to the location of SprData in a
			SpriteSet */
  Animation anim;    /* This contains the information about the currently
			used animation in the Sprite */
} Sprite;

typedef struct object_struct
{
  int layer;       /* The layer the object is in */
  Point pos;       /* The position of the center of the object, in real
		      coordinates. */

  int fine_x;    /* These are more precise coordinate values than the */
  int fine_y;    /* integer coordinates.  They are needed because
		      objects move less than one pixel per cycle. */
  Velocity vel;    /* The object's velocity vector */

  int mass;        /* The object's mass. */

  int w;           /* The dimensions of the object. */
  int h;           /* (The object's top left corner is at (pos.x -w/2,
		      pos.y - h/2) */

  Sprite spr;      /* Contains graphical information for the object */

  int solid;       /* Whether or not the object can pass through things. */

  float elasticity;/* How elastic the object's collisions are. Values: 0 - 1 */
  float friction;  /* Not used yet... */


  Bound *bounds;   /* A pointer to the linked list of boundaries an object
		      has.  These boundaries are defined with coordinates
		      originating from the object's top left corner. */

  int type; // these values are defined in an enumerated list

  /* What the object does every turn -- usually involves processing the list
     of signals received during that turn. */
  void (*go)(struct object_struct *, Time dt);

  /* The object is responsible for freeing its own type-specific attributes: */
  void (*free_atts)(struct object_struct *);
  
  void *atts;      /* This is a pointer to the structure containing attributes
		      specific to this type of object. */

  SigQ signals;    /* Objects accumulate signals in a queue */

  int dead;        /* This flag is set when an object is to be killed, so that
		      if two signals to kill an object, the object is only
		      freed once. */

  struct object_struct *next; // objects are maintained in linked lists

} Object;

typedef struct obj_layer_struct
{
  int w, h;     /* The dimensions in sectors of the layer */
  Object ***obj_array;  /* A 2d array of pointers to objects */
} ObjLayer;

typedef struct obj_container_struct
{
  int n_layers;     /* The number of layers (same as the map) */
  ObjLayer *layers; /* An array of layers of objects */
} ObjContainer;

/* A handy macro for accessing the list of objects in the object container */
#define OBJ_AT(z, x, y) (the_objects.layers[(z)].obj_array[(x)][(y)])

extern void obj_loadSprites(char *sprite_datfile);
extern void obj_freeSprites(void);
extern void obj_loadObjects(char *areafile);
extern void obj_freeObjects(void);
extern void obj_spawnObj(int layer, Point pos, Velocity vel, int type);
extern void obj_killObj(Object *obj);
extern void obj_handleSignals(void);
extern void obj_setSprite(char *name, Object *obj);
extern void obj_setAnim(char *name, Object *obj);
extern int obj_getLayerWidth(int l);
extern int obj_getLayerHeight(int l);
extern Object *obj_getObjList(int l, int x, int y);
extern Object *obj_getNextObj(Object *this_object);
extern SDL_Surface *obj_getObjGfx(Object *obj);
extern Point obj_getObjTopLeft(Object *obj);
extern Point obj_getGfxPos(Object *obj);
extern Bound *obj_getObjBounds(Object *object_ptr);
extern Point obj_getObjPos(Object *obj);
extern int obj_isSolid(Object *obj);
extern int obj_getObjType(Object *obj);
extern int obj_getObjLayer(Object *obj);
extern Point obj_getObjBotRight(Object *obj);
extern Velocity obj_getObjVel(Object *object_ptr);
extern int obj_getObjMass(Object *obj);
extern float obj_getObjElasticity(Object *object_ptr);
extern void obj_setObjPos(Object *object_ptr, Point new_pos);
extern void obj_moveObj(Object *obj, Time dt);
extern void obj_setObjVel(Object *object_ptr, Velocity vel);
extern Object *obj_getPlayerPtr(void);
extern void obj_animateObj(Object *obj);
extern void obj_setAnimSpeed(Object *obj, int speed);
extern void obj_sendObjSignal(Object *obj, Signal *s);
extern int obj_makeSound(Object *obj, char *sound, int loops);
#define obj_stopSound(x) aud_haltSound(x)

#define obj_getSectorW() SECTOR_W
#define obj_getSectorH() SECTOR_H

#endif /* __DEFINED_OBJECT_H */
