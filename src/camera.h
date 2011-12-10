#ifndef __DEFINED_CAMERA_H
#define __DEFINED_CAMERA_H

#include "defs.h"

/* The camera's position refers to a real coordinate in the layer
   which it's focusing on. */

extern void cam_setFocusLayer(int l);
extern void cam_setCameraPos(int x, int y);
extern Point cam_getCameraPos(void);
extern void cam_setCameraSize(int w, int h);
extern void cam_render(void);
extern void cam_moveCamera(int dx, int dy);
extern Rect cam_getViewRange(int l);

/* If we want to render object boundaries: */
//#define RENDER_OBJ_BOUND
#define OBJ_BOUND_RGBA 200, 200, 255, 255

/* If we want to render tile boundaries: */
//#define RENDER_TILE_BOUND
#define TILE_BOUND_RGBA 255, 200, 200, 255

/* If we want to render the normals of line boundaries: */
#define RENDER_LINE_NORMAL

/* If we want to render sectors: */
//#define RENDER_SECTORS
#define SECTOR_RGBA 255, 255, 255, 255

#endif // __DEFINED_CAMERA_H
