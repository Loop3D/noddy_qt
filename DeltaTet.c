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
** If two sets of two apices have the same code, each AA and BB edge can be
** cut by a stratigraphic level, and each point can be projected like the
** Gamma codes onto the AA-BB break plane, which is a trapezoid. Intersecting
** projected points from each side of the break plane will divide the break
** plane up into little trapezoids which can then be split into triangles
**
**
**                         * - stratigraphic layer intersections
**                         X - break plane intersections  
**                         + - projection of strat layer onto break plane  
**                      A........*.........A.........*........A  
**                     . .       +        . .        +       . 
**                    .   .      +       .   .       +      .  
**                   .     .     +      .     .      +     .
**                  .       .    +     .       .     +    .  
**                 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    
**                .   +       .      .      +    .      .      
**               .   +         .    .        +    .    . 
**              .   +           .  .          +    .  .   
**             .   +             ..            +    ..     
**            B...*..............B..............*...A     
**
**
**                       break plane with two projected strat intersections    
**                 ..............+......................
**                  .             +                     .         
**                   .             +                     .       
**                    .             +                     .     
**                     .             +                     .   
**                      .             +                     . 
**                       .             +                     .     
**                        .             +                     .   
**                         .             +                     . 
**                          .             +                     .   
**                           +++++++++++++++++++++++++++++++++++++  
**                            .             +                     . 
**                             ..............+......................
**                                                     
**
**
*************************************************************************** */
#include "xvt.h"
#include "noddy.h"
#include "allSurf.h"

extern COLOR layerColor;
extern long connect;
extern THREED_VIEW_OPTIONS threedViewOptions;

extern char clayer[81];/*global array for current layer name*/

extern LAYER_PROPERTIES *renderLayer ();

                   /* External Functions */
#if XVT_CC_PROTO
extern double distanceToContact (double, double, double, OBJECT *);
extern void allDrawPlane(double [4][3]);
extern OBJECT *SetCLayer(unsigned char *, unsigned char *, int, int);
#else
extern double distanceToContact ();
extern void allDrawPlane();
extern OBJECT *SetCLayer();
#endif

#if XVT_CC_PROTO
int DeltaFindEdgeMids(double [8][3], TETINFO *);
int DeltaFindMids(double, double [8], double [8][3], TETINFO *, int [8], int , int *);
int DeltaCalcPlanes(TETINFO *, int, int);
int DeltaBreakPlane( TETINFO *, int [8]);
double MidVal(double,double, double);
#else
int DeltaFindEdgeMids();
int DeltaFindMids();
int DeltaCalcPlanes();
int DeltaBreakPlane();
double MidVal();
#endif

double dbreakmids[9][2][3][3];
double dbreakdel[9][2];
double dEdgeMids[4][3];

/*
** calculate strat surfaces if 2 sets of 2 corners of tet are part of same
** contiguous volume
*/
void
#if XVT_CC_PROTO
DeltaCode(double Values[8], double Points[8][3],
          int SeqCode[8], TETINFO *t)
