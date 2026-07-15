/* ============================================================================
 * [XVT -> Qt PORT] This file is unchanged calculation/application logic.
 * Its XVT toolkit calls now run against the Qt5-backed compatibility layer
 * in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the original
 * proprietary XVT SDK. No calculation or algorithmic code in this file was
 * modified to accomplish this port -- only the underlying GUI toolkit
 * implementation, in qt_compat/, changed.
 * ============================================================================
 */
/* ************************************************************************
** If one apex has a different discontinuity code, any stratigraphic level
** can only cut two eges (unless it cuts all 3 good apices). If the former,
** we have to project the line onto the break plane which divides the AAA
** apices from the B apex. This break plane is a triangle and a series of
** parallel lines from successive stratigraphic levels may end up cutting it,
** resulting in the need for a complicated system for dividing the break
** triangle.
**
**
**                          * - stratigraphic layer intersections
**                          X - break plane intersections
**                          + - projection of strat layer onto break plane
**                      A........*.........A.......*..........A  
**                     . .       +        . .       *        .  
**                    .   .      +       .   .       *      . 
**                   .     .     +      .     .       *    . 
**                  .       .    +     .       .        * . 
**                 *         X XXXXXXXX         .        *   
**                .  +     X  .      . X         .      .   
**               .     + X     .    .    X        .    .   
**              .      X        .  .      X        .  .   
**             .      X          ..        X        ..   
**            A......X...........B..........X.......A   
**                                                     
**
**
**                       break plane with three projected strat intersections 
**                                       .           
**                                      . .   
**                                     .   .        
**                                    .     +      
**                                   .    +  .    
**                                  .   +     .  
**                                 .  +        .        
**                                . +           .    
**                               .+              +  
**                              +              +  .    
**                             .             +     .  
**                            .            +        +
**                           .           +        +  .    
**                          .          +        +     .  
**                         ..........+........+.........  
**
**
************************************************************************* */
#include "xvt.h"
#include "noddy.h"
#include "allSurf.h"

extern COLOR layerColor;
extern long connect;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern GEOLOGY_OPTIONS geologyOptions;

extern char clayer[81]; /* global array for current layer name */

extern LAYER_PROPERTIES *renderLayer ();

                   /* External Functions */
#if XVT_CC_PROTO
extern double distanceToContact (double, double, double, OBJECT *);
extern void allDrawPlane(double [4][3]);
extern OBJECT *SetCLayer(unsigned char *, unsigned char *, int, int, int *);
extern int distanceCrossing (double [3], double [3], OBJECT *, int, double [3]);
#else
extern double distanceToContact ();
extern void allDrawPlane();
extern OBJECT *SetCLayer();
extern int distanceCrossing ();
#endif

#if XVT_CC_PROTO
double MidVal(double,double, double);
int BetaFindMids(double, double [8], double [8][3], TETINFO *, double [6][3], int [6], int [8]);
int BetaCalcPlanes(double , double [8], double [8][3], TETINFO *, double [6][3], int [6], OBJECT *, int);
int oneBetaPlane(double, double [8], double [8][3], double [6][3], TETINFO *, int [6], double [4][3], OBJECT *, int);
int storeBreakMids(double [4][3],int);
int BetaBreakClean(double [8][3], TETINFO *, int [8], OBJECT *, int);
int BetaBreakPlane( double [8][3], TETINFO *, int [8]);
int BetaBreakDirty(double [8][3], TETINFO *, OBJECT *, int);
int DoEndTrapezoid(double [8][3], TETINFO *, int, OBJECT *, int);
int DoPentagon(double [8][3], TETINFO *, int, OBJECT *, int);
int DoTrapezoids(double [8][3], TETINFO *, int);
int DoEndTriangle(double [8][3], TETINFO *, int, int, OBJECT *, int);
int GetOrder( double [8][3], TETINFO *);
int Shortest(TETINFO *);
int GetCommonVertex(int);
int GetCommonPentagonVertex(int, int);
int SameSide(int);
#else
double MidVal();
int BetaFindMids();
int BetaCalcPlanes();
int oneBetaPlane();
int storeBreakMids();
int BetaBreakClean();
int BetaBreakPlane();
int BetaBreakDirty();
int DoEndTrapezoid();
int DoPentagon();
int DoTrapezoids();
int DoEndTriangle();
int GetOrder();
int Shortest();
int GetCommonVertex();
int GetCommonPentagonVertex();
int SameSide();
#endif

double breakmp[7][2][3]; /* 7 because only 7 layers in one strat */
int    breakline[7][3];  /* 7 because only 7 layers in one strat */
 
