#include "map.h"
#include "camera.h"

/* The tile specific data is in this file: */
#include "types/tiletypes.h"
/* This structure from tiledata.c contains a list of pointers to tile-def
   structures which each tile-type .c file contains. */
extern struct tile_att_define *tile_defs[];

/* The map for 1 area.  (Only 1 for now.) */
static Map map;

/* The tile animation data: */
static Tileset tileset;

/* Private function prototypes */
static int animNameToID(char *name);
static void freeTile(int l, int x, int y);

/* animNameToID
   Searches the loaded tileset array for an animation with the given name,
   returns the number of the cell which that animation was found in.
*/
int
animNameToID(char *name)
{
  extern Tileset tileset;
  int i = 0;
  do
    {
      if (strcmp(name, tileset.data[i].name) == 0) return i;
    } while (i++ < tileset.n_animations);

  /* Still haven't found the animation of this name? Error! */
  fprintf(stderr, "Error: Animation of name %s not loaded.\n", name);
  exit(0);
}

/* Return the number of layers */
int
map_getNLayers(void)
{
  extern Map map;
  return map.n_layers;
}

/* map_getBackgroundColor
   Return the background color.
*/
Color *
map_getBackgroundColor(void)
{
  extern Map map;
  return &map.bg_color;
}

/* Return the width of a layer */
int
map_getLayerWidth(int z)
{
  extern Map map;
  return map.layers[z].w;
}

/* Return the height of a layer */
int
map_getLayerHeight(int z)
{
  extern Map map;
  return map.layers[z].h;
}

/* map_getTileGfx
   Returns a pointer to an SDL_Surface of the currently displayed graphic of
   a tile, or NULL if there is no tile.
*/
SDL_Surface *
map_getTileGfx(int z, int x, int y)
{
  extern Tileset tileset;
  extern Map map;

  return (TILE_AT(z, x, y) == NULL) ? NULL : tileset.data[TILE_AT(z, x, y)->anim.anim_id].frames[TILE_AT(z, x, y)->anim.curr_frame].image;
}

/* map_getTileGfxOffset
   Gets the offset
   of the tile's current graphic.
*/
Point
map_getTileGfxOffset(int z, int x, int y)
{
  extern Map map;
  extern Tileset tileset;
  Point p;

  p.x = TILE_AT(z, x, y)->anim.offset.x + tileset.data[TILE_AT(z, x, y)->anim.anim_id].frames[TILE_AT(z, x, y)->anim.curr_frame].offset.x;
  p.y = TILE_AT(z, x, y)->anim.offset.y + tileset.data[TILE_AT(z, x, y)->anim.anim_id].frames[TILE_AT(z, x, y)->anim.curr_frame].offset.y;

  return p;
}

/* map_getTileBounds
   Returns a pointer to the boundary list of a tile, or NULL
   of there is no tile.
*/
Bound *
map_getTileBounds(int z, int x, int y)
{
  extern Map map;
  return (TILE_AT(z, x, y) == NULL) ? NULL : TILE_AT(z, x, y)->bounds;
}

/* map_getTileType
   Returns a tile's type.
*/
int map_getTileType(int z, int x, int y)
{
  extern Map map;
  return (TILE_AT(z, x, y)->type);
}
/* Load the tileset for an area */
void
map_loadTileset(char *areafile)
{

  extern Tileset tileset;
  char *tileset_datfile;
  char *anim_name, *anim_dir;
  int i = 0;

  tileset_datfile = file_getAreaTilesetFile(areafile);

  /* Count the number of animations in this tileset and allocate the tileset
     array */
  tileset.n_animations = file_countDatPairs(tileset_datfile);
  tileset.data = (AnimData *) dyn_1dArrayAlloc(tileset.n_animations, sizeof(AnimData));
  

  /* Open the datfile for reading */
  file_openFile(tileset_datfile, 'r');

  while (file_getNextDatPair(tileset_datfile, &anim_name, &anim_dir))
    {
      /* Load the animation */
      file_loadAnim(anim_name, anim_dir, &tileset.data[i]);
      free(anim_name); free(anim_dir);
      i++;
    }

  /* Close the datfile */
  file_closeFile(tileset_datfile);

  free(tileset_datfile);

}

