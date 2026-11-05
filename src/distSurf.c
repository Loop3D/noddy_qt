/* [Qt port ADDITION] todo.txt #46 -- new file, part of replacing the
** discontinuity-surface tetrahedra code's discrete/fixed-midpoint break-
** plane construction (the "egg carton" roughness) with a continuous
** signed-distance interpolation, matching the technique already used for
** stratigraphic surfaces (AlphaFindMids/BetaFindMids in AlphaTet.c/
** BetaTet.c) but driven by distanceToContact() (block.c) against a
** target level of 0 instead of a stratigraphic height.
**
** The per-point coordinate-frame handling here mirrors calcAlterationZone()
** (block.c) exactly, rather than re-deriving the event-history dispatch:
** reverse fully back to "creation" via the existing, already-proven
** reverseEvents() (unEvents.c -- the same function calcAlterationZone's own
** caller uses to build its xyzLoc[] array), then forward-model up to (but
** not including) the target event via the existing forwardModel()/fore()
** (events.c -- the same function calcAlterationZone itself calls). This
** avoids maintaining a second, hand-rolled copy of the un-event dispatch
** switch, and reuses two code paths already exercised in production.
*/
#include "xvt.h"
#include "noddy.h"
#include <math.h>

#if XVT_CC_PROTO
extern double distanceToContact (double, double, double, OBJECT *);
extern void reverseEventsIgnoringAgain (double ***, STORY **, int, int);
extern int forwardModel (int, int, double, double, double, double *, double *, double *);
extern double ***qdtrimat (int, int, int, int, int, int);
extern void freeqdtrimat (double ***, int, int, int, int, int, int);
extern STORY **strstomat (int, int, int, int);
extern void free_strstomat (STORY **, int, int, int, int);
extern void izero (unsigned char *);
extern BLOCK_VIEW_OPTIONS *getViewOptions (void);
extern void getSurfaceGridInfo (int *, double *);
double MidVal (double, double, double);
#else
extern double distanceToContact ();
extern void reverseEventsIgnoringAgain ();
extern int forwardModel ();
extern double ***qdtrimat ();
extern void freeqdtrimat ();
extern STORY **strstomat ();
extern void free_strstomat ();
extern void getSurfaceGridInfo ();
extern void izero ();
extern BLOCK_VIEW_OPTIONS *getViewOptions ();
double MidVal ();
#endif

/* [Qt port ADDITION] todo.txt #46 -- worldPositionBeforeEvent() (below) does
** real event-history work per call (a full reverse-to-creation walk plus a
** forward walk) -- relatively expensive, and the same cube corner is
** queried repeatedly (up to 5 tetrahedra per voxel share the voxel's 8
** corners, and each corner participates in several tet edges). Cache the
** result per (corner position, event) pair, cleared once per voxel by
** clearDistanceCrossingCache() (called from allSetVerts(), allSurf.c,
** right before that voxel's 5 tets are processed) -- a plain linear-scan
** cache is enough since the number of distinct (corner, relevant event)
** pairs touched within one voxel is small in practice. */
#define REVERSE_CACHE_SIZE 64
static struct {
   double x, y, z;
   int event;
   double rx, ry, rz;
} reverseCache[REVERSE_CACHE_SIZE];
static int reverseCacheCount = 0;

void
clearDistanceCrossingCache (void)
{
   reverseCacheCount = 0;
}

/* Computes the world-coordinate position of (x,y,z) as it existed just
** BEFORE event `eventIndex` (0-based) fired -- the frame distanceToContact()
** expects (its FAULT/UNCONFORMITY/PLUG surface equations are defined
** relative to the geometry as it existed right when that event activates,
** before ITS OWN displacement is applied; each event function does its own
** translate-into-local-frame internally). x,y,z must already be real-world
** coordinates (matching eventOptions->positionX/Y/Z), not the grid-relative
** coordinates allSurf.c's Points[] array uses -- see distanceCrossing(). */
static void
worldPositionBeforeEvent (double x, double y, double z, int eventIndex,
                           double *nx, double *ny, double *nz)
{
   double ***dots;
   STORY **histoire;
   double cx, cy, cz;

   *nx = x; *ny = y; *nz = z;   /* fail-safe default: no-op */

   if (!(dots = (double ***) qdtrimat (0, 1, 0, 1, 0, 3)))
      return;
   if (!(histoire = (STORY **) strstomat (0, 1, 0, 1)))
   {
      freeqdtrimat (dots, 0, 1, 0, 1, 0, 3);
      return;
   }

   dots[1][1][1] = x;
   dots[1][1][2] = y;
   dots[1][1][3] = z;
   histoire[1][1].again = TRUE;
   izero (histoire[1][1].sequence);

   reverseEventsIgnoringAgain (dots, histoire, 1, 1);   /* -> position at "creation" */

   cx = dots[1][1][1];
   cy = dots[1][1][2];
   cz = dots[1][1][3];

   freeqdtrimat (dots, 0, 1, 0, 1, 0, 3);
   free_strstomat (histoire, 0, 1, 0, 1);

   forwardModel (0, eventIndex, cx, cy, cz, nx, ny, nz);
}