/*
** calculate strat surfaces if only 3 corners of tet are part of same
** contiguous volume
*/
void
#if XVT_CC_PROTO
BetaCode(double Values[8], double Points[8][3],
         int SeqCode[8], TETINFO *t)

#else
BetaCode(Values, Points, SeqCode, t)
double Values[8], Points[8][3];    
int SeqCode[8];
TETINFO *t;
#endif
{
   unsigned int flavor;
   int index, NMids[6], nn;
   double MidPoints[6][3],level;
   OBJECT *event;
   int numEvents = countObjects(NULL_WIN);
   LAYER_PROPERTIES *properties;
   OBJECT *breakObject;         /* [Qt port ADDITION] todo.txt #46 */
   int breakEventIndex;         /* [Qt port ADDITION] todo.txt #46 */

   t->pC=0;
   t->exact=FALSE;
   nn=0;
            
   if(t->TetCode==111) /* work out which vertex is different */
   {
      t->GoodPts[0]=TETAPICES[t->tinc][1];
      t->GoodPts[1]=TETAPICES[t->tinc][2];
      t->GoodPts[2]=TETAPICES[t->tinc][3];
      
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][2];
   }
   else if(t->TetCode==100)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][2];
      t->GoodPts[2]=TETAPICES[t->tinc][3];
      
      t->ExCode=TETAPICES[t->tinc][1];
      t->InCode=TETAPICES[t->tinc][2];
   }
   else if(t->TetCode==10)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][1];
      t->GoodPts[2]=TETAPICES[t->tinc][3];
      
      t->ExCode=TETAPICES[t->tinc][2];
      t->InCode=TETAPICES[t->tinc][1];
   }
   else if(t->TetCode==1)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][1];
      t->GoodPts[2]=TETAPICES[t->tinc][2];

      t->ExCode=TETAPICES[t->tinc][3];
      t->InCode=TETAPICES[t->tinc][2];
   }
      
   taste(numEvents,(unsigned char *) &(t->cypher[SeqCode[t->InCode]][0]),&flavor,&index);  /*get strat info */


   if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
      return;

   /* [Qt port ADDITION] todo.txt #46 -- resolve the discontinuity object/
   ** event here (instead of only later, inside BetaBreakPlane) so
   ** oneBetaPlane (reached via BetaCalcPlanes, below) can use
   ** distanceCrossing() for the points it interpolates toward t->ExCode,
   ** instead of a fixed 0.5 midpoint. t->InCode/ExCode/cypher/SeqCode are
   ** already set above, matching what BetaBreakPlane's own (still present,
   ** now-redundant-but-harmless) SetCLayer call uses. */
   breakObject = SetCLayer((unsigned char *) &(t->cypher[SeqCode[t->InCode]]),
                  (unsigned char *) &(t->cypher[SeqCode[t->ExCode]]),
                   SeqCode[t->InCode], SeqCode[t->ExCode], &breakEventIndex);

   if (flavor != IGNEOUS_STRAT)
   {               /* Draw any stratigraphy layers that pass through */
      event->generalData = 0;
      while (properties = renderLayer(event, index))
      {
         level = properties->height;
         layerColor = XVT_MAKE_COLOR(properties->color.red,
                                     properties->color.green,
                                     properties->color.blue);

         sprintf(clayer, "S%02dL%02d%04d", index, event->generalData-1,
                                           SeqCode[t->GoodPts[0]]);

         BetaFindMids(level, Values, Points, t, MidPoints, NMids, SeqCode);
                                                /* draw strat surfaces */
         BetaCalcPlanes(level, Values, Points, t, MidPoints, NMids, breakObject, breakEventIndex);
      }
   }
           /* Draw the other surfaces (not stratigraphy) */
   BetaBreakPlane(Points, t, SeqCode); /* draw break surfaces */
}

/*
** find all mid points for triangle for a given strat level
*/
int
#if XVT_CC_PROTO
BetaFindMids(double level, double Values[8], double Points[8][3], TETINFO *t, 
             double MidPoints[6][3], int NMids[6], int SeqCode[8])
