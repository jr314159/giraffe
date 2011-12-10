#include "audio.h"

/* There is only one music file at a time, so we keep it here: */
static Mix_Music *music = NULL;

/* Sounds are stored in a binary tree */
static SoundTree sounds = NULL;

/* Private function prototypes */
SoundTree loadSound(SoundNode *node, char *name, char *filename);
void freeSound(SoundNode *node);
Mix_Chunk *findSound(SoundNode *node, char *name);

/* findSound
   Finds a sound in a tree.
*/
Mix_Chunk *findSound(SoundNode *node, char *name)
{
  int cond;

  if (node == NULL)
    return NULL;
  else if ((cond = strcmp(name, node->name)) == 0)
    {
      /* Found it */
      return node->sound;
    }
  else if (cond < 0)
    return findSound(node->left, name);
  else
    return findSound(node->right, name);
}

/* freeSound
   Frees a tree of sounds recursively.
*/
void freeSound(SoundNode *node)
{
  if (node != NULL)
    {
      freeSound(node->left);
      freeSound(node->right);

      /* Free the name */
      free(node->name);
      /* Free the sound */
      Mix_FreeChunk(node->sound);
      free(node);
      node = NULL;
    }
}

/* loadSound
   Add a node in the tree of sounds at or below the given node.

   Using recursive code from The C Programming Language.
*/
SoundTree loadSound(SoundNode *node, char *name, char *filename)
{

  int cond;

  if (node == NULL)
    {
      MALLOC(node, sizeof(SoundNode));
      node->name = name;
      node->sound = Mix_LoadWAV(filename);
      if (!node->sound)
	{
	  fprintf(stderr, "Error: Could not load soundfile %s: %s\n", filename, Mix_GetError());
	  exit(0);
	}
      node->left = node->right = NULL;
    }
  else if ((cond = strcmp(name, node->name)) == 0)
    {
      /* For some reason a sound of this name has already been loaded.
	 Don't a sound here.  Free the sound name string. */
      fprintf(stderr, "Warning: Attempt to load sound file %s with name %s, but a sound file with the same name has already been loaded. Not loading this sound.\n", filename, name);
      free(name);
    }
  else if (cond < 0)
    node->left = loadSound(node->left, name, filename);
  else
    node->right = loadSound(node->right, name, filename);

  return node;
}

/* aud_playSound
   Given a sound name, a stereo panning value from 0 to 255, and the number of
   loops, play a sound.

   Returns the number of the channel the sound is played on, or -1 on failure.
*/
int aud_playSound(char *name, int pan, int vol, int loops)
{
  extern SoundTree sounds;
  Mix_Chunk *chunk;
  int channel;


  /* Find the chunk in the tree */
  if ((chunk = findSound(sounds, name)) == NULL)
    {
      fprintf(stderr, "Error: Could not play unloaded sound \"%s\"\n", name);
      return -1;
    }
  else
    {
      /* Play the sound */
      channel = Mix_PlayChannel(-1, chunk, loops);
      if (channel == -1)
	{
	  fprintf(stderr, "Mix_PlayChannel: %s\n", Mix_GetError());
	}
      else
	{
	  /* Set the volume */
	  Mix_Volume(channel, vol);
	  /* Set the panning */
	  if (!Mix_SetPanning(channel, 254 - pan, pan))
	    fprintf(stderr, "Mix_SetPanning: %s\n", Mix_GetError());
	}
    }
  return channel;
}

/* aud_loadSounds
   Load all sounds into memory.
*/
void
aud_loadSounds(char *datfile)
{
  extern SoundTree sounds;
  char *sound_name, *sound_file;

  /* Open the datfile */
  file_openFile(datfile, 'r');

  while (file_getNextSound(datfile, &sound_name, &sound_file))
    {
      /* Load the sound */
      sounds = loadSound(sounds, sound_name, sound_file); 

      /* Free the string of the filename */
      free(sound_file);

    }

  /* Close the datfile */
  file_closeFile(datfile);
}

/* aud_freeSounds
   Free sounds from memory.
*/
void
aud_freeSounds(void)
{
  extern SoundTree sounds;

  /* Stop all playback */
  Mix_HaltChannel(-1);

  /* Recursively free the tree of sounds */
  freeSound(sounds);

}

/* aud_setMusicVol
   Sets the volume of the music.
*/
void
aud_setMusicVol(int volume)
{
  Mix_VolumeMusic(volume);
}


/* aud_playMusic
   Plays the loaded music.
*/
void
aud_playMusic(int loops)
{
  extern Mix_Music *music;
  if(Mix_PlayMusic(music, loops)==-1)
    {
      fprintf(stderr, "Mix_PlayMusic: %s\n", Mix_GetError());
    }
}

/* aud_haltMusic
   Halts the playback of music.
*/
void
aud_haltMusic(void)
{
  Mix_HaltMusic();
}

/* aud_loadMusic
   Load the music indicated in a given .area file.
*/
void
aud_loadMusic(char *areafile)
{
  extern Mix_Music *music;
  char *musicfile;

  /* Get the filename */
  musicfile = file_getAreaMusicFile(areafile);

  music = Mix_LoadMUS(musicfile);
  free(musicfile);

  if (!music)
    {
      fprintf(stderr, "Mix_LoadMUS(\"%s\"): %s\n", Mix_GetError());
      exit(0);
    }
}

/* aud_freeMusic
   Frees the music.
*/
void
aud_freeMusic(void)
{
  extern Mix_Music *music;
  Mix_FreeMusic(music);
  music = NULL;
}

/* aud_init
   Initialize the audio and mixer.
   SDL must be initialized before this function is called.
*/
void
aud_init(void)
{

  /* Initialize the SDL audio system if it has not already been initialized */
  if (!SDL_WasInit(SDL_INIT_AUDIO))
    {
      if(SDL_InitSubSystem(SDL_INIT_AUDIO)==-1) {
	fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
	exit(0);
      }
    }

  /* Initialize the SDL_mixer system */
  if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, AUDIO_CHUNKSIZE)==-1) {
    fprintf(stderr, "Mix_OpenAudio: %s\n", Mix_GetError());
    exit(0);
  }

  /* Set the default music volume */
  Mix_VolumeMusic(MUSIC_DEFAULT_VOL);

}

/* aud_close
   Close the audio system.
*/
void
aud_close(void)
{

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);

}

