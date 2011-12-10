#include "file.h"

/* A linked list of the open files: */
static OpenFile *open_files = NULL;

/* Private function prototypes */
char *addDataPrefix(char *f);
char *trimString(char *s);
OpenFile *findOpenFile(char *file);
char *getSpriteFilename(char *dir);

/* trimString
   Given a character buffer with a string in it, create a new string
   that is just the right length
*/

char *
trimString(char *s)
{

  char *trimmed;
  MALLOC(trimmed, (strlen(s) + 1) * sizeof(char));
  return(strcpy(trimmed, s));
}

/* addDataPrefix
   Concatenates the path to the data directory to the beginning of a filename.
*/
char *
addDataPrefix(char *f)
{

  char *filename;
  MALLOC(filename, (strlen(f) + strlen(DATA_PREFIX) + 1) * sizeof(char));
  strcpy(filename, DATA_PREFIX);
  return (strcat(filename, f));
}

/* getSpriteFilename
   Given a sprite's directory name, SPRITE_FILENAME onto the end.
*/
char *
getSpriteFilename(char *dir)
{
  char *sprite_file;
  MALLOC(sprite_file, (strlen(dir) + strlen(SPRITE_FILENAME) + 1) * sizeof(char));
  strcpy(sprite_file, dir);
  return (strcat(sprite_file, SPRITE_FILENAME));
}


/* findOpenFile
   Given the name of an open file, get the OpenFile from the list.
*/
OpenFile *
findOpenFile(char *file)
{
  extern OpenFile *open_files;
  OpenFile *this_file = open_files;

  /* Find the open file */
  while (strcmp(this_file->name, file) != 0 && this_file != NULL)
    this_file = this_file->next;

  /* In case the file specified hasn't been opened yet : */
  if (this_file == NULL)
    {
      fprintf(stderr, "Error: Cannot find unopened file: %s\n", file);
      exit(0);
    }

  return this_file;
}


/* file_getAreaTilesetFile
   Gets the filename of the dat file for the tileset for an area.
*/
char *
file_getAreaTilesetFile(char *areafile)
{

  FILE *inf;

  char buffer[128], tilebuffer[64];
  char *tileset_name = NULL;
  char *areafilename = addDataPrefix(areafile);

  FOPEN(areafilename, inf, READ_MODE);
  free(areafilename);

  /* Search the area file line by line for the tileset name */
  /* The line should look like this:
     tileset "tilesetname"
  */
  while (fgets(buffer, 128, inf) != NULL && tileset_name == NULL)
    {
      if (sscanf(buffer, "tileset %*[\"]%[^\"]%*[\"]", tilebuffer) == 1)
	{
	  tileset_name = (trimString(tilebuffer));
	}
    }

  fclose(inf);

  /* Couldn't find it! */
  if (tileset_name == NULL)
    {
      fprintf(stderr, "Error: Could not find tileset specification in %s\n", areafile);
      exit(0);
    }
  else
    {
      /* Look in tilesets.dat for the filename of the dat file for the
	 specified tileset.  Return the filename (without the full path)
      */
      char *tileset_file;
      tileset_file = file_getDatValue(TILESET_DATFILE, tileset_name);
      free (tileset_name);
      return tileset_file;
    }
}


/* file_getAreaMusicFile
   Gets the full path and filename of music for an area.
*/

char *
file_getAreaMusicFile(char *areafile)
{

  FILE *inf;

  char buffer[128], musbuffer[64];
  char *musicname = NULL;
  char *areafilename = addDataPrefix(areafile);


  FOPEN(areafilename, inf, READ_MODE);
  free(areafilename);

  /* Search the area file line by line for the music name */
  /* The line should look like this:
     music "musicname"
  */
  while (fgets(buffer, 128, inf) != NULL && musicname == NULL)
    {
      if (sscanf(buffer, "music %*[\"]%[^\"]%*[\"]", musbuffer) == 1)
	{
	  musicname = (trimString(musbuffer));
	}
    }

  fclose(inf);

  /* Couldn't find it! */
  if (musicname == NULL)
    {
      fprintf(stderr, "Error: Could not find music specification in %s\n", areafile);
      exit(0);
    }
  else
    {

      /* Look in music.dat for the music filename, return it and the
	 full path */
      char *musicfile;
      musicfile = addDataPrefix(file_getDatValue(MUSIC_DATFILE, musicname));
      free (musicname);
      return musicfile;
    }
}

