/* ============================================================================
 * [XVT -> Qt PORT] This file is unchanged calculation/application logic.
 * Its XVT toolkit calls now run against the Qt5-backed compatibility layer
 * in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the original
 * proprietary XVT SDK. No calculation or algorithmic code in this file was
 * modified to accomplish this port -- only the underlying GUI toolkit
 * implementation, in qt_compat/, changed.
 * ============================================================================
 */
/* ======================================================================

                      ** PLEASE READ AND NOTE **

     This file is used for both the sequential and parallel versions
     of Noddy, MAKE SURE any changes that are made are compatible
     with both version of Noddy.

     You may use the _MPL defined symbol to conditionally compile
     code that is only associated with a particular version of Noddy.

     _MPL is defined only for the MASPAR Parallel verion of Noddy.

                      THANK-YOU FOR YOUR ATTENTION

====================================================================== */
#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include <math.h>
#include "scales.h"

#define DEBUG(X)  
#define DEBUG1(X) 
#define DEBUGL(X) 

                 /* ********************************** */
                 /* External Globals used in this file */


                 /* ************************* */
                 /* Globals used in this file */
LAYER_PROPERTIES *getClosestImportBlock ();


#ifndef _MPL
COLOR which(xcord,ycord,index, xLoc, yLoc, zLoc,flavor)
double xcord, ycord, xLoc, yLoc, zLoc;
unsigned int flavor;
int index;
{
/*                           
c                                                                               
c   which calculates which horizon a particular height                          
c   'zLoc' is found in within stratigraphy number 'is'                          
c   with the associated symbol plotted at coordinates                           
c   (xcord,ycord).                                                              
c                                                                               
*/
   WINDOW currentDrawingWindow = getCurrentDrawingWindow ();
   DRAW_CTOOLS tools;
   COLOR color = 0L;
   OBJECT *event;
   STRATIGRAPHY_OPTIONS *stratOptions;
   LAYER_PROPERTIES *layerProp = NULL;
   int l2, layerIndex;
      
   DEBUG(printf("\nWHICH: (%.1f, %.1f), index = %d, zLoc = %f, flavor = %d",xcord,ycord,index,zLoc,flavor);)

   if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
   {
      DEBUG1(printf("\n*************** COULD NOT GET OBJECT %d",index);)
      return (color);
   }

   xvt_app_get_default_ctools(&tools);
   tools.pen.pat = PAT_SOLID;
   tools.brush.pat = PAT_SOLID;

                        /* if it was imported then get the imported block */
   if (event->shape == IMPORT)
   {
      if (layerProp = getClosestImportBlock ((IMPORT_OPTIONS *) event->options,
                                                xLoc, yLoc, zLoc, &layerIndex))
      {
         color = XVT_MAKE_COLOR(layerProp->color.red,
                                layerProp->color.green,
                                layerProp->color.blue);
      }      
   }

                    /* if it was NOT imported or the Imported block was
                    ** "see through" then get the strat layer */
   if (!layerProp)
   {
              /* if the current event does not have a stratigraphy
              ** associated with it then use the base stratigraphy */
      if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
                           getStratigraphyOptionsStructure (event)))
      {
         if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
            getStratigraphyOptionsStructure ((OBJECT *) nthObject (NULL_WIN, 0))))
            return (color);  /* not even a base strat is defined yet */
      }
                                           /* only 1 layer so use that color */
      if (stratOptions->numLayers == 1)
         color = XVT_MAKE_COLOR (stratOptions->properties[0].color.red,
                          stratOptions->properties[0].color.green,
                          stratOptions->properties[0].color.blue);
                                     /* below the bottom so use bottom color */
      else if (zLoc <= stratOptions->properties[1].height)
         color = XVT_MAKE_COLOR (stratOptions->properties[0].color.red,
                          stratOptions->properties[0].color.green,
                          stratOptions->properties[0].color.blue);
                                           /* above the top so use top color */
      else if (zLoc > stratOptions->properties[stratOptions->numLayers-1].height)
         color = XVT_MAKE_COLOR (
               stratOptions->properties[stratOptions->numLayers-1].color.red,
               stratOptions->properties[stratOptions->numLayers-1].color.green,
               stratOptions->properties[stratOptions->numLayers-1].color.blue);
      else
      {
         for (l2 = 1; l2 < (stratOptions->numLayers-1); l2++)
         {
            if ((zLoc > stratOptions->properties[l2].height)
                     && (zLoc <= stratOptions->properties[l2+1].height))
            {
               color = XVT_MAKE_COLOR (stratOptions->properties[l2].color.red,
                                stratOptions->properties[l2].color.green,
                                stratOptions->properties[l2].color.blue);
            }
         }
      }
   }
   
                    /* invalid cordinates so dont draw point */
   if ((xcord < 0.0) || (ycord < 0.0))
      return (color);

                    /* set the color for drawing */
   tools.brush.color = color;
   tools.pen.color = color;
   xvt_dwin_set_draw_ctools(currentDrawingWindow, &tools);

   plotpoint (xcord, ycord);

   return (color);
}                       