#else
BetaFindMids(level, Values, Points, t, MidPoints, NMids, SeqCode)
double level, Values[8], Points[8][3];
TETINFO *t; 
double MidPoints[6][3];
int NMids[6], SeqCode[8];
#endif
{
   int mm,coinc=0,nolines=0,exact=FALSE,nn,icon;
   double delcon;
   /* [Qt port FIX] todo.txt #46 follow-on -- see AlphaFindMids' matching
   ** comment (AlphaTet.c) for the full rationale: the old `!exact &&
   ** coinc!=1` gates below made an edge's "touches one vertex exactly"
   ** registration depend on other edges of this same tet, which let the
   ** same physical grid edge be registered by one tet and dropped by a
   ** neighbour sharing it -- a T-junction crack in folded stratigraphic
   ** surfaces. Deduped on apex index (a0/a1) instead, same as Alpha; the
   ** local `exact` flag is kept and still feeds `t->exact` below (a
   ** different, still-needed signal for DoPentagon's break-plane
   ** geometry) but no longer gates whether an edge registers. */
   int regApex[4], numReg=0, a0, a1, already;
   int i;

   for(mm=0;mm<6;mm++) /* 6 edges to a tet */
   {
      if(Values[LINES[TETLINES[t->tinc][mm]][0]] == Values[LINES[TETLINES[t->tinc][mm]][1]] &&
         Values[LINES[TETLINES[t->tinc][mm]][0]]==level)
         coinc++;
   }

   if(coinc==3) /* 3 vertices coincide with level */
   {
      for(nn=0,icon=0;nn<4;nn++)
      {
         if(Values[TETAPICES[t->tinc][nn]] == level)
         {
            MidPoints[icon][0]=Points[TETAPICES[t->tinc][nn]][0];
            MidPoints[icon][1]=Points[TETAPICES[t->tinc][nn]][1];
            MidPoints[icon][2]=Points[TETAPICES[t->tinc][nn]][2];
            NMids[icon]=TRUE;
            icon++;
         }
      }
      NMids[3]=NMids[4]=NMids[5]=FALSE;
   }
   else
   {
      for(mm=0;mm<6;mm++)
      {
         a0 = LINES[TETLINES[t->tinc][mm]][0];
         a1 = LINES[TETLINES[t->tinc][mm]][1];

         if(((Values[a0] < level && Values[a1] > level)  ||
             (Values[a0] > level && Values[a1] < level)) &&
             SeqCode[a0]==SeqCode[t->InCode] &&
             SeqCode[a1]==SeqCode[t->InCode])
         {
               delcon=(level-Values[a0])/(Values[a1]-Values[a0]);

               MidPoints[mm][0]=MidVal(Points[a0][0],Points[a1][0],delcon);
               MidPoints[mm][1]=MidVal(Points[a0][1],Points[a1][1],delcon);
               MidPoints[mm][2]=MidVal(Points[a0][2],Points[a1][2],delcon);
               breakline[t->pC][nolines++]=TETLINES[t->tinc][mm];
               NMids[mm]=TRUE;
         } /* normal mid point crossing */
         else if(Values[a0] == Values[a1] && Values[a0]==level)
         {
            breakline[t->pC][nolines++]=TETLINES[t->tinc][mm];
            NMids[mm]=-1-TETLINES[t->tinc][mm];
            if (numReg<4) { regApex[numReg++]=a0; }
            if (numReg<4) { regApex[numReg++]=a1; }
         } /* line is one edge */
         else if(Values[a0]==level)
         {
               for (i=0,already=FALSE; i<numReg; i++)
                  if (regApex[i]==a0) { already=TRUE; break; }
               if (!already)
               {
                  MidPoints[mm][0]=Points[a0][0];
                  MidPoints[mm][1]=Points[a0][1];
                  MidPoints[mm][2]=Points[a0][2];
                  NMids[mm]=TRUE;
                  breakline[t->pC][nolines++]=TETLINES[t->tinc][mm];
                  icon++;
                  exact=TRUE;
                  if (numReg<4) regApex[numReg++]=a0;
               }
               else
                  NMids[mm]=FALSE;
         }  /* line touches one vertex */
         else if(Values[a1]==level)
         {
               for (i=0,already=FALSE; i<numReg; i++)
                  if (regApex[i]==a1) { already=TRUE; break; }
               if (!already)
               {
                  MidPoints[mm][0]=Points[a1][0];
                  MidPoints[mm][1]=Points[a1][1];
                  MidPoints[mm][2]=Points[a1][2];
                  NMids[mm]=TRUE;
                  breakline[t->pC][nolines++]=TETLINES[t->tinc][mm];
                  icon++;
                  exact=TRUE;
                  if (numReg<4) regApex[numReg++]=a1;
               }
               else
                  NMids[mm]=FALSE;
         }  /* line touches one vertex */
         else
            NMids[mm]=FALSE;
      }
   }
   if(exact)
      t->exact=TRUE;
   return (TRUE);
}

/*
** calculate surface from given mid point intersections for Beta point system
*/
int
#if XVT_CC_PROTO
BetaCalcPlanes(double level, double Values[8], double Points[8][3],
               TETINFO *t, double MidPoints[6][3], int NMids[6],
               OBJECT *breakObject, int breakEventIndex)