/* file_openFile
   Opens a file.
*/
void
file_openFile(char *file, char mode)
{
  extern OpenFile *open_files;
  OpenFile *new_open_file;
  char *filename = addDataPrefix(file);

  /* Create a new node */
  MALLOC(new_open_file, sizeof(OpenFile));

  /* Open the file for reading or writing */
  if (mode == 'w') FOPEN(filename, new_open_file->fp, WRITE_MODE);
  else FOPEN(filename, new_open_file->fp, READ_MODE);
  free(filename);

  /* Store the name of the file */
  MALLOC(new_open_file->name, (strlen(file) + 1) * sizeof(char));
  strcpy(new_open_file->name, file);

  /* Insert the node in the beginning of the list */
  new_open_file->next = open_files;
  open_files = new_open_file;

}

/* file_openSpriteFile
   Opens a sprite file, given the sprite's directory.
*/
void
file_openSpriteFile(char *dir, char mode)
{

  char *sprite_file = getSpriteFilename(dir);
  file_openFile(sprite_file, mode);
  free(sprite_file);
}

/* file_closeSpriteFile
   Closes a sprite file, given the sprite's directory.
*/
void
file_closeSpriteFile(char *dir)
{
  char *sprite_file = getSpriteFilename(dir);
  file_closeFile(sprite_file);
  free(sprite_file);
}

/* file_closeFile
   Closes an open file.
*/
void
file_closeFile(char *file)
{
  extern OpenFile *open_files;
  OpenFile *last_file;
  OpenFile *this_file = open_files;

  /* Loop through the linked list of open files to find the file */
  while (strcmp(this_file->name, file) != 0)
    {
      last_file = this_file;
      this_file = this_file->next;
    }

  /* In case the file specified hasn't been opened yet : */
  if (this_file == NULL)
    {
      fprintf(stderr, "Error: Cannot close unopened file: %s\n", file);
      exit(0);
    }


  /* If this file is the head, assign the head to the next file: */
  if (this_file == open_files) open_files = this_file->next;
  /* Otherwise set the last file's next file to this file's next file */
  else last_file->next = this_file->next;

  /* Close the file and free it */
  free(this_file->name);
  fclose(this_file->fp);
  free(this_file);
}

/* file_getAreaNLayers
   Gets the number of layers from an open area file.
*/
int
file_getAreaNLayers(char *file)
{

  char buffer[128];
  OpenFile *area_file = findOpenFile(file);
  int n_layers;

  while (fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (sscanf(buffer, "layers %d", &n_layers) == 1) return n_layers;
    }

  fprintf(stderr, "Error: Couldn't find number of layers in area file %s\n", file);
  exit(0);
}

/* file_getAreaBackgroundColor
   Gets the background color of an area from an open file.
*/
Color
file_getAreaBackgroundColor(char *file)
{
  char buffer[128];
  OpenFile *area_file = findOpenFile(file);
  Color c;

  while (fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (sscanf(buffer, "bg_color %d %d %d", &c.r, &c.g, &c.b) == 3) return c;
    }

  fprintf(stderr, "Error: Couldn't find background color in area file %s\n", file);
  exit(0);
}

/* file_nextLayer
   Scan to the next layer in the open area file.
*/
void
file_nextLayer(char *file)
{
  char buffer[128];
  OpenFile *area_file = findOpenFile(file);

  /* A new layer is indicated by "layerstart" on a line by itself */
  while (fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (strstr(buffer, "layerstart") != NULL) return;
    }

  /* No layer found, error */
  fprintf(stderr, "Error: No more layers found in area file %s\n", file);
  exit(0);
}

