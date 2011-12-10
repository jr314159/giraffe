#include "../objtypes.h"
#include "../tiletypes.h"

struct baddie_atts
{
  int hitpoints;
  int on_ground;
};


static Bound *bounds(void)
{
  int bound_type = LINE;

  Bound *b;
  Bound *head;

  MALLOC(head, sizeof(Bound));

  b = head;

  if (bound_type == RECT)
    {
      b->type = RECT;
      b->b.rect.p1.x = 0;
      b->b.rect.p1.y = 0;
      b->b.rect.p2.x = 40;
      b->b.rect.p2.y = 40;
    }
  else if (bound_type == LINE)
    {
      b->type = LINE;
      b->b.line.p1.x = 40;
      b->b.line.p1.y = 40;
      b->b.line.p2.x = 0;
      b->b.line.p2.y = 40;
      
      
      // Left
      MALLOC(b->next, sizeof(Bound));
      b = b->next;
      b->type = LINE;
      b->b.line.p2.x = 0;
      b->b.line.p2.y = 0;
      b->b.line.p1.x = 0;
      b->b.line.p1.y = 40;
      
      
      // Right
      MALLOC(b->next, sizeof(Bound));
      b = b->next;
      b->type = LINE;
      b->b.line.p1.x = 40;
      b->b.line.p1.y = 0;
      b->b.line.p2.x = 40;
      b->b.line.p2.y = 40;
      
      //Top
      MALLOC(b->next, sizeof(Bound));
      b = b->next;
      b->type = LINE;
      b->b.line.p1.x = 0;
      b->b.line.p1.y = 0;
      b->b.line.p2.x = 40;
      b->b.line.p2.y = 0;
    }
  b->next = NULL;
  return head;
}

static void *init_atts(void)
{
  struct baddie_atts *atts;

  /* Initialize the object-specific attributes: */

  MALLOC(atts, sizeof(struct baddie_atts));

  atts->hitpoints = 30;
  atts->on_ground = 0;
  return (void *) atts;
}

static void free_atts(Object *me)
{
  struct baddie_atts *atts;
  atts = (struct baddie_atts *) me->atts;
  free(atts);
}

/* Take damage */
static void damage(Object *me, int n)
{
  struct baddie_atts *atts = (struct baddie_atts *) me->atts;
  atts->hitpoints -= n;
  if (atts->hitpoints <= 0) 
    {
      obj_makeSound(me, "explode", 0);
      obj_killObj(me);
    }
  else obj_makeSound(me, "ow", 0);
}

static void go(Object *me, Time dt)
{

  /* Process signals */
  Signal sig;
  while (sig_poll(&me->signals, &sig))
    {
      switch (sig.type)
	{
	case HIT_SIG:
	  switch (sig.sig.hit.type)
	    {
	    case OBJ_TYPE:
	      switch (sig.sig.hit.u.obj_type)
		{
		case PLAYER_TYPE:
		  /* I've hit the player */
		  obj_makeSound(me, "thock", 0);
		  break;
		case BADDIE_TYPE:
		  /* I've hit a baddie */
		  //obj_makeSound(me, "boing", 0);
		  break;
		case BULLET_TYPE:
		  /* I've been shot! */
		  damage(me, 15);
		  break;
		}
	      break;
	    case TILE_TYPE:
	      switch (sig.sig.hit.u.tile_type)
		{
		case NONE_T:
		  //obj_makeSound(me, "boing", 0);
		  break;
		}
	      break;
	    }
	  break;
	}
    }

  /* Fall */
  me->vel.y += 1000 * dt;

}

struct obj_att_define baddie_def =
{
  /*mass = */ 1,
  /*w = */ 40,
  /*h = */ 40,
  /*elasticity = */ 1,
  /*friction = */ 0,
  /*solid = */ 1,
  /*sprite = */ "baddie",
  /*animation = */ "idle",
  /*bounds = */ bounds,
  /*init_atts = */ init_atts,
  /*go = */ go,
  /*free_atts = */ free_atts
};