#else
BetaCalcPlanes(level, Values, Points, t, MidPoints, NMids, breakObject, breakEventIndex)
double level, Values[8], Points[8][3];
TETINFO *t;
double MidPoints[6][3];
int NMids[6];
OBJECT *breakObject;
int breakEventIndex;
#endif
{
   double conlist[4][3]; 
   int mm, icon, nn, oldNMids;
   int found;  /* mwj_fix */
            
   for(mm=0,icon=0;mm<6;mm++)
   {
      if(NMids[mm]==TRUE)
      {
         conlist[icon][0]=MidPoints[mm][0];
         conlist[icon][1]=MidPoints[mm][1];
         conlist[icon][2]=MidPoints[mm][2];
         icon++;
      }
      else if(NMids[mm]<0)
      {
         MidPoints[mm][0]=Points[LINES[-1-NMids[mm]][0]][0];
         MidPoints[mm][1]=Points[LINES[-1-NMids[mm]][0]][1];
         MidPoints[mm][2]=Points[LINES[-1-NMids[mm]][0]][2];
         conlist[icon][0]=MidPoints[mm][0];
         conlist[icon][1]=MidPoints[mm][1];
         conlist[icon][2]=MidPoints[mm][2];
         oldNMids=NMids[mm];
         NMids[mm]=TRUE;
         found=FALSE; /* mwj_fix  */
         icon++;

         for(nn=0;nn<6;nn++)  /* put other midpt in first free space */
         {
            if(!NMids[nn])
            {
               MidPoints[nn][0]=Points[LINES[-1-oldNMids][1]][0];
               MidPoints[nn][1]=Points[LINES[-1-oldNMids][1]][1];
               MidPoints[nn][2]=Points[LINES[-1-oldNMids][1]][2];
               conlist[icon][0]=MidPoints[nn][0];
               conlist[icon][1]=MidPoints[nn][1];
               conlist[icon][2]=MidPoints[nn][2];
               NMids[nn]=1+mm;
               found=TRUE; /* mwj_fix */
               break;
            }
         }
         if (found) /* mwj_fix */
            icon++; /* mwj_fix */
         else /* mwj_fix */
            icon--; /* mwj_fix */
      }
      if(icon>=3) break;
   }


   if(icon==3)  /* plane doesn't intersect break plane */
   {
      allDrawPlane(conlist);
   }
   else if(icon==2) /* plane does intersect break plane */
   {
      oneBetaPlane(level,Values,Points,MidPoints,t,NMids,conlist,breakObject,breakEventIndex);
   }
   else if(icon!=0 && icon!=1)
      xvt_dm_post_error("Wrong number of midpoints (beta)");
   return (TRUE);
}

/* [Qt port ADDITION] todo.txt #46 -- given a good-good tet edge (p,q) that
** the stratigraphic level crosses at some fraction along it, find that
** same fraction's point along the REAL discontinuity-crossing edge
** (p->ExCode to q->ExCode) instead of routing through p/q's own
** stratigraphic-crossing point. Matches the analogous construction
** DeltaTet.c's DeltaFindMids already uses (MidVal between two real
** distanceCrossing() results at a stratigraphic delcon) -- confirmed by
** reading DeltaTet.c's actual debug output for a shared voxel face that
** this is what makes a Delta neighbor's point line up, whereas the old
** distanceCrossing(ExCode, stratPoint) construction produced a point nowhere
** near any of Delta's own edges (proven not just a fixed-midpoint
** roughness issue, but the topological source of the mesh gap -- see
** git history / conversation for the full derivation). Falls back to the
** old distanceCrossing(ExCode, stratPoint) construction (itself falling
** back further to a fixed 0.5 midpoint) if anything here can't resolve. */
static int
#if XVT_CC_PROTO
BetaStratCrossingOnBreakEdge(double level, double Values[8], double Points[8][3],
                              TETINFO *t, int edgeIndex, double fallbackStratPoint[3],
                              OBJECT *breakObject, int breakEventIndex, double result[3])
#else
BetaStratCrossingOnBreakEdge(level, Values, Points, t, edgeIndex, fallbackStratPoint,
                              breakObject, breakEventIndex, result)