/* file_nextTile
   Scan to the next tile, return 0 if there are more tiles in the layer.
*/
int
file_nextTile(char *file)
{
  char buffer[128];
  OpenFile *area_file = findOpenFile(file);

  /* A new tile is indicated by "tilestart" on a line by itself */
  while (fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (strstr(buffer, "tilestart") != NULL) return 1;

      /* If the line is "layerend", there are no more tiles in the layer */
      if (strstr(buffer, "layerend") != NULL) return 0;
    }
  return 0;
}

/* file_nextObject
   Scan to the next object, return 0 if there are no more tiles.
*/
int
file_nextObject(char *file)
{
  char buffer[128];
  OpenFile *area_file = findOpenFile(file);

  /* A new object is indicated by "objectstart" on a line by itself */
  while (fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (strstr(buffer, "objectstart") != NULL) return 1;

      /* If the line is "objectsend", there are no more objects */
      if (strstr(buffer, "objectsend") != NULL) return 0;
    }
  return 0;
}

/* file_nextTileBound
   Scan to the next boundary of the tile, return 0 if there are no more bounds.
*/
int
file_nextTileBound(char *file)
{
  char buffer[128];
  OpenFile *area_file = findOpenFile(file);

  /* A new bound is created by "boundstart" on a line by itslef */
  while (fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (strstr(buffer, "boundstart") != NULL) return 1;

      /* If the line is "tileend", there are no more boundaries */
      if (strstr(buffer, "tileend") != NULL) return 0;
    }
  return 0;
}

/* file_getTileBound
   Read the current boundary.
*/
void
file_getTileBound(char *file, Bound *b)
{

  char buffer[128];
  char type_buffer[64];
  OpenFile *area_file = findOpenFile(file);

  /* Find the type */
  while (fgets(buffer, 128, area_file->fp) != NULL) 
    {
      if (sscanf(buffer, "type %s", type_buffer) == 1) break;
    }
  
  if (strstr(type_buffer, "rect") != NULL) b->type = RECT;
  else if (strstr(type_buffer, "line") != NULL) b->type = LINE;
  else if (strstr(type_buffer, "circle") != NULL) b->type = CIRCLE;

  /* Get the boundary definition */
  while (fgets(buffer, 128, area_file->fp) != NULL)
    {
      switch (b->type)
	{
	case RECT:
	  if (sscanf(buffer, "%d %d %d %d", &b->b.rect.p1.x, &b->b.rect.p1.y, &b->b.rect.p2.x, &b->b.rect.p2.y) == 4) return;
	  break;
	case LINE:
	  if (sscanf(buffer, "%d %d %d %d", &b->b.line.p1.x, &b->b.line.p1.y, &b->b.line.p2.x, &b->b.line.p2.y) != 4) return;
	  break;
	case CIRCLE:
	  if (sscanf(buffer, "%d %d %d", &b->b.circle.p.x, &b->b.circle.p.y, &b->b.circle.r) != 3) return;
	  break;
	}
    }

  /* Still haven't found the proper boundary defs?  Error! */
  printf("Error: Could not find %s boundary definition in file %s.\n", type_buffer, file);
  exit(0);

}

/* file_getObjectAtts
   Read in the attributes of the current object.
*/
void
file_getObjectAtts(char *file, int *z, Point *pos, Velocity *vel, int *type)
{
  char buffer[128];
  int atts_found = 0;
  OpenFile *area_file = findOpenFile(file);

  while (atts_found < 3 && fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (sscanf(buffer, "pos %d %d %d", z, &pos->x, &pos->y) == 3) atts_found++;
      else if (sscanf(buffer, "vel %f %f", &vel->x, &vel->y) == 2) atts_found++;
      else if (sscanf(buffer, "type %d", type) == 1) atts_found++;
    }

  if (atts_found == 3) return;

  /* Haven't found all the attributes? Error! */
  fprintf(stderr, "Error: Could not find enough object attributes in file %s\n", file);
  exit(0);
}