#else
DeltaCode(Values, Points, SeqCode, t)
double Values[8], Points[8][3];
int SeqCode[8];
TETINFO *t;
#endif
{
   unsigned int flavor;
   int index,NMids,nn;
   double level;
   OBJECT *event;
   int numEvents = countObjects(NULL_WIN);
   LAYER_PROPERTIES *properties;    
    
   t->pC=1;
   t->pC2=1;
            
   if (t->TetCode==101) /* work out which vertex is different */
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][2];
      t->GoodPts[2]=TETAPICES[t->tinc][1];
      t->GoodPts[3]=TETAPICES[t->tinc][3];
    
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][1];
   }
   else if(t->TetCode==11)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][1];
      t->GoodPts[2]=TETAPICES[t->tinc][2];
      t->GoodPts[3]=TETAPICES[t->tinc][3];
    
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][2];
   }
   else if(t->TetCode==110)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][3];
      t->GoodPts[2]=TETAPICES[t->tinc][1];
      t->GoodPts[3]=TETAPICES[t->tinc][2];
    
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][1];
   }

   DeltaFindEdgeMids(Points,t);
   
   for(nn=0;nn<2;nn++)
   {
                                                    /* get strat info  */
      taste(numEvents,(unsigned char *) &(t->cypher[SeqCode[t->GoodPts[2*nn]]][0]),&flavor,&index); 
       
                        /* if all same value exit (too hard to deal with!) */
      if (Values[TETAPICES[t->tinc][0]]==Values[TETAPICES[t->tinc][1]] &&
          Values[TETAPICES[t->tinc][0]]==Values[TETAPICES[t->tinc][2]] &&
          Values[TETAPICES[t->tinc][0]]==Values[TETAPICES[t->tinc][3]])
          return;

      if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
         return;

      if (flavor != IGNEOUS_STRAT)
      {
         event->generalData = 0;
         while (properties = renderLayer(event, index))
         {
            NMids=0;
            level = properties->height;
            layerColor=XVT_MAKE_COLOR(properties->color.red,
                                      properties->color.green,
                                      properties->color.blue);
   
            sprintf(clayer,"S%02dL%02d%04d", index, event->generalData-1,
                                             SeqCode[t->GoodPts[2*nn]]);
            DeltaFindMids(level, Values, Points, t, SeqCode, nn, &NMids);

            DeltaCalcPlanes(t, nn, NMids); /* draw strat surfaces */
         }
      }
   }
        
   DeltaBreakPlane(t, SeqCode); /* draw break surfaces */
}

int
#if XVT_CC_PROTO
DeltaFindEdgeMids(double Points[8][3], TETINFO *t)
#else
DeltaFindEdgeMids(Points, t)
double Points[8][3];
TETINFO *t;
#endif
{
   int mm;
   
   for(mm=0;mm<3;mm++)
      dEdgeMids[0][mm]=MidVal(Points[t->GoodPts[0]][mm],Points[t->GoodPts[2]][mm],0.5);
      
   for(mm=0;mm<3;mm++)
      dEdgeMids[1][mm]=MidVal(Points[t->GoodPts[0]][mm],Points[t->GoodPts[3]][mm],0.5);
      
   for(mm=0;mm<3;mm++)
      dEdgeMids[2][mm]=MidVal(Points[t->GoodPts[1]][mm],Points[t->GoodPts[2]][mm],0.5);
      
   for(mm=0;mm<3;mm++)
      dEdgeMids[3][mm]=MidVal(Points[t->GoodPts[1]][mm],Points[t->GoodPts[3]][mm],0.5);      

   return (TRUE);
}


int
#if XVT_CC_PROTO
DeltaFindMids(double level, double Values[8], double Points[8][3], 
                            TETINFO *t, int SeqCode[8], int nn, int *NMids)
#else
DeltaFindMids(level, Values, Points, t, SeqCode, nn, NMids)
double level, Values[8], Points[8][3]; 
TETINFO *t;
int SeqCode[8], nn, *NMids;
#endif
{
   int mm,nomids;
   static int pairs[2][4]={0,2,1,3,0,1,2,3};
   double delcon;
   
   if(nn==0)
      nomids=t->pC;
   else
      nomids=t->pC2;
   
   if(Values[t->GoodPts[2*nn]] < level && Values[t->GoodPts[(2*nn)+1]] > level ||
      Values[t->GoodPts[2*nn]] > level && Values[t->GoodPts[(2*nn)+1]] < level )
   {
      delcon=(level-Values[t->GoodPts[2*nn]])
                / (Values[t->GoodPts[(2*nn)+1]]-Values[t->GoodPts[2*nn]]);
      
      dbreakdel[nomids][nn]=delcon;
      
      for(mm=0;mm<3;mm++)
      {
         dbreakmids[nomids][nn][0][mm]=MidVal(dEdgeMids[pairs[nn][0]][mm],dEdgeMids[pairs[nn][1]][mm],delcon);
         dbreakmids[nomids][nn][1][mm]=MidVal(dEdgeMids[pairs[nn][2]][mm],dEdgeMids[pairs[nn][3]][mm],delcon);
         dbreakmids[nomids][nn][2][mm]=MidVal(Points[t->GoodPts[2*nn]][mm],Points[t->GoodPts[(2*nn)+1]][mm],delcon);
      }
      if(nn==0)
         t->pC++;
      else
         t->pC2++;
      
      *NMids=TRUE;
   }
   return (TRUE);
}