static void
worldPositionBeforeEventCached (double x, double y, double z, int eventIndex,
                                 double *nx, double *ny, double *nz)
{
   int i;

   for (i = 0; i < reverseCacheCount; i++)
   {
      if ((reverseCache[i].x == x) && (reverseCache[i].y == y)
                                    && (reverseCache[i].z == z)
                                    && (reverseCache[i].event == eventIndex))
      {
         *nx = reverseCache[i].rx;
         *ny = reverseCache[i].ry;
         *nz = reverseCache[i].rz;
         return;
      }
   }

   worldPositionBeforeEvent (x, y, z, eventIndex, nx, ny, nz);

   if (reverseCacheCount < REVERSE_CACHE_SIZE)
   {
      reverseCache[reverseCacheCount].x = x;
      reverseCache[reverseCacheCount].y = y;
      reverseCache[reverseCacheCount].z = z;
      reverseCache[reverseCacheCount].event = eventIndex;
      reverseCache[reverseCacheCount].rx = *nx;
      reverseCache[reverseCacheCount].ry = *ny;
      reverseCache[reverseCacheCount].rz = *nz;
      reverseCacheCount++;
   }
}

/* Finds where the straight line between world-coordinate points A and B
** crosses `object`'s own discontinuity surface (distance == 0), using the
** exact edge-interpolation pattern AlphaFindMids/BetaFindMids already use
** for stratigraphic level crossings -- just substituting a signed
** distanceToContact() reading for a stratigraphic height.
**
** PointsA/PointsB come in as allSurf.c's Points[] array uses them: relative
** to the voxel grid's own index space (see allSetVerts(), allSurf.c --
** `((ii-1)+CUBE[mm][0])*plotscale`), NOT real-world coordinates -- they are
** missing the block model's own origin (viewOptions->originX/Y/Z, the same
** offset create3dStratMap(), calc3d.c, adds when it builds the grid that
** SeqCode/histoire.sequence were actually classified against). Every event's
** positionX/Y/Z, and everything reverseEvents()/forwardModel() do with them,
** is defined in real-world coordinates, so that offset is added back here
** before doing any event-history work. The crossing point returned is
** interpolated between the ORIGINAL (grid-relative) A/B, matching what every
** caller of this function (and allDrawPlane) expects to receive.
**
** Returns FALSE (caller should fall back to the old MidVal(A,B,0.5)) if
** A and B don't actually straddle zero, or `object` isn't a discontinuity
** type this function supports -- a defensive fallback for numerical edge
** cases (near-degenerate events, a corner landing exactly on the
** boundary), not expected to trigger often in normal use since
** SeqCode[A] != SeqCode[B] already implies a crossing exists between
** them (except for DYKE, deliberately excluded below -- see comment). */
/* [Qt port ADDITION] todo.txt #46 -- distanceToContact() at the point
** delcon of the way from PointsA to PointsB (Points[]-frame in, world-frame
** conversion + reverse-transform done internally, same as distanceCrossing
** itself). Used by distanceCrossing's bisection refinement below. */
static double
#if XVT_CC_PROTO
distanceAtDelcon (double PointsA[3], double PointsB[3], double delcon,
                   OBJECT *object, int eventIndex)