int lwhich (xcord, ycord, index, xLoc, yLoc, zLoc, flavor, maptype)
double xcord, ycord, xLoc, yLoc, zLoc;
unsigned int flavor;
int index, maptype;
{
/*
c
c   which calculates which horizon a particular height
c   'zLoc' is found in within stratigraphy number 'is'
c   with the associated symbol plotted at coordinates
c   (xcord,ycord).
c
*/

   DEBUG(printf("\nWHICHL: (%.1f, %.1f), index = %d, zLoc = %f, flavor = %d",xcord,ycord,index,zLoc,flavor);)

   if (maptype == WELLLOG) /* Wellog*/
   {
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      SetRectPointSize (21, 16);
#else
      SetRectPointSize (19, 15);
#endif
   }

   which (xcord, ycord, index, xLoc, yLoc, zLoc, flavor);
   return (TRUE);
}


/* ======================================================================
FUNCTION        getClosestImportBlock
DESCRIPTION
        pick out the layer properties struct that reflects the block closest
        to a given xyz location.
INPUT
   importOptions - the options structure describing the imported block
   xLoc, yLoc, zLoc - the point we are checking for

RETURNED
   pointer to a properties structure if the point was in the import area
   or NULL (if it is not part of the imported area)

====================================================================== */
LAYER_PROPERTIES *getClosestImportBlock (importOptions, xLoc, yLoc, zLoc, layerIndex)
IMPORT_OPTIONS *importOptions;
double xLoc, yLoc, zLoc;
int *layerIndex;
{
   LAYER_PROPERTIES *propertyStructure = NULL;
   int blockX, blockY, blockZ, index;
   int outsideImportBlock = FALSE;
   double blockLocX, blockLocY, blockLocZ;
   int numXBlocks = 0, numYBlocks = 0, numZBlocks;
   double blockBlockSize = importOptions->blockSize;

   if (!importOptions->layerDimensions)
      return (propertyStructure);

   for (index = 0; index < importOptions->numLayers; index++)   
   {
      if (importOptions->layerDimensions[index][0] > numXBlocks)
         numXBlocks = importOptions->layerDimensions[0][0];
      if (importOptions->layerDimensions[index][1] > numYBlocks)
         numYBlocks = importOptions->layerDimensions[0][1];
   }
   numZBlocks = importOptions->numLayers;
                                                                           
   *layerIndex = 0;                                                                          
                      /* work out position of top left front corner of import block */
                      /* as that is where the origin is of a noddy block */
   if (importOptions->positionBy == POS_BY_CORNER)
   {
      blockLocX = importOptions->positionX;
      blockLocY = importOptions->positionY;
      blockLocZ = importOptions->positionZ;
   }
   else
   {
      blockLocX = importOptions->positionX - ((numXBlocks/2) * blockBlockSize);
      blockLocY = importOptions->positionY - ((numYBlocks/2) * blockBlockSize);
      blockLocZ = importOptions->positionZ - ((numZBlocks/2) * blockBlockSize);
   }
   
   blockZ = (int) floor((double) ((blockLocZ-zLoc) / blockBlockSize) + 0.5);
   blockZ = numZBlocks - blockZ - 1;

   if (blockZ < 0) outsideImportBlock = TRUE;
   else if (blockZ > numZBlocks-1) outsideImportBlock = TRUE;

   if (!outsideImportBlock)
   {
      numXBlocks = importOptions->layerDimensions[blockZ][0];
      numYBlocks = importOptions->layerDimensions[blockZ][1];
      blockX = (int) floor((double) ((xLoc-blockLocX) / blockBlockSize) + 0.5);
      blockY = (int) floor((double) ((yLoc-blockLocY) / blockBlockSize) + 0.5);
   }
                      /* Make sure we are inside the imported block */
   if (blockX < 0) outsideImportBlock = TRUE;
   else if (blockX > numXBlocks-1) outsideImportBlock = TRUE;

   if (blockY < 0) outsideImportBlock = TRUE;
   else if (blockY > numYBlocks-1) outsideImportBlock = TRUE;


         /* assign the propertyStructure for the index in the blockData */
   if (!outsideImportBlock)
   {
      index = importOptions->blockData[blockZ][blockX][numYBlocks - blockY - 1];
       /* Zero + any other layer are "see through" to what ever is behind */
      if (index && (index != importOptions->transparentLayer))
      {
         propertyStructure = &(importOptions->properties[index-1]);
         *layerIndex = index-1;
      }
   }
   
   return (propertyStructure);
}

#endif

