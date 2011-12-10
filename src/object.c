#include "object.h"

#include "camera.h"

/* The definitions for object types are in this header: */
#include "types/objtypes.h"
extern struct obj_att_define *obj_defs[];

/* The sprites are loaded in a SpriteSet */
static SpriteSet sprite_set;

/* The objects are loaded into this structure */
static ObjContainer the_objects;

/* Objects can send signals to the object module */
static SigQ signals;


/* A pointer we maintain that knows where the player object is: */
static Object *player_pointer;

/* Private function prototypes */
static void insertObj(Object *obj);
static void removeObj(Object *obj);
static int spriteNameToID(char *name);
static int animNameToID(int spr_id, char *name);
static void setPlayerPtr(Object *ptr);
static Object *newObject(int layer, Point pos, Velocity vel, int type);
static void freeObject(Object *obj);

/* obj_loadSprites
   Loads sprite animations.
*/
void
obj_loadSprites(char *sprite_datfile)
{
  extern SpriteSet sprite_set;
  char *spr_name, *spr_dir;
  int i = 0;

  /* Count the number of sprites and allocate the spriteset array */
  sprite_set.n_sprites = file_countDatPairs(sprite_datfile);
  sprite_set.data = (SprData *) dyn_1dArrayAlloc(sprite_set.n_sprites, sizeof(SprData));

  /* Open the sprite datfile for reading */
  file_openFile(sprite_datfile, 'r');

  /* Get all of the sprite names and directories */
  while (file_getNextDatPair(sprite_datfile, &spr_name, &spr_dir))
    {
      char *anim_name, *anim_dir;
      int j = 0;

      /* Assign the name to the sprite */
      sprite_set.data[i].name = spr_name;
      /* Count the number of animations in the sprite */
      sprite_set.data[i].n_animations = file_countSpriteAnims(spr_dir);
      /* Allocate the array of animations */
      sprite_set.data[i].data = (AnimData *) dyn_1dArrayAlloc(sprite_set.data[i].n_animations, sizeof(AnimData));


      /* Open the sprite file to read out the animations */
      file_openSpriteFile(spr_dir, 'r');

      while (file_getNextSpriteAnim(spr_dir, &anim_name, &anim_dir))
	{
	  file_loadAnim(anim_name, anim_dir, &sprite_set.data[i].data[j]);
	  free(anim_name); free(anim_dir);
	  j++;
	}
      /* Close the sprite file */
      file_closeSpriteFile(spr_dir);

      free(spr_dir);
      /* Don't free spr_name because we saved that string in the sprite */
      i++;
    }

  file_closeFile(sprite_datfile);
}

/* obj_freeSprites
   free the loaded sprite animations
*/
void
obj_freeSprites(void)
{
  extern SpriteSet sprite_set;
  int i, j;

  /* Loop through all of the sprites in the sprite set */
  for (i = 0; i < sprite_set.n_sprites; i++)
    {
      /* Loop through all of the animations in the sprite */
      for (j = 0; j < sprite_set.data[i].n_animations; j++)
	{
	  anim_freeAnim(&sprite_set.data[i].data[j]);
	}
      /* Free the animation array */
      dyn_1dArrayFree(sprite_set.data[i].data);

      /* Free the sprite's name */
      free(sprite_set.data[i].name);
    }
  /* Free the sprite array */
  dyn_1dArrayFree(sprite_set.data);
}