#else
distanceAtDelcon (PointsA, PointsB, delcon, object, eventIndex)
double PointsA[3], PointsB[3], delcon;
OBJECT *object;
int eventIndex;
#endif
{
   BLOCK_VIEW_OPTIONS *viewOptions;
   double originX, originY, originZ;
   int yMax;
   double blockSize;
   double px, py, pz, wx, wy, wz, rx, ry, rz;

   px = MidVal (PointsA[0], PointsB[0], delcon);
   py = MidVal (PointsA[1], PointsB[1], delcon);
   pz = MidVal (PointsA[2], PointsB[2], delcon);

   viewOptions = getViewOptions ();
   originX = viewOptions->originX;
   originY = viewOptions->originY;
   originZ = viewOptions->originZ - viewOptions->lengthZ;
   getSurfaceGridInfo (&yMax, &blockSize);

   wx = px + originX;
   wy = (yMax - 1) * blockSize - py + originY;
   wz = pz + originZ;

   worldPositionBeforeEventCached (wx, wy, wz, eventIndex, &rx, &ry, &rz);

   return (distanceToContact (rx, ry, rz, object));
}

int
#if XVT_CC_PROTO
distanceCrossing (double PointsA[3], double PointsB[3], OBJECT *object,
                   int eventIndex, double crossing[3])
