#include "camera.h"

#include "map.h"
#include "graphics.h"
#include "object.h"
#include "collision.h"

/* The camera's position refers to a coordinate on this layer */
static int focus_layer;

/* The point at the center of the camera's view. */
static Point position;

// Camera dimensions
static int width, height;

/* cam_setFocusLayer
   sets the layer the camera is pointed at
*/
void
cam_setFocusLayer(int l)
{
  extern int focus_layer;
  focus_layer = l;
}

/* cam_setCameraPos
   set the position of the camera
*/
void
cam_setCameraPos(int x, int y)
{
  extern Point position;
  position.x = x;
  position.y = y;
}

/* cam_getCameraPos
   get the position of the camera
*/
Point
cam_getCameraPos(void)
{

  extern Point position;
  return position;
}

/* cam_getViewRange
   Returns a rectangle representing the range in real coordinates on a 
   layer that the camera currently sees.
*/
Rect cam_getViewRange(int l)
{
  extern int width, height, focus_layer;
  extern Point position;
  Point pos_on_layer;
  Rect range;
  
  /* Find the camera's position on this layer based on the size of this
     layer relative to the size of the active layer:
     This creates parallax scrolling. */

  pos_on_layer.x = position.x * map_getLayerWidth(l) / map_getLayerWidth(focus_layer);
  pos_on_layer.y = position.y * map_getLayerHeight(l) / map_getLayerHeight(focus_layer);
  range.p1.x = pos_on_layer.x - width / 2;
  range.p1.y = pos_on_layer.y - height / 2;
  range.p2.x = range.p1.x + width;
  range.p2.y = range.p1.y + height;
  return range;
}


/* cam_setCameraSize
   set the dimensions of the camera and set the clipping rectangle on the
   screen
*/
void
cam_setCameraSize(int w, int h)
{
  extern int width, height;
  width = w;
  height = h;
  gfx_setClipRect((gfx_getScreenWidth() - width) / 2, (gfx_getScreenHeight() - height) / 2, width, height);
}

/* cam_moveCamera

   move the camera a given distance

*/
void
cam_moveCamera(int dx, int dy)
{
  extern Point position;
  extern int focus_layer;

  // move the camera
  position.x += dx;
  position.y += dy;

  
  // if the camera is off the map, move it back on
  position.x = (position.x < 0) ? 0 : position.x;
  position.y = (position.y < 0) ? 0 : position.y;
  position.x = (position.x >= map_mapToRealX(map_getLayerWidth(focus_layer))) ? (map_mapToRealX(map_getLayerWidth(focus_layer)) - 1) : position.x;
  position.y = (position.y >= map_mapToRealY(map_getLayerHeight(focus_layer))) ? (map_mapToRealY(map_getLayerHeight(focus_layer)) - 1) : position.y;
}