/* obj_loadObjects
   Load objects for an area.
   (The map must be loaded first before this can be done.)
*/
void
obj_loadObjects(char *areafile)
{
  extern ObjContainer the_objects;
  int l;

  /* Allocate the same number of layers as the map has: */
  the_objects.n_layers = map_getNLayers();
  the_objects.layers = (ObjLayer *) dyn_1dArrayAlloc(the_objects.n_layers, sizeof(ObjLayer));

  /* Allocate the layers based on the dimensions of sectors and the dimensions
     of each map layer */
  for (l = 0; l < the_objects.n_layers; l++)
    {
      /* Make sure to round up here so that there are enough sectors: */
      the_objects.layers[l].w = (int) ceilf(obj_realToSectorX((float) map_mapToRealX(map_getLayerWidth(l))));
      the_objects.layers[l].h = (int) ceilf(obj_realToSectorY((float) map_mapToRealY(map_getLayerHeight(l))));

      the_objects.layers[l].obj_array = (Object ***) dyn_arrayAlloc(the_objects.layers[l].w, the_objects.layers[l].h, sizeof(Object *));
    }

  /* Load the objects from the area file */
  file_openFile(areafile, 'r');

  /* Loop through all of the objects in the file */
  while (file_nextObject(areafile))
    {
      Velocity vel;
      Point pos;
      int z, type;

      /* Get the attributes for this object */
      file_getObjectAtts(areafile, &z, &pos, &vel, &type);

      /* Create the object and put it into the container */
      insertObj(newObject(z, pos, vel, type));
    }

  /* Close the file */
  file_closeFile(areafile);
}

/* obj_freeObjects
   Free all loaded objects.
*/
void
obj_freeObjects(void)
{
  extern ObjContainer the_objects;
  int l, x, y;

  /* In case there are any stray objects that were created but not put into
     the object container, put them in now: */
  obj_handleSignals();

  /* Loop through all of the layers */
  for (l = 0; l < the_objects.n_layers; l++)
    {
      /* Loop through all the sectors */
      for (x = 0; x < the_objects.layers[l].w; x++)
	{
	  for (y = 0; y < the_objects.layers[l].h; y++)
	    {
	      //free all of the objects in this sector
	      while (OBJ_AT(l, x, y) != NULL)
		{
		  freeObject(OBJ_AT(l, x, y));
		}
	    }
	}
      /* Free the layer */
      dyn_arrayFree ((void **) the_objects.layers[l].obj_array, the_objects.layers[l].w);
    }
  /* Free the array of layers */
  dyn_1dArrayFree(the_objects.layers);

}

/* obj_handleSignals
   Handle signals that have accumulated in the module's queue.  Such signals
   may include object killing or spawning signals.
*/
void obj_handleSignals(void)
{
  extern SigQ signals;
  Signal sig;

  while (sig_poll(&signals, &sig))
    {
      switch (sig.type)
	{
	case SPAWN_KILL_SIG:
	  switch (sig.sig.sk.todo)
	    {
	    case SPAWN_OBJECT:
	      insertObj((Object *) sig.sig.sk.obj);
	      break;
	    case KILL_OBJECT:
	      freeObject((Object *) sig.sig.sk.obj);
	      break;
	    }
	  break;
	}
    }
}

/* obj_spawnObj
   Create a new object and send a spawn object signal to the module.  This 
   is the function that objects or other things call when they want to make
   a new object.  This function does not actually insert the object into the
   object container.
*/
void obj_spawnObj(int layer, Point pos, Velocity vel, int type)
{
  extern SigQ signals;
  Signal sig;

  /* Only spawn the object if the values we got were okay: */
  if (layer < 0 || layer >= map_getNLayers() ||
      pos.x < 0 || pos.x >= map_mapToRealX(map_getLayerWidth(layer)) ||
      pos.y < 0 || pos.y >= map_mapToRealY(map_getLayerHeight(layer)))
    return;
  
  sig.type = SPAWN_KILL_SIG;
  sig.sig.sk.todo = SPAWN_OBJECT;
  sig.sig.sk.obj = (void *) newObject(layer, pos, vel, type);
  sig_push(&signals, &sig);

}