/* map_freeTileset
   Frees the loaded tileset.
*/
void
map_freeTileset(void)
{
  extern Tileset tileset;
  int i;

  for (i = 0; i < tileset.n_animations; i++)
    {
      anim_freeAnim(&tileset.data[i]);
    }
  dyn_1dArrayFree(tileset.data);
}

/* map_loadMap
   Loads the map for an area.
*/
void
map_loadMap(char *areafile)
{
  extern Map map;
  extern Tileset tileset;
  extern struct tile_att_define *tile_defs[];
  int i;

  /* Open the area file */
  file_openFile(areafile, 'r');

  /* Get the background color */
  map.bg_color = file_getAreaBackgroundColor(areafile);

  /* Get the number of layers */
  map.n_layers = file_getAreaNLayers(areafile);

  /* Allocate the array of layers */
  map.layers = (Layer *) dyn_1dArrayAlloc(map.n_layers, sizeof(Layer));

  /* Create the layers */
  for (i = 0; i < map.n_layers; i++)
    {
      int w, h;
      /* Find the next layer in the area file */
      file_nextLayer(areafile);
      /* Get the layer's dimensions. */
      file_getLayerDims(areafile, &w, &h);
      map.layers[i].w = w;
      map.layers[i].h = h;

      /* Allocate the layer data array */
      map.layers[i].data = (Tile ***) dyn_arrayAlloc(map.layers[i].w, map.layers[i].h, sizeof(Tile *));


      /* Get all the tiles in the layer */
      while (file_nextTile(areafile))
	{
	  Bound *b;
	  int x, y, type;
	  char *anim_name;

	  /* Get the tile's attributes */
	  file_getTileAtts(areafile, &x, &y, &type, &anim_name);

	  /* Allocate the tile in the proper place in the array */
	  MALLOC(TILE_AT(i, x, y), sizeof(Tile));
	  
	  /* Initialize the signal queue */
	  sig_initQ(&TILE_AT(i, x, y)->signals);

	  /* Set the tile's type: */
	  TILE_AT(i, x, y)->type = type;

	  /* Get tile-specific attributes from the definitions included in
	     tiledata.c */
	  TILE_AT(i, x, y)->elasticity = tile_defs[type]->elasticity;
	  TILE_AT(i, x, y)->solid = tile_defs[type]->solid;
	  TILE_AT(i, x, y)->friction = tile_defs[type]->friction;
	  if (TILE_AT(i, x, y)->active = tile_defs[type]->active)
	    {
	      /* Only assign the tile functions if it is active */
	      TILE_AT(i, x, y)->atts = tile_defs[type]->init_atts();
	      TILE_AT(i, x, y)->go = tile_defs[type]->go;
	      TILE_AT(i, x, y)->free_atts = tile_defs[type]->free_atts;
	    }

	  /* Set the tile's animation: */
	  TILE_AT(i, x, y)->anim.anim_id = animNameToID(anim_name);
	  free(anim_name);
	  TILE_AT(i, x, y)->anim.curr_frame = 0;
	  TILE_AT(i, x, y)->anim.play_dir = 0;
	  TILE_AT(i, x, y)->anim.offset.x = 0;
	  TILE_AT(i, x, y)->anim.offset.y = 0;
	  /* Get the animation speed from the animation's default: */
	  time_init(&TILE_AT(i, x, y)->anim.timer, tileset.data[TILE_AT(i, x, y)->anim.anim_id].def_delay);

	  /* Get all of the boundaries for this tile */
	  TILE_AT(i, x, y)->bounds = NULL;
	  while (file_nextTileBound(areafile))
	    {
	      /* If this is the head of the list: */
	      if (TILE_AT(i, x, y)->bounds == NULL)
		{
		  MALLOC(TILE_AT(i, x, y)->bounds, sizeof(Bound));
		  b = TILE_AT(i, x, y)->bounds;
		}
	      else
		{
		  MALLOC(b->next, sizeof(Bound));
		  b = b->next;
		}
	      /* Read the boundary in */
	      file_getTileBound(areafile, b);
	      b->next = NULL;
	    }

	}  /* Found all the tiles in the layer */

    }  /* Found all the layers */

  file_closeFile(areafile);

}

