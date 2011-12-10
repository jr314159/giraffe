#ifndef __DEFINED_MAP_H
#define __DEFINED_MAP_H

#include "animation.h"
#include "defs.h"
#include "dynarray.h"
#include "file.h"
#include "signal.h"

/* The map module:
   - Loads animations for tiles
   - Loads tiles into a 3-dimensional map array
   - Handles tiles and their actions
*/


/* The Tile is the map equivalent of an Object */
typedef struct tile_struct
{

  Animation anim;    /* The tile's animation */
  Bound *bounds;     /* The linked list of boundaries */

  int solid;         /* Whether or not objects can pass through the bounds */
  float elasticity;
  float friction;

  int type;          /* The tile's type */

  int active;        /* If a tile is active, it does things.  That means it
			receives signals, does its go() function, and has its
			own type-specific attributes */

  /* This is what the tile does every cycle */
  void (*go)(int l, int x, int y);

  /* The tile may have done its own memory allocation when it was created.
     If so, this frees whatever it allocated. */
  void (*free_atts)(int l, int x, int y);

  SigQ signals;      /* The tile accumulates signals in this queue */

  void *atts;

} Tile;

typedef struct layer_struct
{
  int w, h;     /* The layer's dimensions */
  Tile ***data;    /* The data is a 2d array of pointers to tiles. */
} Layer;

typedef struct map_struct
{
  int n_layers;     /* The number of layers */
  Color bg_color;   /* The RGB color of the background */
  Layer *layers;         /* A 1d array of the layers */
} Map;

typedef struct tileset_struct
{
  int n_animations;   /* The number of animations in a tileset */
  AnimData *data;        /* A 1d array of animations */
} Tileset;

/* A handy macro to get tile pointers in layer z, coordinate x, y */
#define TILE_AT(z, x, y) (map.layers[(z)].data[(x)][(y)])

/* Dimensions of tiles in real coordinates */
#define TILE_W 32
#define TILE_H 32

/* How many tiles off the screen we should work with in each dimension: */
#define TILE_X_RANGE 5
#define TILE_Y_RANGE 5

/* Coordinate conversion macros */
#define map_mapToRealX(x) ((x) * TILE_W)
#define map_mapToRealY(y) ((y) * TILE_H)
/* These two macros do integer division */
#define map_realToMapX(x) ((x) / TILE_W)
#define map_realToMapY(y) ((y) / TILE_H)
/* Find real coordinates within a tile: */
#define map_xInTile(x) ((x) % TILE_W)
#define map_yInTile(y) ((y) % TILE_W)

extern void map_loadTileset(char *areafile);
extern void map_freeTileset(void);
extern void map_loadMap(char *areafile);
extern void map_freeMap(void);
extern int map_getNLayers(void);
extern int map_getLayerHeight(int z);
extern int map_getLayerWidth(int z);
extern SDL_Surface *map_getTileGfx(int z, int x, int y);
extern Point map_getTileGfxOffset(int z, int x, int y);
extern Bound *map_getTileBounds(int z, int x, int y);
extern int map_getTileType(int z, int x, int y);
extern int map_tileIsSolid(int z, int x, int y);
extern void map_sendTileSignal(int z, int x, int y, Signal *s);
extern void map_runTiles(void);
extern Color *map_getBackgroundColor(void);

#endif /* __DEFINED_MAP_H */