/* obj_killObj
   When objects want to kill an object, they call this function, which adds
   a kill object signal to the module's queue.
*/
void obj_killObj(Object *obj)
{

  /* Only allow a kill signal to be sent once for an object. */
  if (!obj->dead)
    {
      extern SigQ signals;
      Signal sig;
      
      sig.type = SPAWN_KILL_SIG;
      sig.sig.sk.todo = KILL_OBJECT;
      sig.sig.sk.obj = (void *) obj;
      sig_push(&signals, &sig);
      obj->dead = 1;
    }
}

/* newObject
   Given the object's layer, position, starting velocity and type, create it
   , but do not put it in the object container.  Returns
   a pointer to the object.  NOTE: This is a private function.  When other
   objects want to create objects, they should use the public function
   obj_spawnObj()
*/
Object *
newObject(int layer, Point pos, Velocity vel, int type)
{

  extern SpriteSet sprite_set;
  extern struct obj_att_define *obj_defs[];

  Object *obj;

  MALLOC(obj, sizeof(Object));

  /* Set the position and velocity */
  obj->pos = pos;
  obj->layer = layer;
  obj->vel = vel;

  /* Its fine coordinate values are 0 to start */
  obj->fine_x = 0.0;
  obj->fine_y = 0.0;

  /* Set bounds to NULL just in case the object does not create any. */
  obj->bounds = NULL;

  /* Initialize the signal queue */
  sig_initQ(&obj->signals);

  /* The object is not to be killed yet: */
  obj->dead = 0;

  /* Objects go in linked lists */
  obj->next = NULL;

  /* Do type-specific initialization */
  obj->type = type;

  /* obj_data.c contains an array of pointers to object definition structures
     which each object-type .c file contains */
  obj->mass = obj_defs[type]->mass;
  obj->elasticity = obj_defs[type]->elasticity;
  obj->friction = obj_defs[type]->friction;
  obj->solid = obj_defs[type]->solid;
  obj->w = obj_defs[type]->w;
  obj->h = obj_defs[type]->h;
  obj_setSprite(obj_defs[type]->sprite, obj);
  obj_setAnim(obj_defs[type]->animation, obj);
  obj->bounds = obj_defs[type]->bounds();
  obj->atts = obj_defs[type]->init_atts();
  obj->go = obj_defs[type]->go;
  obj->free_atts = obj_defs[type]->free_atts;


  /* Special: If this is the player, save a pointer to it */
  if (obj->type == PLAYER_TYPE) setPlayerPtr(obj);

  return obj;
}

/* freeObject
   Frees an object, removing it from the object container.
*/
void
freeObject(Object *obj)
{
  extern ObjContainer the_objects;
  Bound *b;

  /* Free the object's boundaries */
  b = obj->bounds;
  while (b != NULL)
    {
      Bound *next_bound = b->next;
      free(b);
      b = next_bound;
    }

  /* Flush the signal queue */
  sig_flush(&obj->signals);

  /* The object has its own function for freeing attributes because within
     the atts structure, it may have allocated additional memory: */
  obj->free_atts(obj);

  removeObj(obj);
  free(obj);

  obj = NULL;
}

/* insertObj
   Insert an object in the right place in the right object layer.
*/
void
insertObj(Object *obj)
{

  extern ObjContainer the_objects;
  int array_x, array_y;

  /* Insert the object in the beginning of the linked list in the right
     sector in the object array: */

  array_x = obj_realToSectorX(obj->pos.x);
  array_y = obj_realToSectorY(obj->pos.y);

  /*Insert the object in the front of the linked list:*/

  obj->next = OBJ_AT(obj->layer, array_x, array_y);
  OBJ_AT(obj->layer, array_x, array_y) = obj;
}

/* removeObj
   removes an object from the object container (but does not free it)
*/