/*
** draw strat surfaces
*/
int
#if XVT_CC_PROTO
DeltaCalcPlanes(TETINFO *t, int nn, int NMids)
#else
DeltaCalcPlanes(t, nn, NMids)
TETINFO *t;
int nn, NMids;
#endif
{
   int mm,nomids;
   double conlist[4][3];

   if (nn==0)
      nomids=t->pC-1;
   else
      nomids=t->pC2-1;
   
   if(NMids>0)
   {
      for(mm=0;mm<3;mm++)
      {
         conlist[0][mm]=dbreakmids[nomids][nn][0][mm];
         conlist[1][mm]=dbreakmids[nomids][nn][1][mm];
         conlist[2][mm]=dbreakmids[nomids][nn][2][mm];
      }
      allDrawPlane(conlist);
   }
   return (TRUE);
}

/*
** draw break plane
*/
int
#if XVT_CC_PROTO
DeltaBreakPlane( TETINFO *t, int SeqCode[8])
#else
DeltaBreakPlane(t, SeqCode)
TETINFO *t;
int SeqCode[8];
#endif
{
   OBJECT *object;
   double conlist[4][3]; 
   int mm, nn, pp;

   if (!(object = SetCLayer((unsigned char *) &(t->cypher[SeqCode[t->InCode]]),
                  (unsigned char *) &(t->cypher[SeqCode[t->ExCode]]),
                   SeqCode[t->InCode], SeqCode[t->ExCode])))
       return (FALSE);
       
   for(mm=0;mm<3;mm++)
   {
      dbreakmids[0][0][0][mm]=dEdgeMids[0][mm];
      dbreakmids[0][0][1][mm]=dEdgeMids[1][mm];
   
      dbreakmids[t->pC][0][0][mm]=dEdgeMids[2][mm];
      dbreakmids[t->pC][0][1][mm]=dEdgeMids[3][mm];
   
   
      dbreakmids[0][1][0][mm]=dEdgeMids[0][mm];
      dbreakmids[0][1][1][mm]=dEdgeMids[2][mm];
   
      dbreakmids[t->pC2][1][0][mm]=dEdgeMids[1][mm];
      dbreakmids[t->pC2][1][1][mm]=dEdgeMids[3][mm];
      
   }
   
   dbreakdel[0][0]=0;
   dbreakdel[0][1]=0;
   dbreakdel[t->pC][0]=1.0;
   dbreakdel[t->pC2][1]=1.0;
   
   
   for (pp = 0; pp < t->pC; pp++)
   {
      for (nn = 0; nn < t->pC2; nn++)
      {
         for (mm = 0; mm < 3; mm++)
         {
            conlist[0][mm]=MidVal(dbreakmids[pp][0][0][mm],dbreakmids[pp][0][1][mm],dbreakdel[nn][1]);
            conlist[1][mm]=MidVal(dbreakmids[pp][0][0][mm],dbreakmids[pp][0][1][mm],dbreakdel[nn+1][1]);
            conlist[2][mm]=MidVal(dbreakmids[pp+1][0][0][mm],dbreakmids[pp+1][0][1][mm],dbreakdel[nn][1]);
         }
         allDrawPlane(conlist); 
         for(mm=0;mm<3;mm++)
            conlist[0][mm]=MidVal(dbreakmids[pp+1][0][0][mm],dbreakmids[pp+1][0][1][mm],dbreakdel[nn+1][1]);
         allDrawPlane(conlist); 
      }
   }
   
    return (TRUE);
}
   