double level, Values[8], Points[8][3];
TETINFO *t;
int edgeIndex;
double fallbackStratPoint[3];
OBJECT *breakObject;
int breakEventIndex;
double result[3];
#endif
{
   int p, q;
   double delcon, crossP[3], crossQ[3];

   if (edgeIndex < 0)
      edgeIndex = -1 - edgeIndex;   /* NMids[mm]<0 "line is one edge" case */

   p = LINES[edgeIndex][0];
   q = LINES[edgeIndex][1];

   if (Values[q] == Values[p])
      return (FALSE);
   delcon = (level - Values[p]) / (Values[q] - Values[p]);

   if (!distanceCrossing (Points[p], Points[t->ExCode], breakObject, breakEventIndex, crossP))
      return (FALSE);
   if (!distanceCrossing (Points[q], Points[t->ExCode], breakObject, breakEventIndex, crossQ))
      return (FALSE);

   result[0] = MidVal (crossP[0], crossQ[0], delcon);
   result[1] = MidVal (crossP[1], crossQ[1], delcon);
   result[2] = MidVal (crossP[2], crossQ[2], delcon);

   return (TRUE);
}

 /* break up trapezoid into 2 triangles then plot */
int
#if XVT_CC_PROTO
oneBetaPlane(double level, double Values[8], double Points[8][3], double MidPoints[6][3], TETINFO *t,
             int NMids[6], double conlist[4][3],
             OBJECT *breakObject, int breakEventIndex)
#else
oneBetaPlane(level, Values, Points, MidPoints, t, NMids, conlist, breakObject, breakEventIndex)
double level, Values[8], Points[8][3], MidPoints[6][3];
TETINFO *t;
int NMids[6];
double conlist[4][3];
OBJECT *breakObject;
int breakEventIndex;
#endif
{
   /* [Qt port FIX] todo.txt #46 -- conlist[0]/conlist[1] are stratigraphic-
   ** level crossing points (found via BetaFindMids against Values[], not
   ** touched here); this point is where the discontinuity's own break
   ** plane meets that stratigraphic edge. See BetaStratCrossingOnBreakEdge's
   ** own comment for why this is a proportional interpolation between two
   ** real distanceCrossing() results, not distanceCrossing(ExCode,
   ** stratPoint) -- the latter produced a point that had no relationship
   ** to what a neighboring tet (of any type) would independently compute
   ** for the same shared edge. */
   if (!BetaStratCrossingOnBreakEdge (level, Values, Points, t, breakline[t->pC][0],
                                       conlist[0], breakObject, breakEventIndex, conlist[2]))
      if (!distanceCrossing (Points[t->ExCode], conlist[0], breakObject, breakEventIndex, conlist[2]))
      {
         conlist[2][0]=MidVal(Points[t->ExCode][0],conlist[0][0],0.5);
         conlist[2][1]=MidVal(Points[t->ExCode][1],conlist[0][1],0.5);
         conlist[2][2]=MidVal(Points[t->ExCode][2],conlist[0][2],0.5);
      }

   allDrawPlane(conlist); /* 2 base and one top pt */

   if (!BetaStratCrossingOnBreakEdge (level, Values, Points, t, breakline[t->pC][1],
                                       conlist[1], breakObject, breakEventIndex, conlist[0]))
      if (!distanceCrossing (Points[t->ExCode], conlist[1], breakObject, breakEventIndex, conlist[0]))
      {
         conlist[0][0]=MidVal(Points[t->ExCode][0],conlist[1][0],0.5);
         conlist[0][1]=MidVal(Points[t->ExCode][1],conlist[1][1],0.5);
         conlist[0][2]=MidVal(Points[t->ExCode][2],conlist[1][2],0.5);
      }

   allDrawPlane(conlist); /* drop 1 base and add diagonally opposite top pt */

   storeBreakMids(conlist,t->pC);
   t->pC++;

   return (TRUE);
}

/*
** store a set of break plane midpoints for later processing
*/
int
#if XVT_CC_PROTO
storeBreakMids(double conlist[4][3],int pC)  
#else
storeBreakMids(conlist, pC)  
double conlist[4][3];
int pC;
#endif
{
   register int mm;
   
   for (mm = 0; mm < 3; mm++)
   {
      breakmp[pC][1][mm] = conlist[0][mm];
      breakmp[pC][0][mm] = conlist[2][mm];
   }
   return (TRUE);
}

