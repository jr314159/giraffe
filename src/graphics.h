#ifndef __DEFINED_GRAPHICS_H
#define __DEFINED_GRAPHICS_H

#include <stdio.h>
#include "defs.h"
#include "SDL.h"

/* This is the color in any bitmap images which will be considered
 * transparent.  So, sadly, no hot pink for my games. :(
 */
#define TRANSPARENT_RGB 255, 0, 255

#define gfx_freeImage(x) SDL_FreeSurface(x)

extern SDL_Surface *gfx_loadImage(char *filename);
extern int gfx_blitImage(SDL_Surface *src_surf, int x, int y);
extern void gfx_clearScreen(Color *c);
extern void gfx_initScreen(int xres, int yres, int fullscreen);
extern int gfx_getScreenWidth(void);
extern int gfx_getScreenHeight(void);
extern void gfx_renderScreen(void);
extern void gfx_setClipRect(int x, int y, int w, int h);
extern void gfx_drawRect(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
extern void gfx_drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
extern void gfx_drawCircle(int x, int y, int radius, int r, int g, int b, int a);
extern void gfx_toggleFullscreen(void);



#endif // __DEFINED_GRAPHICS_H



