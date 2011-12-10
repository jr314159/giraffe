#include <stdio.h>
#include "SDL.h"
#include "map.h"
#include "object.h"
#include "collision.h"
#include "file.h"
#include "defs.h"
#include "audio.h"
#include "camera.h"
#include "input.h"
#include "signal.h"

/* If more than this number of seconds passes during a cycle, the game will
   run slowly : */
#define MAX_ELAPSED_TIME .01


#define LEVELS_DAT "levels/levels.dat"
#define SPRITES_DAT "sprites/sprites.dat"
#define SOUND_DAT "sounds/sounds.dat"


/* Here's our code for running the world. */
/* This is what I have in "The Process, v2" in my notes, more or less: */
void
runTheWorld(Time dt)
{

  /* For now anyway, just run the world in the layer that the player is in.
     If I decide in the future to have action in the other layers, I'll have
     to add another for loop here: */
  int l = obj_getObjLayer(obj_getPlayerPtr());

  /* Don't get stuck in infinite collision loops: */
  int loop_timeout = 0;

  /* Say we have some omniscient being, who runs the world.  Let's call him
     god, and have him looking at a particular sector: */
  Point god_sector;
  Point curr_sector;

  Object *this_object = NULL;
  
  
  // Set god's position to the same position as the camera:
  god_sector.x = obj_realToSectorX(cam_getCameraPos().x);
  god_sector.y = obj_realToSectorY(cam_getCameraPos().y);
  
  /* Apply velocity for all objects within our desired range of sectors: */
  for (curr_sector.y = (god_sector.y - SECTOR_Y_RANGE >= 0) ? god_sector.y - SECTOR_Y_RANGE : 0;
       curr_sector.y <= god_sector.y + SECTOR_Y_RANGE && curr_sector.y < obj_getLayerHeight(l);
       curr_sector.y++
       )
    {
      for (curr_sector.x = (god_sector.x - SECTOR_X_RANGE >= 0) ? god_sector.x - SECTOR_X_RANGE : 0;
	   curr_sector.x <= god_sector.x + SECTOR_X_RANGE && curr_sector.x < obj_getLayerWidth(l);
	   curr_sector.x++
	   )
	{
	  // Loop through all of the objects in the sector:  
	  
	  this_object = obj_getObjList(l, curr_sector.x, curr_sector.y);
	  
	  while (this_object != NULL)
	    {
	      
	      // Apply the object's velocity:

	      obj_moveObj(this_object, dt);
	      
	      // get the next object
	      this_object = obj_getNextObj(this_object);
	    }
	  
	}
      
    }  /* End of for loops */



  /* Detect collisions for each object within our desired range of sectors: */
  for (curr_sector.y = (god_sector.y - SECTOR_Y_RANGE >= 0) ? god_sector.y - SECTOR_Y_RANGE : 0;
       curr_sector.y <= god_sector.y + SECTOR_Y_RANGE && curr_sector.y < obj_getLayerHeight(l);
       curr_sector.y++
       )
    {
      for (curr_sector.x = (god_sector.x - SECTOR_X_RANGE >= 0) ? god_sector.x - SECTOR_X_RANGE : 0;
	   curr_sector.x <= god_sector.x + SECTOR_X_RANGE && curr_sector.x < obj_getLayerWidth(l);
	   curr_sector.x++
	   )
	
	{

	  // Loop through all of the objects in the sector:  

	  this_object = obj_getObjList(l, curr_sector.x, curr_sector.y);

	  /* If there are objects in this sector: */
	  while (this_object != NULL)
	    {
	      col_doCollisions(this_object, dt);
	      
	      // get the next object
	      this_object = obj_getNextObj(this_object);
	    }
	  
	}
      
    }  /* End of for loops */


  /* Have visible tiles do their go() function and animate: */
  map_runTiles();

  /* Have each object do its go() function and animate: */
  for (curr_sector.y = (god_sector.y - SECTOR_Y_RANGE > 0) ? god_sector.y - SECTOR_Y_RANGE : 0;
       curr_sector.y <= god_sector.y + SECTOR_Y_RANGE && curr_sector.y < obj_getLayerHeight(l);
       curr_sector.y++
       )
    {
      for (curr_sector.x = (god_sector.x - SECTOR_X_RANGE > 0) ? god_sector.x - SECTOR_X_RANGE : 0;
	   curr_sector.x <= god_sector.x + SECTOR_X_RANGE && curr_sector.x < obj_getLayerWidth(l);
	   curr_sector.x++
	   )
	
	{
	  // Loop through all of the objects in the sector:  
	  
	  this_object = obj_getObjList(l, curr_sector.x, curr_sector.y);

	  while (this_object != NULL)
	    {
	      

	      // Have the object do its think code or whatever it does:
	      this_object->go(this_object, dt);

	      /* Animate */
	      obj_animateObj(this_object);

	      // get the next object
	      this_object = obj_getNextObj(this_object);
	    }
	  
	}
      
    }  /* End of for loops */

  /* Objects may send signals to the object module during their turn: */
  obj_handleSignals();

}


