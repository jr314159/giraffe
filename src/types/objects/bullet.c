#include "../objtypes.h"
#include "../tiletypes.h"

static Bound *bounds(void)
{

  Bound *head;
  MALLOC(head, sizeof(Bound));

  head->type = RECT;
  head->b.rect.p1.x = 0;
  head->b.rect.p1.y = 0;
  head->b.rect.p2.x = 9;
  head->b.rect.p2.y = 6;

  head->next = NULL;
  return head;
}

static void *init_atts(void)
{
  return NULL;
}

static void free_atts(Object *me)
{
}

static void go(Object *me, Time dt)
{

  /* The problem is that the bullet gets this hit signal more than once, so
     the object signal handler tries to free the same object twice.  Maybe
     I need to have objects have some sort of killme flag? */

  /* The bullet dies when it hits an object or a solid plain tile.*/
  Signal sig;
  while (sig_poll(&me->signals, &sig))
    {
      switch (sig.type)
	{
	case HIT_SIG:
	  switch (sig.sig.hit.type)
	    {
	    case OBJ_TYPE:
	      obj_killObj(me);
	      break;
	    case TILE_TYPE:
	      switch (sig.sig.hit.u.tile_type)
		{
		case NONE_T:
		  obj_killObj(me);
		  break;
		}
	      break;
	    }
	  
	  break;
	}
    }
}

struct obj_att_define bullet_def =
{
  /*mass = */ 1,
  /*w = */ 9,
  /*h = */ 6,
  /*elasticity = */ 0,
  /*friction = */ 0,
  /*solid = */ 1,
  /*sprite = */ "bullet",
  /*animation = */ "fuh",
  /*bounds = */ bounds,
  /*init_atts = */ init_atts,
  /*go = */ go,
  /*free_atts = */ free_atts
};