void which_prop(index, xLoc, yLoc, zLoc, flavor, lutindex, piStrati, piLayer)
#ifdef _MPL
plural int index;
plural double xLoc, yLoc, zLoc;
plural unsigned int flavor;
plural int *lutindex;
plural int *piStrati, *piLayer;
#else
int index;
double xLoc, yLoc, zLoc;
unsigned int flavor;
int *lutindex;
int *piStrati,*piLayer;
#endif
{
/*                           
**   which calculates which horizon a particular height
**   'zLoc' is found in within stratigraphy number 'is'
**   with the associated value provided for geophysical keying
**     piStrati - IS,
**     piLayer - a layer withing IS.
*/

#ifdef _MPL
   plural int height[7];
   plural int l2, numLayers;
   OBJECT * plural event;
   STRATIGRAPHY_OPTIONS * plural stratOptions;
#else
   int height[7];
   int l2, numLayers;
   OBJECT *event;
   STRATIGRAPHY_OPTIONS *stratOptions;
#endif
   LAYER_PROPERTIES *layerProp = NULL;
   int layerIndex;


#ifdef _MPL
   if (!(event = (OBJECT * plural) p_nthObject (NULL_WIN, index)))
      event = (OBJECT * plural) p_nthObject (NULL_WIN, 0);
#else
   if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
      event = (OBJECT *) nthObject (NULL_WIN, 0);
#endif


                        /* if it was imported then get the imported block */
   if (event->shape == IMPORT)
   {
      if (layerProp = getClosestImportBlock ((IMPORT_OPTIONS *) event->options,
                                             xLoc, yLoc, zLoc, &layerIndex))
      {
         *piLayer = *piLayer + layerIndex;
      }
   }

                    /* if it was NOT imported or the Imported block was
                    ** "see through" then get the strat layer */
   if (!layerProp)
   {
      
#ifdef _MPL
      if (!(stratOptions = (STRATIGRAPHY_OPTIONS * plural)
                            p_getStratigraphyOptionsStructure (event)))
      {
         if (!(stratOptions = (STRATIGRAPHY_OPTIONS * plural)
            p_getStratigraphyOptionsStructure ((OBJECT * plural) p_nthObject (NULL_WIN, 0))))
         return;  /* not even a base strat is defined yet */
      }
#else
      if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
                         getStratigraphyOptionsStructure (event)))
      {
         if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
            getStratigraphyOptionsStructure ((OBJECT *) nthObject (NULL_WIN, 0))))
         return;  /* not even a base strat is defined yet */
      }
#endif

      numLayers = stratOptions->numLayers;
      height[0] = stratOptions->properties[0].height;
      height[1] = stratOptions->properties[1].height;
      height[2] = stratOptions->properties[2].height;
      height[3] = stratOptions->properties[3].height;
      height[4] = stratOptions->properties[4].height;
      height[5] = stratOptions->properties[5].height;
      height[6] = stratOptions->properties[6].height;
                                     /* only 1 layer */
      if (numLayers == 1)
      {
         *piLayer = *piLayer + 1;
      }                                 /* below the bottom */
      else if (zLoc <= height[1])
      {
         *piLayer = *piLayer + 1;
      }                                 /* above the top */
      else if (zLoc > height[numLayers-1])
      {
         *piLayer = *piLayer + numLayers;
      }
      else
      {
         for (l2 = 1; l2 < (numLayers-1); l2++)
         {
            if ((zLoc > height[l2]) && (zLoc <= height[l2+1]))
            {
               *piLayer = *piLayer + l2 + 1;
            }
         }
      }
   }

   *lutindex = *piLayer;
   *piStrati = 0;
}                                                                    

 
LAYER_PROPERTIES *whichLayer(eventIndex, xLoc, yLoc, zLoc)
int eventIndex;
double xLoc, yLoc, zLoc;
{
   int numLayers, layer;
   OBJECT *event;
   STRATIGRAPHY_OPTIONS *stratOptions;
   LAYER_PROPERTIES *layerProp = NULL;
   int layerIndex;

   if (!(event = (OBJECT *) nthObject (NULL_WIN, eventIndex)))
      if (!(event = (OBJECT *) nthObject (NULL_WIN, 0)))
			return (NULL);

   if (event->shape == IMPORT)
      layerProp = getClosestImportBlock ((IMPORT_OPTIONS *) event->options,
                                          xLoc, yLoc, zLoc, &layerIndex);

        /* Just a Stratigraphy or a SEE THROUGH of an Import block */
   if (!layerProp)
   {
      if ((event->shape == STRATIGRAPHY)
                            || (event->shape == UNCONFORMITY))
      {
         if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
                        getStratigraphyOptionsStructure (event)))
            return ((LAYER_PROPERTIES *) NULL);

         numLayers = stratOptions->numLayers;
                         
         if (numLayers == 1)              /* only 1 layer */
            layerProp = &(stratOptions->properties[0]);
                                       /* below the bottom */
         else if (zLoc <= stratOptions->properties[1].height)
            layerProp = &(stratOptions->properties[0]);
                                       /* above the top */
         else if (zLoc > stratOptions->properties[numLayers-1].height)
            layerProp = &(stratOptions->properties[numLayers-1]);
         else
         {
            for (layer = 1; layer < (numLayers-1); layer++)
            {
               if ((zLoc > stratOptions->properties[layer].height) &&
                   (zLoc <= stratOptions->properties[layer+1].height))
               {  
                  layerProp = &(stratOptions->properties[layer]);
               }
            }
         }
      }
      else  /* an Igneous Layer */
      {
         layerProp = getLayerPropertiesStructure (event);
      }
   }
   
   return (layerProp);
}                                                                    