/* file_getTileAtts
   Read the attributes of the current tile, not including boundaries.
*/
void
file_getTileAtts(char *file, int *x, int *y, int *type, char **anim_name)
{

  char buffer[128];
  char anim_name_buffer[64];
  int atts_found = 0;
  OpenFile *area_file = findOpenFile(file);

  while (atts_found < 3 && fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (sscanf(buffer, "pos %d %d", x, y) == 2) atts_found++;
      else if (sscanf(buffer, "type %d", type) == 1) atts_found++;
      else if (sscanf(buffer, "anim %*[\"]%[^\"]%*[\"]", anim_name_buffer) == 1)
	{
	  atts_found++;
	  *anim_name = trimString(anim_name_buffer);
	}
    }

  if (atts_found == 3) return;

  /* Haven't found all the attributes? Error! */
  fprintf(stderr, "Error: Could not find enough tile attributes in file %s\n", file);
  exit(0);

}

/* file_getLayerDims
   Gets the dimensions for the current layer.
*/
void
file_getLayerDims(char *file, int *w, int *h)
{
  char buffer[128];
  OpenFile *area_file = findOpenFile(file);
  while (fgets(buffer, 128, area_file->fp) != NULL)
    {
      if (sscanf(buffer, "dimensions %d %d", w, h) == 2) return;
    }

  /* No dimensions found, error */
  fprintf(stderr, "Error: No dimensions found in area file %s\n", file);
  exit(0);
}

/* file_getNextDatPair
   Gets the next data pair out of an open dat file.  Returns 0 when a pair
   cannot be found.
*/
int
file_getNextDatPair(char *file, char **name, char **value)
{
  OpenFile *dat_file;

  /* Temporary strings for name and value pairs */
  char n[64], v[64];

  dat_file = findOpenFile(file);

  /* Get the next line: */
  while (fscanf(dat_file->fp, "%*[\"]%[^\"]%*[\"] %*[\"]%[^\"]%*[\"]\n", n, v) == 2)
    {
      *name = trimString(n);
      *value = trimString(v);
      return 1;
    }

  /* None found, return 0 */
  return 0;
}

/* file_getNextSound
   Gets the next sound name and file out of a datfile.
*/
int
file_getNextSound(char *file, char **name, char **sound_file)
{
  char *short_filename;

  if (!file_getNextDatPair(file, name, &short_filename)) return 0;
  *sound_file = addDataPrefix(short_filename);
  free (short_filename);
  return 1;
}

/* file_getNextSpriteAnim
   Gets the next animation name and directory out of an open sprite file.
*/
int
file_getNextSpriteAnim(char *dir, char **anim_name, char **anim_dir)
{
  char n[64], v[64];
  char *sprite_filename = getSpriteFilename(dir);
  OpenFile *sprite_file = findOpenFile(sprite_filename);

  while (fscanf(sprite_file->fp, "%*[\"]%[^\"]%*[\"] %*[\"]%[^\"]%*[\"]\n", n, v) == 2)
    {

      *anim_name = trimString(n);
      MALLOC(*anim_dir, (strlen(v) + strlen(dir) + 1) * sizeof(char));
      strcpy(*anim_dir, dir);
      strcat(*anim_dir, v);
      return 1;
    }

  /* None found, return 0 */
  return 0;
}

/* file_countDatPairs
   Counts the number of name value pairs in a datfile.
*/
int
file_countDatPairs(char *file)
{
  FILE *inf;
  char *filename = addDataPrefix(file);
  int n = 0;
  FOPEN(filename, inf, READ_MODE);
  free(filename);

  while (fscanf(inf, "%*[\"]%*[^\"]%*[\"] %*[\"]%*[^\"]%*[\"]\n") != EOF)
    n++;

  fclose(inf);
  return n;
}