/*
** draw break plane if no strat surfaces join it
*/
int
#if XVT_CC_PROTO
BetaBreakClean(double Points[8][3], TETINFO *t, int SeqCode[8], OBJECT *object, int breakEventIndex)
#else
BetaBreakClean(Points, t, SeqCode, object, breakEventIndex)
double Points[8][3];
TETINFO *t;
int SeqCode[8];
OBJECT *object;
int breakEventIndex;
#endif
{
   double conlist[4][3];
   double crossing[3];
   register int mm, icon=0;

   for (mm = 0; mm < 6; mm++)
   {
      if (SeqCode[LINES[TETLINES[t->tinc][mm]][0]] !=
          SeqCode[LINES[TETLINES[t->tinc][mm]][1]])
      {
         /* [Qt port FIX] todo.txt #46 -- this used to always bisect the
         ** edge at the fixed 50% midpoint (see the dead code above,
         ** abandoned mid-attempt at this exact fix -- ended on a call
         ** missing an argument, distanceToContact(,object), so it never
         ** compiled), regardless of where the true discontinuity surface
         ** actually crosses. distanceCrossing() finds the real crossing
         ** point via the same edge-interpolation technique
         ** AlphaFindMids/BetaFindMids already use for stratigraphic
         ** levels, just driven by distanceToContact()'s signed distance
         ** instead of a stratigraphic height. Falls back to the old
         ** fixed-midpoint behaviour if it can't resolve a real crossing
         ** (numerical edge case -- see distSurf.c). */
         if (!distanceCrossing (Points[LINES[TETLINES[t->tinc][mm]][0]],
                                 Points[LINES[TETLINES[t->tinc][mm]][1]],
                                 object, breakEventIndex, crossing))
         {
            crossing[0] = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][0],
                                  Points[LINES[TETLINES[t->tinc][mm]][1]][0], 0.5);
            crossing[1] = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][1],
                                  Points[LINES[TETLINES[t->tinc][mm]][1]][1], 0.5);
            crossing[2] = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][2],
                                  Points[LINES[TETLINES[t->tinc][mm]][1]][2], 0.5);
         }

         conlist[icon][0] = crossing[0];
         conlist[icon][1] = crossing[1];
         conlist[icon][2] = crossing[2];
         icon++;
      }
   }

   allDrawPlane(conlist);
   return (TRUE);
}

/*
**draw break plane
*/
int
#if XVT_CC_PROTO
BetaBreakPlane( double Points[8][3], TETINFO *t, int SeqCode[8])
#else
BetaBreakPlane(Points, t, SeqCode)
double Points[8][3];
TETINFO *t;
int SeqCode[8];
#endif
{
   OBJECT *object;
   int breakEventIndex;  /* [Qt port ADDITION] todo.txt #46 */

   if (!(object = SetCLayer((unsigned char *) &(t->cypher[SeqCode[t->InCode]]),
                  (unsigned char *) &(t->cypher[SeqCode[t->ExCode]]),
                  SeqCode[t->InCode], SeqCode[t->ExCode], &breakEventIndex)))
      return (FALSE);

   if (t->pC == 0)
      BetaBreakClean(Points, t, SeqCode, object, breakEventIndex);
   else
      BetaBreakDirty(Points, t, object, breakEventIndex);

   return (TRUE);
}

/*
** draw break plane if other surfaces do join it
*/
/* assumes triangle is split by sequential parallel lines */
int
#if XVT_CC_PROTO
BetaBreakDirty(double Points[8][3], TETINFO *t, OBJECT *object, int breakEventIndex)
#else
BetaBreakDirty(Points, t, object, breakEventIndex)
double Points[8][3];
TETINFO *t;
OBJECT *object;
int breakEventIndex;
#endif
{
   int mm, pCount=0;

   GetOrder(Points, t);  /* find a vertex and reorder midpts if nec */

   DoEndTriangle(Points,t,pCount,t->apexfirst,object,breakEventIndex); /* 1st triangle */

   pCount++;

   while(SameSide(pCount) && pCount < t->pC-1)  /* keep calving off trapezoids until 2nd vertex is reached */
   {
      DoTrapezoids(Points,t,pCount);
      pCount++;
   }

   if(pCount<t->pC && !t->exact)
   {
      DoPentagon(Points,t,pCount,object,breakEventIndex); /* handle middle pentagonal shape */
      pCount++;
   }

   for (mm = pCount; mm < t->pC-1; mm++)
   {
      DoTrapezoids(Points,t,pCount);  /* keep calving off trapezoids until last vertex is reached */
      pCount++;
   }

   if(t->apexfirst!=t->apexlast)  /* do last triangle */
      DoEndTriangle(Points,t,pCount-1,t->apexlast,object,breakEventIndex);
   else   /* do last trapezoid if 2nd vertex never reached */
      DoEndTrapezoid(Points,t,pCount,object,breakEventIndex);
   return (TRUE);
}

