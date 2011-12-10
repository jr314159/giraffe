#include "timer.h"

/* time_newTimer
   Initializes a new timer.
*/
void
time_init(Timer *t, Time max_elapsed)
{
  t->ticks = SDL_GetTicks();
  t->max_elapsed = max_elapsed;
  
}

/* time_getMax
   Returns max_elapsed.
*/
Time
time_getMax(Timer *t)
{
  return t->max_elapsed;
}

/* time_setMax
   Sets max_elapsed.
*/
void
time_setMax(Timer *t, Time max_elapsed)
{
  t->max_elapsed = max_elapsed;
}

/* time_dt
   Gets the elapsed time for a timer in milliseconds.
*/
Time
time_dt(Timer *t)
{
  Uint32 now = SDL_GetTicks();


  /* If there is no max_elapsed time limit, return the ticks passed. */
  if (t->max_elapsed == 0) return ((now - t->ticks) / UNIT_TIME);
  /* Otherwise, return the number of ticks passed up to the limit. */
  else return ((now - t->ticks) / UNIT_TIME > t->max_elapsed) ? t->max_elapsed : (now - t->ticks) / UNIT_TIME;

}

/* time_expired
   Returns true if the time elapsed is greater or equal to the max_elapsed time
*/
int
time_expired(Timer *t)
{
  return (time_dt(t) >= t->max_elapsed);
}

/* time_update
   Updates the ticks in a timer.
*/
void
time_update(Timer *t)
{
  t->ticks = SDL_GetTicks();
}