/* file_countSpriteAnims
   Counts the number of animations in a sprite file given the sprites directory
*/
int
file_countSpriteAnims(char *dir)
{
  int n;
  char *sprite_file = getSpriteFilename(dir);
  n = file_countDatPairs(sprite_file);
  free(sprite_file);
  return n;
}

/* file_getDatValue
   Searches through a dat file for a value to a name.
*/
char *
file_getDatValue(char *file, char *name)
{

  FILE *inf;
  /* Temporary strings for name and value pairs */
  char n[64], v[64];
  char *filename = addDataPrefix(file);

  FOPEN(filename, inf, READ_MODE);
  free(filename);

  /* Search line by line: */
  /* Lines are in this format:
     "name" "value"
  */
  while (fscanf(inf, "%*[\"]%[^\"]%*[\"] %*[\"]%[^\"]%*[\"]\n", n, v) == 2)
    {
      /* Return the value if the name matched */
      if (strcmp(name, n) == 0)
	{
	  fclose(inf);
	  return (trimString(v));
	}
    }
  /* Not found, error and quit */
  fclose(inf);
  fprintf(stderr, "Error: value for name \"%s\" in file \"%s\" not found.\n", name, file);
  exit(0);
}

/* file_loadAnim
   Given an animation's directory, the animations name, 
   and the address where the
   animation is to be loaded, load the animation.
*/
void
file_loadAnim(char *name, char *dir, AnimData *anim)
{
  FILE *inf;

  char *anim_dir = addDataPrefix(dir);
  char *anim_file;
  char buffer[128], img_buff[64];
  int x, y, play_mode, i = 0;

  /* Get the full path and filename of the animation file */
  MALLOC(anim_file, (strlen(anim_dir) + strlen(ANIM_FILENAME) + 1) * sizeof(char));
  strcpy(anim_file, anim_dir);
  strcat(anim_file, ANIM_FILENAME);

  /* Open the file */
  FOPEN(anim_file, inf, READ_MODE);
  free(anim_file);

  /* Set the animation's name: */
  MALLOC(anim->name, (strlen(name) + 1) * sizeof(char));
  strcpy(anim->name, name);

  /* Get the playback mode from the first line of the file */
  /* Format:
     play 0
  */
  if (fscanf(inf, "play %d ", &play_mode) != 1)
    {
      fprintf(stderr, "Error: Could not find the play mode in animation: %s\n", dir);
      exit(0);
    }
  /* Get the default delay from the 2nd line */
  /* Format:
     delay .25
  */
  if (fscanf(inf, "delay %f ", &anim->def_delay) != 1)
    {
      fprintf(stderr, "Error: Could not find the default delay in animation: %s\n", dir);
      exit(0);
    }


  anim->play_mode = (play_mode == 0) ? SINE : REPEAT;

  /* Count the number of frames */
  while (fscanf(inf, "%*[\"]%*[^\"]%*[\"] %*d %*d\n") != EOF)
    i++;
  
  anim->n_frames = i;

  /* Go back to the beginning of the file */
  rewind(inf);

  /* Allocate the array for the frames */
  anim->frames = (Frame *) dyn_1dArrayAlloc(anim->n_frames, sizeof(Frame));

  i = 0;
  /* Populate the frame array */
  /* Format of frame data:
     "gfx_file.png" 0 0
     where the two numbers are the x,y offset
  */
  while (fgets(buffer, 128, inf) != NULL)
    {
      if (sscanf(buffer, "%*[\"]%[^\"]%*[\"] %d %d", img_buff, &x, &y) == 3)
	{
	  char *gfx_file;
	  MALLOC(gfx_file, (strlen(img_buff) + strlen(anim_dir) + 1) * sizeof(char));
	  strcpy(gfx_file, anim_dir);
	  strcat(gfx_file, img_buff);

	  anim->frames[i].image = gfx_loadImage(gfx_file);
	  anim->frames[i].offset.x = x;
	  anim->frames[i].offset.y = y;

	  free(gfx_file);
	  i++;
	}
    }

  free(anim_dir);
  fclose(inf);

}



