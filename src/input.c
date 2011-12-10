#include "input.h"

static int key_states[] = {0, 0, 0, 0, 0, 0, 0};

static int horiz_input = 0, vert_input = 0;

/* inp_keyDown
   Sets the state for a key
*/
void
inp_keyDown(int key)
{
  extern int key_states[];
  key_states[key] = 1;
}

/* inp_keyUp
   Sets the state for a key
*/
void
inp_keyUp(int key)
{
  extern int key_states[];
  key_states[key] = 0;
}

/* inp_isDown
   Returns true if a key is down.
*/
int
inp_isDown(int key)
{
  extern int key_states[];
  return key_states[key];
}

/* inp_setHoriz

   sets the horizontal input

*/
void
inp_setHoriz(int hor)
{

  extern int horiz_input;
  horiz_input = hor;

}

/* inp_setVert

   sets the vertical input

*/

void
inp_setVert(int vert)
{

  extern int vert_input;
  vert_input = vert;

}

/* inp_getHoriz

   gets the horizontal input

*/

int
inp_getHoriz(void)
{

  extern int horiz_input;
  return horiz_input;

}

/* inp_getVert

   gets the vertical input

*/

int
inp_getVert(void)
{

  extern int vert_input;
  return vert_input;

}






