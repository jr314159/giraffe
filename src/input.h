#ifndef __DEFINED_INPUT_H
#define __DEFINED_INPUT_H

/* input.h

   This is just a stupid little module to keep track of the player input
   states, it doesn't actually do anything.

*/

enum {LEFT = -1, RIGHT = 1};
enum {UP = -1 , DOWN = 1};

enum keys {JUMP_KEY, RUN_KEY, SHOOT_KEY, 
	   LEFT_KEY, RIGHT_KEY,
	   UP_KEY, DOWN_KEY};

extern void inp_keyDown(int key);
extern void inp_keyUp(int key);
extern int inp_isDown(int key);
extern int inp_getVert(void);
extern int inp_getHoriz(void);
extern void inp_setVert(int);
extern void inp_setHoriz(int);

#endif /* __DEFINED_INPUT_H */