/* freeTile
   Frees a tile.
*/
void
freeTile(int l, int x, int y)
{

  Tile *t = TILE_AT(l, x, y);
  /* Free its boundaries */
  Bound *b = t->bounds;
  while (b != NULL)
    {
      Bound *next_bound = b->next;
      free(b);
      b = next_bound;
    }

  /* Flush the signal queue */
  sig_flush(&t->signals);

  /* If the object is active, it may have allocated its own type-specific
     attributes which it will free itself: */
  if (t->active) t->free_atts(l, x, y);

  free(t);
  TILE_AT(l, x, y) = NULL;
}

/* map_freeMap
   Frees the loaded map.
*/
void
map_freeMap(void)
{
  extern Map map;
  int i;

  /* Free it layer by layer */
  for (i = 0; i < map.n_layers; i++)
    {
      int x, y;

      /* Free all of the tiles in the layer */
      for (x = 0; x < map.layers[i].w; x++)
	{
	  for (y = 0; y < map.layers[i].h; y++)
	    {
	      /* Is there a tile here? */
	      if (TILE_AT(i, x, y) != NULL)
		{
		  freeTile(i, x, y);
		}
	    }
	}
      /* Free the array */
      dyn_arrayFree((void **) map.layers[i].data, map.layers[i].w);
    }
  /* Free the array of layers */
  dyn_1dArrayFree(map.layers);
}

/* map_sendTileSignal
   Sends a signal to an active tile.
*/
void
map_sendTileSignal(int z, int x, int y, Signal *s)
{
  extern Map map;

  if (TILE_AT(z, x, y)->active)
    sig_push(&TILE_AT(z, x, y)->signals, s);
}

/* map_tileIsSolid
   Returns true if a tile is solid
*/
int map_tileIsSolid(int z, int x, int y)
{
  extern Map map;
  return (TILE_AT(z, x, y)->solid);
}


/* map_runTiles
   Has tiles which are within our specified range run and animate
*/
void
map_runTiles(void)
{
  extern Map map;
  extern Tileset tileset;
  int l, x, y;

  for (l = 0; l < map.n_layers; l++)
    {
      /* Get the range of real coordinates on this layer the camera can see */
      Rect cam_range = cam_getViewRange(l);

      /* Loop through all of the onscreen tiles, plus a certain range beyond
	 that */
      for (x = (map_realToMapX(cam_range.p1.x) - TILE_X_RANGE) >= 0 ?
	     (map_realToMapX(cam_range.p1.x) - TILE_X_RANGE) : 0;
	   (x <= map_realToMapX(cam_range.p2.x) + TILE_X_RANGE)
	     && (x < map_getLayerWidth(l));
	   x++)
	{
	  for (y = (map_realToMapY(cam_range.p1.y) - TILE_Y_RANGE) >= 0 ?
		 (map_realToMapY(cam_range.p1.y) - TILE_Y_RANGE) : 0;
	       (y <= map_realToMapY(cam_range.p2.y) + TILE_Y_RANGE)
		 && (y < map_getLayerHeight(l));
	       y++)
	    {

	      /* If there is a tile here */
	      if (TILE_AT(l, x, y) != NULL)
		{
		  /* Have the tile do its go action if it's active */
		  if (TILE_AT(l, x, y)->active)
		    TILE_AT(l, x, y)->go(l, x, y);

		  /* Animate */
		  anim_animate(&TILE_AT(l, x, y)->anim, &tileset.data[TILE_AT(l, x, y)->anim.anim_id]);
		}
	    }
	}
    }
}