void
removeObj(Object *obj)
{

  extern ObjContainer the_objects;
  int array_x, array_y;
  Object *this_obj, *last_obj;

  // Find the sector the object is in:

  array_x = obj_realToSectorX(obj->pos.x);
  array_y = obj_realToSectorY(obj->pos.y);

  // Loop through the linked list in that sector until we find the
  // object, and delete it:

  this_obj = OBJ_AT(obj->layer, array_x, array_y);

  while (this_obj != obj)
    {
      last_obj = this_obj;
      this_obj = this_obj->next;
    }

  // If this object is the first one in the list, assign the pointer in the
  // array to the next object:
  if (OBJ_AT(obj->layer, array_x, array_y) == this_obj)
    {
      OBJ_AT(obj->layer, array_x, array_y) = this_obj->next;
    }
  // Otherwise, point the previous object in the list to the object after
  // this object:
  else
    {
      last_obj->next = this_obj->next;
    }

}

/* spriteNameToID
   Given the name of a sprite, search the sprite set for the corresponding id
*/
int
spriteNameToID(char *name)
{
  extern SpriteSet sprite_set;
  int i = 0;
  do
    {
      if (strcmp(name, sprite_set.data[i].name) == 0) return i;
    } while (i++ < sprite_set.n_sprites);

  /* Still haven't found the sprite of this name? Error! */
  fprintf(stderr, "Error: Sprite of name %s not loaded.\n", name);
  exit(0);
}

/* animNameToID
   Given the sprite ID of a sprite and the name of an animation, search the
   sprite data for the corresponding animation id
*/
int
animNameToID(int spr_id, char *name)
{
  extern SpriteSet sprite_set;

  int i = 0;
  do
    {
      if (strcmp(name, sprite_set.data[spr_id].data[i].name) == 0) return i;
    } while (i++ < sprite_set.data[spr_id].n_animations);

  /* Still haven't found the animation of this name? Error! */
  fprintf(stderr, "Error: Animation of name %s not loaded in sprite %s.\n", name, sprite_set.data[spr_id].name);
  exit(0);
}

/* obj_setSprite
   Given a sprite's name, set the object's sprite.
*/
void
obj_setSprite(char *name, Object *obj)
{

  /* Find the sprite id in the sprite set */
  obj->spr.spr_id = spriteNameToID(name);
}

/* obj_setAnim
   Given an animation's name, set the object's
   animation.
*/
void
obj_setAnim(char *name, Object *obj)
{
  extern SpriteSet sprite_set;

  /* Find the animation id in the sprite set */
  obj->spr.anim.anim_id = animNameToID(obj->spr.spr_id, name);
  obj->spr.anim.curr_frame = 0;
  obj->spr.anim.play_dir = FORWARD;
  obj->spr.anim.offset.x = 0;
  obj->spr.anim.offset.y = 0;

  /* Set the default animation speed: */
  time_init(&obj->spr.anim.timer, sprite_set.data[obj->spr.spr_id].data[obj->spr.anim.anim_id].def_delay);
}

/* obj_setAnimSpeed
   Set the animation speed of the current object's animation.
   Delay = 1/speed
*/
void
obj_setAnimSpeed(Object *obj, int speed)
{
  Time delay;
  delay = (speed == 0) ? 0 : 1 / (float) speed;
  time_setMax(&obj->spr.anim.timer, delay);
}

/* obj_animateObj
   Animate an object.
*/
void obj_animateObj(Object *obj)
{
  extern SpriteSet sprite_set;
  anim_animate(&obj->spr.anim, &sprite_set.data[obj->spr.spr_id].data[obj->spr.anim.anim_id]);
}

/* obj_getLayerWidth
   Gets the width in sectors of the layer.
*/
int
obj_getLayerWidth(int l)
{
  extern ObjContainer the_objects;
  return the_objects.layers[l].w;
}

/* obj_getLayerHeight
   Gets the height in sectors of the layer.
*/
int
obj_getLayerHeight(int l)
{
  extern ObjContainer the_objects;
  return the_objects.layers[l].h;
}

/* obj_getObjList
   Returns the list of objects in a sector.
*/
Object *
obj_getObjList(int l, int x, int y)
{
  extern ObjContainer the_objects;
  return OBJ_AT(l, x, y);
}