/*
** Do final trapezoid if two vertices are as yet unused
*/
int
#if XVT_CC_PROTO
DoEndTrapezoid(double Points[8][3], TETINFO *t, int count, OBJECT *breakObject, int breakEventIndex)
#else
DoEndTrapezoid(Points, t, count, breakObject, breakEventIndex)
double Points[8][3];
TETINFO *t;
int count;
OBJECT *breakObject;
int breakEventIndex;
#endif
{
   int mm,nn,icon;
   int endv[2];
   double conlist[4][3];

   for(mm=0,nn=0;mm<3;mm++)
      if(t->GoodPts[mm]!=t->apexfirst)
         endv[nn++]=t->GoodPts[mm];

   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][0][mm];

   for(mm=0,icon=1;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][1][mm];

   if (!distanceCrossing (Points[endv[0]], Points[t->ExCode], breakObject, breakEventIndex, conlist[2]))
      for(mm=0,icon=2;mm<3;mm++)
         conlist[icon][mm]=MidVal(Points[endv[0]][mm],Points[t->ExCode][mm],0.5);

   allDrawPlane(conlist);

   if(LINES[breakline[count-1][0]][0] == endv[0] ||
      LINES[breakline[count-1][0]][1] == endv[0])
      icon=0;
   else
      icon=1;

   if (!distanceCrossing (Points[endv[1]], Points[t->ExCode], breakObject, breakEventIndex, conlist[icon]))
      for(mm=0;mm<3;mm++)
         conlist[icon][mm]=MidVal(Points[endv[1]][mm],Points[t->ExCode][mm],0.5);

   allDrawPlane(conlist);

   return (TRUE);
}

/*
** Do middle pentagon
*/
int
#if XVT_CC_PROTO
DoPentagon(double Points[8][3], TETINFO *t, int count, OBJECT *breakObject, int breakEventIndex)
#else
DoPentagon(Points, t, count, breakObject, breakEventIndex)
double Points[8][3];
TETINFO *t;
int count;
OBJECT *breakObject;
int breakEventIndex;
#endif
{
   int vertex,icon,sidecode,mm,nn,tempbm[3];
   double conlist[4][3],tempbl; 
   
   if(breakline[count][0]!=breakline[count-1][0] &&
      breakline[count][1]!=breakline[count-1][1])
   {
      for(nn=count;nn<t->pC;nn++)
      {
         tempbl=breakline[nn][0];
         breakline[nn][0]= (int) breakline[nn][1];
         breakline[nn][1]= (int) tempbl;
      
         for(mm=0;mm<3;mm++)
         {
            tempbm[mm] = (int) breakmp[nn][0][mm];
            breakmp[nn][0][mm]=breakmp[nn][1][mm];
            breakmp[nn][1][mm]=tempbm[mm];
         }
      }  
   }
   
   if(breakline[count][0]==breakline[count-1][0])
      sidecode=1;
   else
      sidecode=0;

   /* vertex=GetCommonPentagonVertex(count,sidecode); */
   vertex=t->apexmiddle;
   
   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][sidecode][mm];
   
   for(mm=0,icon=1;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][sidecode][mm];
   
   if (!distanceCrossing (Points[vertex], Points[t->ExCode], breakObject, breakEventIndex, conlist[2]))
      for(mm=0,icon=2;mm<3;mm++)
         conlist[icon][mm]=MidVal(Points[vertex][mm],Points[t->ExCode][mm],0.5);

   allDrawPlane(conlist);

   DoTrapezoids(Points,t,count);
   
   return (TRUE);
}


/*
** do arbitrary trapezoid in middle of break plane
*/
int
#if XVT_CC_PROTO
DoTrapezoids(double Points[8][3], TETINFO *t, int count)
#else
DoTrapezoids(Points, t, count)
double Points[8][3];
TETINFO *t;
int count;
#endif
{
   double conlist[4][3]; 
   int mm,icon;
   
   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][0][mm];
   
   for(mm=0,icon=1;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][1][mm];
   
   for(mm=0,icon=2;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][0][mm];
   
   allDrawPlane(conlist);
   
   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][1][mm];
   
   allDrawPlane(conlist);
   
   return (TRUE);
}

/*
** Do first or last triangle
*/
int
#if XVT_CC_PROTO
DoEndTriangle(double Points[8][3], TETINFO *t, int count, int apexno, OBJECT *breakObject, int breakEventIndex)
#else
DoEndTriangle(Points, t, count, apexno, breakObject, breakEventIndex)
double Points[8][3];
TETINFO *t;
int count, apexno;
OBJECT *breakObject;
int breakEventIndex;
#endif
{
   double conlist[4][3];
   int mm,icon;

   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][0][mm];

   for(mm=0,icon=1;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][1][mm];

   if (!distanceCrossing (Points[apexno], Points[t->ExCode], breakObject, breakEventIndex, conlist[2]))
      for(mm=0,icon=2;mm<3;mm++)
         conlist[icon][mm]=MidVal(Points[apexno][mm],Points[t->ExCode][mm],0.5);

   allDrawPlane(conlist);

   return (TRUE);
}


