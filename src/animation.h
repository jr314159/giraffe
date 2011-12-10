#ifndef __DEFINED_ANIMATION_H
#define __DEFINED_ANIMATION_H

#include "dynarray.h"
#include "graphics.h"
#include "SDL.h"
#include "defs.h"
#include "timer.h"

/* Animation Data "AnimData" is loaded into an array.  Things that have
   animations have "Animation"s, which point to AnimData.
*/

/* Two play modes for animations:
   SINE = play to the end of the frames and then back in reverse
   REPEAT = play to the end of the frames and start from the beginning
*/
enum play_modes {SINE, REPEAT};

/* An animation can either be playing forward or back */
enum play_dirs {FORWARD, BACK};

typedef struct frame_struct
{
  SDL_Surface *image;    /* A pointer to the frame's image */
  Point offset;          /* x,y offset of the image */
} Frame;

typedef struct animdata_struct
{
  char *name;       /* The name of the animation */
  int n_frames;     /* Number of frames */
  int play_mode;    /* Playback mode */
  Time def_delay;   /* The default delay between frames */
  Frame *frames;    /* A dynamic array containing the frames */
} AnimData;

typedef struct animation_struct
{
  int anim_id;      /* The id corresponds to the location of
		       AnimData in an array */
  int curr_frame;   /* The number of the current frame. */
  int play_dir;     /* Is it playing forwards or backwards? */
  Point offset;     /* x,y offset of the animation */
  Timer timer;      /* A timer for animation speed. */

} Animation;

extern void anim_freeAnim(AnimData *anim);
extern void anim_animate(Animation *anim, AnimData *data);

#endif /* __DEFINED_ANIMATION_H */