#else
distanceCrossing (PointsA, PointsB, object, eventIndex, crossing)
double PointsA[3], PointsB[3];
OBJECT *object;
int eventIndex;
double crossing[3];
#endif
{
   BLOCK_VIEW_OPTIONS *viewOptions;
   double originX, originY, originZ;
   int yMax;
   double blockSize;
   double wxA, wyA, wzA, wxB, wyB, wzB;
   double rxA, ryA, rzA, rxB, ryB, rzB;
   double dA, dB, delcon;
   double loDelcon, hiDelcon, dLo, dHi;
   int iter;

   if (!object)
      return (FALSE);

   /* [Qt port NOTE] todo.txt #46 -- DYKE deliberately excluded here (not a
   ** temporary gap): dyke()'s (events.c) distmt is NOT a clean single-
   ** boundary signed distance -- a dyke has TWO walls, and dyke()'s STOPE
   ** branch returns a flat 0.0 for the whole inside region while its
   ** DILATION_DYKE branch conflates "outside the far wall" with "inside"
   ** (both land negative). FAULT/SHEAR_ZONE/UNCONFORMITY/PLUG are all single
   ** unambiguous surfaces (a plane, or -- confirmed by reading plug()'s
   ** consistent `distmt = r - contac` radial-distance pattern across all 4
   ** plug types -- a closed cylinder/cone/paraboloid/ellipsoid) and don't
   ** have this problem. Revisit DYKE separately once its two walls can be
   ** told apart. */
   switch (object->shape)
   {
      case FAULT:
      case SHEAR_ZONE:
      case UNCONFORMITY:
      case PLUG:
         break;
      default:
         return (FALSE);
   }

   viewOptions = getViewOptions ();
   originX = viewOptions->originX;
   originY = viewOptions->originY;
   originZ = viewOptions->originZ - viewOptions->lengthZ;
   getSurfaceGridInfo (&yMax, &blockSize);

   /* [Qt port NOTE] todo.txt #46 -- X and Z map to world coordinates by a
   ** plain +origin offset, but Y does not: allSetVerts() (allSurf.c) builds
   ** Points[]'s Y straight off the array index (`(jj-1+CUBE)*plotscale`),
   ** while calc3d.c's own world-Y-from-array-index formula flips it
   ** (`(yMax-1-y)*blockSize+yLoc`). Invert that flip here so distanceToContact
   ** (which needs true world coordinates matching eventOptions->positionY)
   ** gets the right value; see getSurfaceGridInfo()'s comment for the
   ** derivation. The FINAL crossing point is still interpolated between the
   ** original (Points[]-frame) A/B below, so this only affects the distance
   ** evaluation, not what gets handed to allDrawPlane(). */
   wxA = PointsA[0] + originX;
   wyA = (yMax - 1) * blockSize - PointsA[1] + originY;
   wzA = PointsA[2] + originZ;
   wxB = PointsB[0] + originX;
   wyB = (yMax - 1) * blockSize - PointsB[1] + originY;
   wzB = PointsB[2] + originZ;

   worldPositionBeforeEventCached (wxA, wyA, wzA, eventIndex, &rxA, &ryA, &rzA);
   worldPositionBeforeEventCached (wxB, wyB, wzB, eventIndex, &rxB, &ryB, &rzB);

   dA = distanceToContact (rxA, ryA, rzA, object);
   dB = distanceToContact (rxB, ryB, rzB, object);

   /* [Qt port FIX] todo.txt #46 -- a corner landing essentially exactly ON
   ** the discontinuity surface (dA or dB within DISTANCE_EPSILON of zero)
   ** used to require a strict opposite-sign pair to be treated as a real
   ** crossing -- if the OTHER endpoint's distance had the same sign as
   ** this near-zero value (both technically "inside", e.g. dA=-93.17,
   ** dB=-0.000000), the "both same side" check rejected it, falling back
   ** to the fixed 0.5 midpoint even though this corner IS (for all
   ** practical purposes) the crossing point. For a PLANAR surface that's a
   ** minor rounding difference; for a CURVED one (PLUG) it produced
   ** residual errors over 150 units -- a corner sitting near the apex of a
   ** steep paraboloid/cone can be numerically on the boundary while the
   ** naive midpoint toward the other (deeply inside) corner is nowhere
   ** near the true surface. Treat a near-zero endpoint as the crossing
   ** point directly instead of falling through to that fallback. */
#define DISTANCE_EPSILON 1.0e-6
   if (fabs (dA) < DISTANCE_EPSILON)
   {
      crossing[0] = PointsA[0]; crossing[1] = PointsA[1]; crossing[2] = PointsA[2];
      return (TRUE);
   }
   if (fabs (dB) < DISTANCE_EPSILON)
   {
      crossing[0] = PointsB[0]; crossing[1] = PointsB[1]; crossing[2] = PointsB[2];
      return (TRUE);
   }

   if (((dA < 0.0) && (dB < 0.0)) || ((dA > 0.0) && (dB > 0.0)))
      return (FALSE);    /* both same side -- no real crossing here */

   if (dA == dB)
      return (FALSE);    /* degenerate, avoid divide by zero */

   /* [Qt port FIX] todo.txt #46 -- a single linear interpolation of dA/dB
   ** assumes distanceToContact() varies linearly along this edge, true for
   ** a planar surface (FAULT/SHEAR_ZONE/UNCONFORMITY) but NOT for PLUG:
   ** plug()'s CONE_PLUG/PARABOLIC_PLUG branch has a genuine cusp at its
   ** apex plane (continuous value, discontinuous gradient -- confirmed by
   ** reading events.c's plug(): the "beside the cone" branch uses
   ** `r_radial - contac` while the "beyond the tip" branch uses
   ** `sqrt(grots[1]^2+r_radial^2)`, agreeing in value but not slope at the
   ** boundary). A single linear interpolation across that cusp produced
   ** residual errors over 150 units on a real model (confirmed via a
   ** debugPointDistanceToContact() residual check), visible as a "dimple"
   ** in the rendered plug surface. Bisection only relies on the sign
   ** change (intermediate value theorem), so it converges correctly
   ** regardless of smoothness -- refine the initial linear guess with a
   ** fixed number of bisection steps (each cheap: distanceToContact() is
   ** closed-form, and worldPositionBeforeEventCached() is memoized per
   ** voxel) rather than trusting it outright. */
   delcon = dA / (dA - dB);
   if (delcon < 0.0) delcon = 0.0;
   if (delcon > 1.0) delcon = 1.0;

   loDelcon = 0.0; dLo = dA;
   hiDelcon = 1.0; dHi = dB;
   for (iter = 0; iter < 12; iter++)
   {
      double dMid = distanceAtDelcon (PointsA, PointsB, delcon, object, eventIndex);

      if (dMid == 0.0)
         break;
      if (((dMid < 0.0) && (dLo < 0.0)) || ((dMid > 0.0) && (dLo > 0.0)))
      {
         loDelcon = delcon; dLo = dMid;
      }
      else
      {
         hiDelcon = delcon; dHi = dMid;
      }
      /* regula falsi when well-conditioned (fast convergence on the
      ** smooth side of a cusp), falling back to plain bisection when the
      ** two endpoints are too close in value to interpolate safely. */
      if (dHi != dLo)
         delcon = loDelcon + (0.0 - dLo) * (hiDelcon - loDelcon) / (dHi - dLo);
      if ((delcon <= loDelcon) || (delcon >= hiDelcon) || (dHi == dLo))
         delcon = 0.5 * (loDelcon + hiDelcon);
   }

   crossing[0] = MidVal (PointsA[0], PointsB[0], delcon);
   crossing[1] = MidVal (PointsA[1], PointsB[1], delcon);
   crossing[2] = MidVal (PointsA[2], PointsB[2], delcon);

   return (TRUE);
}