/* cam_render
   renders what the camera is looking at to the center of the screen
*/
void
cam_render(void)
{

  extern int width, height;
  extern Point position;
  extern int focus_layer;

  int l;  /* The current layer */
  int screen_w, screen_h; // width and height of the screen

  Point blit_start; // the top left point where the blitting starts from

  int cam_sector_x, cam_sector_y;  // the sector the camera is in
  int sector_x, sector_y; // the sector we are scanning for object sprites
  //Object *this_object; // the current object we are looking at


  // Get the screen dimensions:
  screen_w = gfx_getScreenWidth();
  screen_h = gfx_getScreenHeight();

  // We want the drawing centered on the screen:
  blit_start.x = (screen_w - width) / 2;
  blit_start.y = (screen_h - height) / 2;

  // clear the screen:
  gfx_clearScreen(map_getBackgroundColor());

  /* Loop through all of the layers */
  for (l = 0; l < map_getNLayers(); l++)
    {
      int map_x, map_y;
      int camera_sector_x, camera_sector_y;
      int sector_x, sector_y;
      Rect range;

      /* Find the camera's position on this layer based on the size of this
	 layer relative to the size of the active layer:
	 This creates parallax scrolling. */
      Point pos_on_layer, camera_top_left;
      pos_on_layer.x = position.x * map_getLayerWidth(l) / map_getLayerWidth(focus_layer);
      pos_on_layer.y = position.y * map_getLayerHeight(l) / map_getLayerHeight(focus_layer);
      camera_top_left.x = pos_on_layer.x - width / 2;
      camera_top_left.y = pos_on_layer.y - height / 2;

      /* Render tiles */

      /* Loop through all of the map coordinates 
	 that the camera sees at least a
	 part of.  Also make sure that the 
	 coordinates are not off the map. */
      for (map_x = (map_realToMapX(camera_top_left.x) >= 0) ? map_realToMapX(camera_top_left.x) : 0; 
	   (map_x <= map_realToMapX(camera_top_left.x + width))
	     && (map_x < map_getLayerWidth(l)); 
	   map_x++)
	{
	  
	  for (map_y = (map_realToMapY(camera_top_left.y) >= 0) ? map_realToMapY(camera_top_left.y) : 0; 
	       (map_y <= map_realToMapY(camera_top_left.y + height))
		 && (map_y < map_getLayerHeight(l)); 
	       map_y++)
	    {
	      
	      // If there is a tile at this map coordinate, render it:
	      if (map_getTileGfx(l, map_x, map_y) != NULL)
		{

		  /* Blit the image of the tile at 
		     this map coordinate, clipped 
		     by the rectangle of the camera size, 
		     to the right position on the screen. */
		  Point tile_gfx_offset = map_getTileGfxOffset(l, map_x, map_y);
		  int x_offset = map_xInTile(camera_top_left.x);
		  int y_offset = map_yInTile(camera_top_left.y);

		  int blit_x = blit_start.x - x_offset + tile_gfx_offset.x + map_mapToRealX(map_x - map_realToMapX(camera_top_left.x));
		  int blit_y = blit_start.y - y_offset + tile_gfx_offset.y + map_mapToRealY(map_y - map_realToMapY(camera_top_left.y));
		  
		  gfx_blitImage(map_getTileGfx(l, map_x, map_y), 
				blit_x, 
				blit_y);

		  /* Render tile boundaries for testing purposes: */
#ifdef RENDER_TILE_BOUND
		  {
		    Bound *this_bound;
		    this_bound = map_getTileBounds(l, map_x, map_y);
		    
		    while (this_bound != NULL)
		      {
			switch (this_bound->type)
			  {
			  case RECT:
			    gfx_drawRect(blit_x + this_bound->b.rect.p1.x, blit_y + this_bound->b.rect.p1.y, blit_x + this_bound->b.rect.p2.x, blit_y + this_bound->b.rect.p2.y, TILE_BOUND_RGBA);
			    break;
			  case LINE:
			    gfx_drawLine(blit_x + this_bound->b.line.p1.x, blit_y + this_bound->b.line.p1.y, blit_x + this_bound->b.line.p2.x, blit_y + this_bound->b.line.p2.y, TILE_BOUND_RGBA);
#ifdef RENDER_LINE_NORMAL
			    gfx_drawLine(blit_x + (this_bound->b.line.p1.x + this_bound->b.line.p2.x) / 2, blit_y + (this_bound->b.line.p1.y + this_bound->b.line.p2.y) / 2, (blit_x + (this_bound->b.line.p1.x + this_bound->b.line.p2.x) / 2) + (10 * normal(this_bound->b.line).x), (blit_y + (this_bound->b.line.p1.y + this_bound->b.line.p2.y) / 2) + (10 * normal(this_bound->b.line).y), TILE_BOUND_RGBA);
#endif /* RENDER_LINE_NORMAL */
			    break;
			  case CIRCLE:
			    gfx_drawCircle(blit_x + this_bound->b.circle.p.x, blit_y + this_bound->b.circle.p.y, this_bound->b.circle.r, TILE_BOUND_RGBA);
			    break;
			  default :
			    break;
			  }
			this_bound = this_bound->next;
		      }
		  }
#endif /* RENDER_TILE_BOUND */
		}  /* endif there are graphics at this tile */
	    }
	}  /* end x,y for loops */

      /* Render objects */

      /* Determine which sector the camera's position on this layer is in,
	 and check for all objects in that sector and surrounding sectors. */
      
      cam_sector_x = obj_realToSectorX(pos_on_layer.x);
      cam_sector_y = obj_realToSectorY(pos_on_layer.y);

      /* Loop through the range of sectors, making sure the range is within the
	 layer dimensions */
      for (sector_y = (cam_sector_y - SECTOR_Y_RANGE >= 0) ? cam_sector_y - SECTOR_Y_RANGE : 0;
	   sector_y <= cam_sector_y + SECTOR_Y_RANGE && sector_y < obj_getLayerHeight(l);
	   sector_y++)
	{
      
	  for (sector_x = (cam_sector_x - SECTOR_X_RANGE >= 0) ? cam_sector_x - SECTOR_X_RANGE : 0;
	       sector_x <= cam_sector_x + SECTOR_X_RANGE && sector_x < obj_getLayerWidth(l);
	       sector_x++)
	    {
	      
	      /* Loop through all of the objects in this sector: */	  
	      Object *this_object = obj_getObjList(l, sector_x, sector_y);
	      
	      while (this_object != NULL)
		{
		  
		  Point gfx_pos;
		  /* Get the top left point in real coordinates of the object's
		     graphic: */
		  gfx_pos = obj_getGfxPos(this_object);
		  SDL_Surface *obj_gfx = obj_getObjGfx(this_object);
		  
		  /* Render it if any part of the graphic is on screen: */
		  if (
		      !((gfx_pos.y > camera_top_left.y + height) ||
			(gfx_pos.y + obj_gfx->h < camera_top_left.y) ||
			(gfx_pos.x > camera_top_left.x + width) ||
			(gfx_pos.x + obj_gfx->w < camera_top_left.x))
		      )
		    {
		      
		      //Blit it !!!
		      
		      int blit_x, blit_y; // the blit position on screen
		      blit_x = blit_start.x + gfx_pos.x - camera_top_left.x;
		      blit_y = blit_start.y + gfx_pos.y - camera_top_left.y;
		      
		      gfx_blitImage(obj_gfx, blit_x, blit_y);
		      
		      /* Render the object's boundaries for testing purposes: */
#ifdef RENDER_OBJ_BOUND
		      { 
			Point top_left; 
			Bound *this_bound;
			int draw_x, draw_y;
			
			top_left = obj_getObjTopLeft(this_object);
			this_bound = obj_getObjBounds(this_object);
			
			draw_x = blit_start.x + top_left.x - camera_top_left.x;
			draw_y = blit_start.y + top_left.y - camera_top_left.y;
			
			while (this_bound != NULL)
			  {
			    switch (this_bound->type)
			      {
			      case RECT:
				gfx_drawRect(draw_x + this_bound->b.rect.p1.x, draw_y + this_bound->b.rect.p1.y, draw_x + this_bound->b.rect.p2.x, draw_y + this_bound->b.rect.p2.y, OBJ_BOUND_RGBA);
				break;
			      case LINE:
				gfx_drawLine(draw_x + this_bound->b.line.p1.x, draw_y + this_bound->b.line.p1.y, draw_x + this_bound->b.line.p2.x, draw_y + this_bound->b.line.p2.y, OBJ_BOUND_RGBA);
#ifdef RENDER_LINE_NORMAL
				gfx_drawLine(draw_x + (this_bound->b.line.p1.x + this_bound->b.line.p2.x) / 2, draw_y + (this_bound->b.line.p1.y + this_bound->b.line.p2.y) / 2, (draw_x + (this_bound->b.line.p1.x + this_bound->b.line.p2.x) / 2) + (10 * normal(this_bound->b.line).x), (draw_y + (this_bound->b.line.p1.y + this_bound->b.line.p2.y) / 2) + (10 * normal(this_bound->b.line).y), TILE_BOUND_RGBA);
#endif /* RENDER_LINE_NORMAL */
				
				
				break;
			      case CIRCLE:
				gfx_drawCircle(draw_x + this_bound->b.circle.p.x, draw_y + this_bound->b.circle.p.y, this_bound->b.circle.r, OBJ_BOUND_RGBA);
				break;
			      default :
				break;
			      }
			    this_bound = this_bound->next;
			  }
			
		      }
#endif /* RENDER_OBJ_BOUND */
		      
		    } /* endif part of the graphic is onscreen */
		  
		  /* get the next object */
		  this_object = obj_getNextObj(this_object);
		} /* end looping through objects in sector */
	      
	    }
	} /* end sector for loops */

      /* Render sector boundaries for testing purposes: */	
#ifdef RENDER_SECTORS
      {
	int i;
	for (i = blit_start.x - (camera_top_left.x % obj_getSectorW()); i < blit_start.x + width; i += obj_getSectorW())
	  gfx_drawLine(i, blit_start.y, i, blit_start.y + height, SECTOR_RGBA);
	
	for (i = blit_start.y  - (camera_top_left.y % obj_getSectorH()); i < blit_start.y + height; i += obj_getSectorH())
	  gfx_drawLine(blit_start.x, i, blit_start.x + width, i, SECTOR_RGBA);
      }
#endif /* RENDER SECTORS */

    }  /* end layer for loop */


  //render the screen (page flip)
  
  gfx_renderScreen();
  
}