/* obj_getNextObj
   given a pointer to an object, gets the next object in that linked list
*/
Object *
obj_getNextObj(Object *this_object)
{
  return this_object->next;
}

/* obj_getGfxPos

   given a pointer to an object, calculate the upper-left coordinates of the
   object's sprite's current frame, considering the frame's offset, the
   sprite's offset, and the position of the object.

*/
Point
obj_getGfxPos(Object *obj)
{

  extern SpriteSet sprite_set;
  Point gfx_pos;

  gfx_pos.x = obj_getObjTopLeft(obj).x + 
    obj->spr.anim.offset.x +
    sprite_set.data[obj->spr.spr_id].data[obj->spr.anim.anim_id].frames[obj->spr.anim.curr_frame].offset.x;

  gfx_pos.y = obj_getObjTopLeft(obj).y + 
    obj->spr.anim.offset.y +
    sprite_set.data[obj->spr.spr_id].data[obj->spr.anim.anim_id].frames[obj->spr.anim.curr_frame].offset.y;
  
  return gfx_pos;
}

/* obj_getObjTopLeft
   given a pointer to an object, return the object's
   top left point, in real coordinates
*/
Point
obj_getObjTopLeft(Object *obj)
{
  Point top_left;
  top_left.x = obj->pos.x - obj->w / 2;
  top_left.y = obj->pos.y - obj->h / 2;
  return top_left;
}

/* obj_getObjGfx
   Return the object's currently displayed graphic.
*/
SDL_Surface *
obj_getObjGfx(Object *obj)
{
  extern SpriteSet sprite_set;
  return sprite_set.data[obj->spr.spr_id].data[obj->spr.anim.anim_id].frames[obj->spr.anim.curr_frame].image;

}

/* obj_getObjBounds
   given a pointer to an object, returns a pointer to the object's
   boundaries
*/
Bound *
obj_getObjBounds(Object *object_ptr)
{
  return object_ptr->bounds;
}

/* obj_getObjPos
   gets the position of an object
*/

Point
obj_getObjPos(Object *object_ptr)
{
  return object_ptr->pos;
}

/* obj_getObjLayer
   Gets the layer the object is in.
*/
int
obj_getObjLayer(Object *obj)
{
  return obj->layer;
}

/* obj_getObjBotRight
   given a pointer to an object, return the object's
   bottom right point, in real coordinates
*/
Point
obj_getObjBotRight(Object *object_ptr)
{
  Point bot_right;
  bot_right.x = object_ptr->pos.x + object_ptr->w / 2;
  bot_right.y = object_ptr->pos.y + object_ptr->h / 2;
  return bot_right;
}

/* obj_getObjVel
   gets the velocity of an object
*/
Velocity
obj_getObjVel(Object *object_ptr)
{
  return object_ptr->vel;
}

/* obj_getObjMass
   gets the mass of an object
*/
int
obj_getObjMass(Object *object_ptr)
{
  return object_ptr->mass;
}

/* obj_getObjElasticity
   gets the elasticity of an object
*/
float
obj_getObjElasticity(Object *object_ptr)
{
  return object_ptr->elasticity;
}

/* obj_getObjType
   returns the object's type
*/
int
obj_getObjType(Object *obj)
{
  return obj->type;
}

/* obj_setObjVel
   sets the velocity of an object
*/
void
obj_setObjVel(Object *object_ptr, Velocity vel)
{
  object_ptr->vel = vel;
}

/* obj_isSolid
   True if the object is solid
*/
int obj_isSolid(Object *obj)
{
  return obj->solid;
}

/* obj_sendObjSignal
   Add a signal onto the object's signal queue
*/
void obj_sendObjSignal(Object *obj, Signal *s)
{
  sig_push(&obj->signals, s);
}

