#ifndef __DEFINED_FILE_H
#define __DEFINED_FILE_H

#include <stdio.h>
#include "defs.h"
#include "graphics.h"
#include "animation.h"

/* File types that file.c works with:

   .dat files.  These files contain lists of name and value pairs.  For example
   levels.dat would contain lists of the names of levels and their associated
   filenames.  The outside world does not care what format .dat files take.

   .area files.  These files contain the actual level data.  Again, the outside
   world does not care about the format of these files.

   anim files.  These contain information about animations.

   For now, because I'm lazy, all path names are relative to the root data
   directory path.

*/

/* Open files are kept in a linked list.  Each node has the name of the file
   and the pointer to the file. */
typedef struct openfile_struct
{
  char *name;
  FILE *fp;
  struct openfile_struct *next;
} OpenFile;

/* String for opening files in read mode: */
#define READ_MODE "r"
/* Write mode */
#define WRITE_MODE "w"

/* Open files, exiting on failure. */
#define FOPEN(name, pointer, mode) do { \
if (((pointer) = fopen((name), (mode))) == NULL) \
{ \
  fprintf(stderr, "Unable to open file: %s\n", name); \
  exit(0); \
} \
} while(0)

#define DATA_PREFIX "data/"
#define MUSIC_DATFILE "music/music.dat"
#define TILESET_DATFILE "tilesets/tilesets.dat"
#define ANIM_FILENAME "anim"
#define SPRITE_FILENAME "sprite"

extern char *file_getDatValue(char *file, char *name);
extern char *file_getAreaMusicFile(char *areafile);
extern char *file_getAreaTilesetFile(char *areafile);
extern void file_openFile(char *file, char mode);
extern void file_closeFile(char *file);
extern void file_openSpriteFile(char *dir, char mode);
extern void file_closeSpriteFile(char *dir);
extern int file_getNextDatPair(char *file, char **name, char **value);
extern int file_getNextSound(char *file, char **name, char **sound_file);
extern int file_countDatPairs(char *file);
extern void file_loadAnim(char *name, char *dir, AnimData *anim);
extern int file_getAreaNLayers(char *file);
extern Color file_getAreaBackgroundColor(char *file);
extern void file_nextLayer(char *file);
extern void file_getLayerDims(char *file, int *w, int *h);
extern int file_nextTile(char *file);
extern void file_getTileAtts(char *file, int *x, int *y, int *type, char **anim_name);
extern int file_nextTileBound(char *file);
extern void file_getTileBound(char *file, Bound *b);
extern int file_countSpriteAnims(char *dir);
extern int file_getNextSpriteAnim(char *dir, char **anim_name, char **anim_dir);
extern int file_nextObject(char *file);
extern void file_getObjectAtts(char *file, int *z, Point *pos, Velocity *vel, int *type);

#endif /* __DEFINED_FILE_H */
