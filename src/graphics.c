#include "graphics.h"

#include "SDL_image.h"
#include "SDL_gfxPrimitives.h"

// Maintain the screen information in this module
static SDL_Surface *screen;


/* gfx_loadImage
 * loads an image from a file
 */
SDL_Surface *gfx_loadImage(char *filename) {

  extern SDL_Surface *screen;

  SDL_Surface *src_img = NULL, *temp_img;

  /* Load the image from the file */
  if (!(temp_img = IMG_Load(filename))) {
    printf("Error loading image: %s\n", IMG_GetError());
    exit(0);
  }

  /* Format the image to that of the framebuffer, scrap the temp image.
   * (Important step!  It doesn't do the transparency right if I don't
   * do SDL_DisplayFormat)
   */
  src_img = SDL_DisplayFormat(temp_img);
  SDL_FreeSurface(temp_img);


  /* Set the transparent color of the image to TRANSPARENT_RGB */
  SDL_SetColorKey(src_img, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(screen->format, TRANSPARENT_RGB));

  return (src_img);

}


/* gfx_blitImage
 * blits an SDL_Surface onto another surface at a specified location
 * returns 0 on failure
 */

int gfx_blitImage(SDL_Surface *src_surf, int x, int y) {

  extern SDL_Surface *screen;
  SDL_Rect dest_rect;

  dest_rect.x = x;
  dest_rect.y = y;
  dest_rect.w = src_surf->w;
  dest_rect.h = src_surf->h;


  /* Do the blit! */
  if ((SDL_BlitSurface(src_surf, NULL, screen, &dest_rect)) < 0) {
    printf("Error blitting image: %s\n", IMG_GetError());
    return (0);
  }

  return (1);
}

/* gfx_setClipRect

   sets a clipping rect on the screen

*/

void
gfx_setClipRect(int x, int y, int w, int h)
{
  extern SDL_Surface *screen;

  SDL_Rect clip_rect = {x, y, w, h};

  SDL_SetClipRect(screen, &clip_rect);
}


/* gfx_clearScreen
 * "clears" the screen by filling it with the background color
 */
void gfx_clearScreen(Color *c) {

  extern SDL_Surface *screen;

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, c->r, c->g, c->b));

}

/* gfx_initScreen
 * creates the screen surface with specified resolution
 */
void
gfx_initScreen(int xres, int yres, int fullscreen) 
{

  Uint32 flags = SDL_HWSURFACE|SDL_DOUBLEBUF;
  extern SDL_Surface *screen;

  if (fullscreen) flags |= SDL_FULLSCREEN;

  /* Attempt to use hardware mode, use software mode if that fails. */
  if ((screen = SDL_SetVideoMode(xres, yres, 16, flags)) == NULL) {
    fprintf(stderr, "Unable to use hardware mode: %s\n Attempting to use software...", SDL_GetError());

    flags &= SDL_HWSURFACE; flags |= SDL_SWSURFACE;
    if ((screen = SDL_SetVideoMode(xres, yres, 16, flags)) == NULL)
      fprintf(stderr, "Unable to set %dx%d video: %s\n", xres, yres, SDL_GetError());
  }

}

/* gfx_getScreenWidth

   returns the width of the screen

*/
int
gfx_getScreenWidth(void)
{
  extern SDL_Surface *screen;
  return screen->w;
}

/* gfx_getScreenHeight

   returns the height of the screen

*/
int
gfx_getScreenHeight(void)
{
  extern SDL_Surface *screen;
  return screen->h;
}

/* gfx_renderScreen

   renders the screen (performs a page flip)

*/

void
gfx_renderScreen(void)
{
  extern SDL_Surface *screen;
  SDL_Flip(screen);
}

/* gfx_drawRect

   Draws a rectangle on the screen

*/

void
gfx_drawRect(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{

  extern SDL_Surface *screen;
  rectangleRGBA(screen, x1, y1, x2, y2, r, g, b, a);

}

/* gfx_drawLine

   Draws a line on the screen

*/

void
gfx_drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{

  extern SDL_Surface *screen;
  lineRGBA(screen, x1, y1, x2, y2, r, g, b, a);

}

/* gfx_drawCircle

   Draws a circle on the screen

*/

void
gfx_drawCircle(int x, int y, int radius, int r, int g, int b, int a)
{

  extern SDL_Surface *screen;

  circleRGBA(screen, x, y, radius, r, g, b, a);

}

/* gfx_toggleFullscreen
   Toggles fullscreen mode.
*/
void
gfx_toggleFullscreen(void)
{
  extern SDL_Surface *screen;
  SDL_WM_ToggleFullScreen(screen);
}