/* obj_setObjPos
   sets the position of an object, making sure it is in the appropriate sector
*/
void
obj_setObjPos(Object *object_ptr, Point new_pos)
{
  /* Move it if the sector has changed: */
  if (obj_realToSectorX(new_pos.x) != obj_realToSectorX(object_ptr->pos.x)
      || obj_realToSectorY(new_pos.y) != obj_realToSectorY(object_ptr->pos.y))
    {
      
      /* Remove the object from its current sector: */
      removeObj(object_ptr);
      
      /* Actually update the position: */
      object_ptr->pos = new_pos;
      
      /* Insert the object into its new sector: */
      insertObj(object_ptr);
    }
  /* If it hasn't changed sectors just update the position: */
  else
    {
      object_ptr->pos = new_pos;
    }

}

/* obj_moveObj
   moves an object according to its current velocity, making sure it is in the
   appropriate sector
*/
void
obj_moveObj(Object *object_ptr, Time dt)
{

  /* Only do any of this if the object has velocity: */
  if (object_ptr->vel.x != 0 || object_ptr->vel.y != 0)
    {
      
      Point new_pos;    /* The tentative new position */

      object_ptr->fine_x += object_ptr->vel.x * dt * 1000;
      object_ptr->fine_y += object_ptr->vel.y * dt * 1000;

      new_pos = object_ptr->pos;

      
      /* Add the integer portion of the fine coordinate values to the actual
	 coordinates. */
      new_pos.x += (object_ptr->fine_x) / 1000;
      new_pos.y += (object_ptr->fine_y) / 1000;
	
      /* Remove the integer portion of the fine coordinate values */
      object_ptr->fine_x = object_ptr->fine_x % 1000;
      object_ptr->fine_y = object_ptr->fine_y % 1000;

      /* Make sure the object hasn't gone off the edge of the layer */
      if (new_pos.x < 0)
	{
	  new_pos.x = 0;
	  object_ptr->vel.x = 0;
	}
      else if (new_pos.x >= map_mapToRealX(map_getLayerWidth(object_ptr->layer)))
	{
	  new_pos.x = map_mapToRealX(map_getLayerWidth(object_ptr->layer)) - 1;
	  object_ptr->vel.x = 0;
	}
      if (new_pos.y < 0)
	{
	  new_pos.y = 0;
	  object_ptr->vel.y = 0;
	}
      else if (new_pos.y >= map_mapToRealY(map_getLayerHeight(object_ptr->layer)))
	{
	  new_pos.y = map_mapToRealY(map_getLayerHeight(object_ptr->layer)) - 1;
	  object_ptr->vel.y = 0;
	}
      
      obj_setObjPos(object_ptr, new_pos);

    }

}

/* setPlayerPtr
   sets a pointer to the player object
*/
void
setPlayerPtr(Object *ptr)
{
  extern Object *player_pointer;
  player_pointer = ptr;
}

/* obj_getPlayerPtr
   gets a pointer to the player object
*/
Object *
obj_getPlayerPtr(void)
{
  extern Object *player_pointer;
  return player_pointer;
}

/* obj_makeSound
   Given the name of a sound, play the sound, mixing it for the object's
   position on screen.

   Returns the number of the channel the sound is played on.
*/
int
obj_makeSound(Object *obj, char *sound, int loops)
{
  int pan;
  Rect cam_range = cam_getViewRange(obj->layer);

  /* If the object is offscreen, set the pan to the min or max */
  if (obj->pos.x <= cam_range.p1.x)
    pan = 0;
  else if (obj->pos.x >= cam_range.p2.x)
    pan = PANNING_MAX;

  /* The object is onscreen, find the right amount of panning */
  else
    {
      pan = (int) (PANNING_MAX * (float) (obj->pos.x - cam_range.p1.x) / (float) (cam_range.p2.x - cam_range.p1.x));
    }
  return (aud_playSound(sound, pan, SOUND_DEFAULT_VOL, loops));
}