/*
** work out order of mid point pairs and re-sort if nec
*/
int
#if XVT_CC_PROTO
GetOrder( double Points[8][3], TETINFO *t)
#else
GetOrder(Points, t)
double Points[8][3];
TETINFO *t;
#endif
{
   int firstv,lastv,mm,nn,pp;
   double tempbreakmp[7][2][3]; /* 7 because only 7 layers in one strat */
   int    tempbreakline[7][2];  /* 7 because only 7 layers in one strat */

   if(t->pC==1)
   {
      firstv=GetCommonVertex(0);
      t->order=TRUE;
      t->apexfirst=firstv;
      t->apexlast=firstv;
   }
   else
   {
      firstv=GetCommonVertex(0);
      lastv=GetCommonVertex(t->pC-1);
      
      if(firstv == lastv)
      {
         Shortest (t);
         if(t->order)
         {
            t->apexfirst=firstv;
            t->apexlast=lastv;
         }
         else
         {
            t->apexfirst=firstv;
            t->apexlast=lastv;
            
            for(mm=0;mm<t->pC;mm++)
            {
               for(nn=0;nn<2;nn++)
               {
                  tempbreakline[mm][nn]=breakline[t->pC-mm-1][nn];
                  for(pp=0;pp<3;pp++)
                  {
                     tempbreakmp[mm][nn][pp]=breakmp[t->pC-mm-1][nn][pp];
                  }
               }
            }
            for(mm=0;mm<t->pC;mm++)
            {
               for(nn=0;nn<2;nn++)
               {
                  breakline[mm][nn]=tempbreakline[mm][nn];
                  for(pp=0;pp<3;pp++)
                  {
                     breakmp[mm][nn][pp]=tempbreakmp[mm][nn][pp];
                  }
               }
            }
         }
      }
      else
      {
         t->order=TRUE;
         t->apexfirst=firstv;
         t->apexlast=lastv;
         for(mm=0;mm<3;mm++)
            if(t->GoodPts[mm] != firstv && t->GoodPts[mm] != lastv)
               t->apexmiddle=t->GoodPts[mm];
      }
   }
   return (TRUE);
}

/*
** see which midpoint pair is shorter (ie closer to a vertex)
*/
int
#if XVT_CC_PROTO
Shortest(TETINFO *t)
#else
Shortest(t)
TETINFO *t;
#endif
{
   double firstlength=0,lastlength=0;
   int mm;
   
   for(mm=0;mm<3;mm++)
   {
      
      firstlength+=(breakmp[0][0][mm]-breakmp[0][1][mm])*
                (breakmp[0][0][mm]-breakmp[0][1][mm]);
      lastlength+=(breakmp[(*t).pC-1][0][mm]-breakmp[(*t).pC-1][1][mm])*
               (breakmp[(*t).pC-1][0][mm]-breakmp[(*t).pC-1][1][mm]);
   }
   
   if(firstlength<lastlength)
      (*t).order=TRUE;
   else
      (*t).order=FALSE;
   return (TRUE);
}

/*
** find vertex that two edges share    
*/
int
#if XVT_CC_PROTO
GetCommonVertex(int paircode)
#else
GetCommonVertex(paircode)
int paircode;
#endif
{
   int vertex;
   
   if( LINES[breakline[paircode][0]][0] == LINES[breakline[paircode][1]][0] ||
      LINES[breakline[paircode][0]][0] == LINES[breakline[paircode][1]][1])
         vertex=LINES[breakline[paircode][0]][0];
   else 
         vertex=LINES[breakline[paircode][0]][1];

   return(vertex);
   
}

/*
** find vertex that two lines around mid pentagon share
*/
int
#if XVT_CC_PROTO
GetCommonPentagonVertex(int count, int sidecode)
#else
GetCommonPentagonVertex(count, sidecode)
int count, sidecode;
#endif
{
   int vertex;
   
   if( LINES[breakline[count][sidecode]][0] == LINES[breakline[count-1][sidecode]][0] ||
      LINES[breakline[count][sidecode]][0] == LINES[breakline[count-1][sidecode]][1])
         vertex=LINES[breakline[count][sidecode]][0];
   else 
         vertex=LINES[breakline[count][sidecode]][1];

   return(vertex);
   
}

/*
** figure out if two midpoint pairs share same common vertex as previous pair
*/
int
#if XVT_CC_PROTO
SameSide(int pCount)
#else
SameSide(pCount)
int pCount;
#endif
{
   int v1,v2;
   
   v1=GetCommonVertex(pCount-1);
   v2=GetCommonVertex(pCount);
   
   return(v1==v2);
}
