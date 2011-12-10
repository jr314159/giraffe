#ifndef __DEFINED_AUDIO_H
#define __DEFINED_AUDIO_H

#include <stdio.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "file.h"

/* The sound playing is pretty simple.  The audio module just plays a sound
   and forgets about it, and relies on the module calling it to stop the sound
   if it's looping or whatever.  What would be better but more complicated
   would be for the audio module to keep some sort of list of the channels
   it's playing and the things associated with those sounds so that the 
   audio could automatically update the panning as objects with looping sounds
   moved around, or stop a sound when an object moved too far off the screen.
*/

#define AUDIO_CHUNKSIZE 4096
#define MUSIC_DEFAULT_VOL (MIX_MAX_VOLUME * .5)
#define SOUND_DEFAULT_VOL (MIX_MAX_VOLUME * .8)
#define PANNING_MAX 254

/* Sounds are stored in a binary tree */
typedef struct sound_node
{
  char *name;                 /* The name of the sound */
  Mix_Chunk *sound;           /* The sound */
  struct sound_node *left;
  struct sound_node *right;
} SoundNode;

typedef SoundNode *SoundTree;


extern void aud_playMusic(int loops);
extern void aud_haltMusic(void);
extern void aud_loadMusic(char *areafile);
extern void aud_freeMusic(void);
extern void aud_init(void);
extern void aud_close(void);
extern void aud_setMusicVol(int volume);
extern void aud_loadSounds(char *datfile);
extern int aud_playSound(char *name, int pan, int vol, int loops);
#define aud_haltSound(x) Mix_HaltChannel(x)

#endif /* __DEFINED_AUDIO_H */
