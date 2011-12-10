#ifndef __DEFINED_TIMER_H
#define __DEFINED_TIMER_H

#include "defs.h"
#include "SDL.h"

/* One unit of time is how many ticks: */
#define UNIT_TIME 1000.0f

#define Time float

typedef struct timer_struct
{
  Uint32 ticks;          /* The number of ticks when time_update was last
			    called */
  Time max_elapsed;      /* The maximum amount of time that can elapse */

}
Timer;

extern void time_init(Timer *t, Time max_elapsed);
extern Time time_getMax(Timer *t);
extern void time_setMax(Timer *t, Time max_elapsed);
extern Time time_dt(Timer *t);
extern void time_update(Timer *t);
extern int time_expired(Timer *t);

#endif // __DEFINED_TIMER_H
