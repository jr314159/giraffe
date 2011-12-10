#include "animation.h"

/* anim_freeAnim
   Frees animation data.
*/
void
anim_freeAnim(AnimData *anim)
{
  int i;
  /* Free the frames */
  for (i = 0; i < anim->n_frames; i++)
    {
      gfx_freeImage(anim->frames[i].image);
    }

  dyn_1dArrayFree(anim->frames);

  /* Free the name */
  free(anim->name);
}

/* anim_animate
   Advances an animation's current frame, given pointers to the animation
   and its animation data.
*/
void
anim_animate(Animation *anim, AnimData *data)
{

  /* Only do any of this if it's really an animation and not just 1 frame: */
  /* Only animate if the timer's max_elapsed is not 0 */
  /* Only animate if the time passed exceeds the specified delay */
  if (data->n_frames > 1 && time_getMax(&anim->timer) > 0 && time_expired(&anim->timer))
    {
      
      /* If the animation is playing forwards: */
      if (anim->play_dir == FORWARD)
	{

	  if (++(anim->curr_frame) >= data->n_frames)
	    {
	      if (data->play_mode == SINE)
		{
		  anim->play_dir = BACK;
		  anim->curr_frame -= 2;
		}
	      else
		{
		  anim->curr_frame = 0;
		}
	    }
	}
      // If the animation is playing backwards:
      else
	{
	  if (--(anim->curr_frame) < 0)	  
	    {
	      if (data->play_mode == SINE)
		{
		  anim->play_dir = FORWARD;
		  anim->curr_frame += 2;
		}
	      else
		{
		  anim->curr_frame = data->n_frames - 1;
		}
	    }
	}
      /* Update the timer: */
      time_update(&anim->timer);
    }
}
      
