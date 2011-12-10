#ifndef __DEFINED_SIGNAL_H
#define __DEFINED_SIGNAL_H

#include "defs.h"
#include <stdio.h>
#include <stdlib.h>


/* Types of signals:
   Hit: Signals that a collision occurred with something.
   Impulse: In addition to having a Hit signal, also carries the vector info.
*/


typedef struct
{
  int type;        /* Object or tile */
  union
  {
    int obj_type;
    int tile_type;
  } u;
} Hit;

typedef struct
{
  Vector vec;
  Hit hit;
} Impulse;

enum {SPAWN_OBJECT, KILL_OBJECT};
typedef struct
{
  int todo;        /* Kill or spawn */
  void *obj;
} SpawnKill;

enum signal_types {IMPULSE_SIG, HIT_SIG, SPAWN_KILL_SIG};

typedef struct
{
  int type;
  union
  {
    Impulse imp;
    Hit hit;
    SpawnKill sk;
  } sig;
} Signal;

/* Definitions for a signal queue */
/* Syntax from "A Book on C" */
struct signal_list
{
  Signal s;
  struct signal_list *next;
};

typedef struct signal_list Sig_Element;
typedef Sig_Element *Sig_Link;

struct signal_queue
{
  Sig_Link front, rear;
};

typedef struct signal_queue SigQ;

extern void sig_initQ(SigQ *q);
extern int sig_poll(SigQ *q, Signal *s);
extern void sig_push(SigQ *q, Signal *s);
extern void sig_flush(SigQ *q);


#endif /* __DEFINED_SIGNAL_H */