int main(int argc, char *argv[])
{

  int xres, yres, fullscreen;
  SDL_Event event;
  int quit = 0;
  Timer main_timer;
  char *levelfile, *areafile;

  Object *player_ptr; // A pointer to the player object

  // Get resolution from command line options -- default to 640x480
  if (argc >= 3) {
    if (!(xres = atoi(argv[1]))) xres = 640;
    if (!(yres = atoi(argv[2]))) yres = 480;
  }
  else {
    xres = 640; yres = 480;
  }
  // Fullscreen? default no
  if (argc >= 4) {
    if (1 == atoi(argv[3])) fullscreen = 1;
    else fullscreen = 0;
  }
  else {
    fullscreen = 0;
  }

  /* Init SDL */
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(0);
  }
  atexit(SDL_Quit);
  printf("SDL Initialized\n");

  /* Set window caption */
  SDL_WM_SetCaption("The Giraffe Project", NULL);
  /* Init graphics */
  gfx_initScreen(xres, yres, fullscreen);

  /* Init audio */
  aud_init();

  /* Start off on level 1, area 1 */

  /* Find the area filename */
  levelfile = file_getDatValue(LEVELS_DAT, "demo");
  areafile = file_getDatValue(levelfile, "demo_area");
  free(levelfile);

  /* Load the music for this area: */
  printf("Loading music...\n");
  aud_loadMusic(areafile);

  /* Load sounds */
  printf("Loading sounds...\n");
  aud_loadSounds(SOUND_DAT);

  /* Load the tileset for this area: */
  printf("Loading tile animations...\n");
  map_loadTileset(areafile);

  printf("Loading map...\n");
  /* Load the map for this area: */
  map_loadMap(areafile);

  printf("Loading sprite animations...\n");
  /* Load the sprites */
  obj_loadSprites(SPRITES_DAT);

  printf("Loading objects...\n");
  /* Load the objects for this area: */
  obj_loadObjects(areafile);


  /* Play the music */
  aud_playMusic(-1);


  /* Get a pointer to the player */
  player_ptr = obj_getPlayerPtr();


  /* Set the camera to focus on the player */
  cam_setFocusLayer(obj_getObjLayer(player_ptr));

  cam_setCameraPos(obj_getObjPos(player_ptr).x, obj_getObjPos(player_ptr).y);

  /* Make the camera the size of the screen */
  cam_setCameraSize(xres, yres);


  /* Initialize the main timer: */
  time_init(&main_timer, MAX_ELAPSED_TIME);


  while (!quit)
    {


      runTheWorld(time_dt(&main_timer));
      time_update(&main_timer);


      /* Set the camera's position to center on the player */
      cam_setCameraPos(obj_getObjPos(player_ptr).x, obj_getObjPos(player_ptr).y);
      cam_render();

      while (SDL_PollEvent(&event))
	{
	  switch (event.type)
	    {
	    case SDL_QUIT:
	      quit = 1;
	      break;
	    case SDL_KEYDOWN:
	      switch( event.key.keysym.sym )
		{
		case SDLK_F4:
		  /* Toggle fullscreen */
		  gfx_toggleFullscreen();
		  break;
		case SDLK_ESCAPE:
		  quit = 1;
		  break;
		case SDLK_LEFT:
		  inp_setHoriz(LEFT);
		  break;
		case SDLK_RIGHT:
		  inp_setHoriz(RIGHT);
		  break;
		case SDLK_UP:
		  inp_setVert(UP);
		  break;
		case SDLK_DOWN:
		  inp_setVert(DOWN);
		  break;
		case SDLK_x:
		  inp_keyDown(JUMP_KEY);
		  break;
		case SDLK_z:
		  inp_keyDown(RUN_KEY);
		  inp_keyDown(SHOOT_KEY);
		default:
		  break;
		}
	      break;
	    case SDL_KEYUP:
	      switch( event.key.keysym.sym )
		{
		case SDLK_LEFT:
		  if (inp_getHoriz() == LEFT) inp_setHoriz(0);
		  break;
		case SDLK_RIGHT:
		  if (inp_getHoriz() == RIGHT) inp_setHoriz(0);
		  break;
		case SDLK_UP:
		  if (inp_getVert() == UP) inp_setVert(0);
		  break;
		case SDLK_DOWN:
		  if (inp_getVert() == DOWN) inp_setVert(0);
		  break;
		case SDLK_x:
		  inp_keyUp(JUMP_KEY);
		  break;
		case SDLK_z:
		  inp_keyUp(RUN_KEY);
		  inp_keyUp(SHOOT_KEY);
		default:
		  break;
		}
	      break;
	    default:
	      break;
	    }
	}
    }

  obj_freeObjects();
  printf("Objects freed.\n");

  obj_freeSprites();
  printf("Sprite animations freed.\n");

  map_freeMap();
  printf("Map freed.\n");

  map_freeTileset();
  printf("Tileset freed.\n");

  aud_freeMusic();
  aud_freeSounds();
  aud_close();
  printf("Audio freed.\n");

}
