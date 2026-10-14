/* ============================================================================
 * [Qt port ADDITION] Export of the current Noddy model as Tomofast-x input
 * files (Tools > Export Block > Tomofast-x Inversion Files...).
 *
 * Written into a directory chosen (or created) in the dialog:
 *   model_grid.txt            Tomofast-x model grid (core + horizontal padding)
 *   model_density.txt         density model  (kg/m3), one value per grid cell
 *   model_susceptibility.txt  susceptibility model (SI), one value per grid cell
 *   data_grav.csv             observation points (value column = 0)
 *   data_magn.csv             observation points (value column = 0)
 *   paramfile_grav.txt        gravity inversion
 *   paramfile_magn.txt        magnetic inversion
 *   paramfile_joint.txt       joint (cross-gradient) inversion
 *
 * The parameter files set useSyntheticModelForDataValues = 1 so Tomofast-x
 * forward-calculates the "observed" data from the exported density and
 * susceptibility models before inverting them.
 *
 * Mesh / conventions (mirrors Data2Tomofast.py of the Tomofast-x QGIS plugin):
 *  - core cells = the Noddy block at the geophysics cube size; horizontal
 *    padding cells grow by 1.15x per cell out to a distance equal to the model
 *    depth, and carry zero density/susceptibility. No vertical padding.
 *  - z is elevation (global.zAxisDirection = -1). Without topography the top
 *    of the mesh is the top of the Noddy block; with topography each column of
 *    cells hangs from the topography surface.
 *  - X/Y include the project's false easting/northing.
 *  - observation points are at the core cell centres, at the survey height
 *    (above topography when draped, above the block top otherwise).
 *  - Noddy remanence and anisotropy cannot be represented by Tomofast-x's
 *    scalar model and are ignored (the isotropic X susceptibility is used).
 * ============================================================================
 */
#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern int TopoRow, TopoCol;
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE;
extern double **topographyMap;

#define TOMOFAST_PAD_GROWTH      1.15
#define TOMOFAST_MIN_SENSOR_HEIGHT 0.1  /* keep sensors off the top cell face */
#define TOMOFAST_GRAV_UNITS_MULT 1.0E-5 /* mGal -> m/s2 */
#define TOMOFAST_MAGN_UNITS_MULT 1.0E-9 /* nT -> T */

/* Tomofast-x wants forward slashes in paths on every platform. */
static void toSlashes (char *s)
{
   for (; *s; s++)
      if (*s == '\\')
         *s = '/';
}

/* Number of growing padding cells needed to cover paddingSize; sizes returned
** in pad[] (smallest first). */
static int generatePadding (double cellSize, double paddingSize, double *pad, int maxPad)
{
   int n = 0;
   double cur = cellSize, total = 0.0;

   while (total < paddingSize && n < maxPad)
   {
      cur *= TOMOFAST_PAD_GROWTH;
      total += cur;
      pad[n++] = cur;
   }
   return n;
}

/* Surface elevation at a Noddy x,y; falls back to the block top. The location
** is clamped to just inside the topography grid (see filterBlockDataAboveTopo
** in nodLib2.c for why the margin is needed). */
static double surfaceElevation (int useTopo, double x, double y, double blockTop)
{
   double minX, maxX, minY, maxY, cellX, cellY, z;
   int error = FALSE;

   if (!useTopo)
      return blockTop;

   minX = MIN (TopomapXW, TopomapXE);  maxX = MAX (TopomapXW, TopomapXE);
   minY = MIN (TopomapYW, TopomapYE);  maxY = MAX (TopomapYW, TopomapYE);
   cellX = (maxX - minX) / TopoCol;
   cellY = (maxY - minY) / TopoRow;
   minX += 0.501*cellX;  maxX -= 0.501*cellX;
   minY += 0.501*cellY;  maxY -= 0.501*cellY;
   if (x < minX) x = minX;  else if (x > maxX) x = maxX;
   if (y < minY) y = minY;  else if (y > maxY) y = maxY;

   z = getTopoValueAtLoc (topographyMap, TopoCol, TopoRow,
                          TopomapYW, TopomapYE, TopomapXW, TopomapXE,
                          x, y, &error);
   return error ? blockTop : z;
}

static void section (FILE *f, const char *title)
{
   fprintf (f, "\n===================================================================================\n");
   fprintf (f, "%s\n", title);
   fprintf (f, "===================================================================================\n");
}

/* Sensitivity matrix wavelet (Haar) compression, with the rate formula of the
** Tomofast-x QGIS plugin (update_ideal_compression_ratio in Tomofast_x.py,
** from Bruce et al. 2025), fitted to give ~1% compression error. Compression
** is used when the sensitivity matrix (nData x mesh cells, 8 bytes per
** element) would exceed 1 GB; n is the full mesh size including padding.
** Returns the compression type (0 none, 1 Haar) and sets *rate. */
#define TOMOFAST_COMPRESSION_MIN_BYTES (1024.0*1024.0*1024.0)
#define TOMOFAST_COMPRESSION_ERROR     0.01
static int chooseCompression (int nData, int nx, int ny, int nz, double *rate)
{
   double n = (double) nx * (double) ny * (double) nz;
   double ideal;

   if (8.0 * n * (double) nData <= TOMOFAST_COMPRESSION_MIN_BYTES)
   {
      *rate = 1.0;
      return 0;
   }
   ideal = 35.07 * pow (TOMOFAST_COMPRESSION_ERROR, -0.872) * pow (n, -0.884);
   *rate = 2.0 * ideal;
   if (*rate > 1.0)
      *rate = 1.0;
   return 1;
}

/* Writes one parameter file. mode: 1 = gravity, 2 = magnetics, 3 = joint. */
static int writeParamFile (const char *dir, const char *fileName, const char *tag,
                           int mode, int nx, int ny, int nz, int nData,
                           double fieldInc, double fieldDec, double fieldInt,
                           int comprType, double comprRate, double refDensity)
{
   char path[1200];
   FILE *f;
   int doGrav = (mode == 1 || mode == 3);
   int doMagn = (mode == 2 || mode == 3);

   sprintf (path, "%s/%s", dir, fileName);
   if (!(f = fopen (path, "w")))
      return FALSE;

   section (f, "GLOBAL");
   fprintf (f, "global.outputFolderPath             = %s/OUTPUT_%s\n", dir, tag);
   fprintf (f, "global.description                  = Noddy model, %s inversion\n", tag);
   /* read by the Tomofast-x QGIS plugin (same comment line it writes itself) */
   fprintf (f, "#global.experimentType               = %d\n", mode);
   if (doGrav)
   {
      fprintf (f, "global.grav.dataUnitsMultiplier     = %g\n", TOMOFAST_GRAV_UNITS_MULT);
      fprintf (f, "global.grav.modelUnitsMultiplier    = 1.0\n");
   }
   if (doMagn)
   {
      fprintf (f, "global.magn.dataUnitsMultiplier     = %g\n", TOMOFAST_MAGN_UNITS_MULT);
      fprintf (f, "global.magn.modelUnitsMultiplier    = 1.0\n");
   }
   fprintf (f, "global.zAxisDirection               = -1\n");

   section (f, "MODEL GRID parameters");
   fprintf (f, "modelGrid.size                      = %d %d %d\n", nx, ny, nz);
   if (doGrav)
      fprintf (f, "modelGrid.grav.file                 = %s/model_grid.txt\n", dir);
   if (doMagn)
      fprintf (f, "modelGrid.magn.file                 = %s/model_grid.txt\n", dir);

   if (doMagn)
   {
      section (f, "MAGNETIC FIELD constants");
      fprintf (f, "forward.magneticField.inclination          = %g\n", fieldInc);
      fprintf (f, "forward.magneticField.declination          = %g\n", fieldDec);
      fprintf (f, "forward.magneticField.intensity_nT         = %g\n", fieldInt);
      fprintf (f, "forward.magneticField.XaxisDeclination     = 0.0\n");
   }

   section (f, "DATA parameters");
   if (doGrav)
   {
      fprintf (f, "# Density model = Noddy density minus %g kg/m3 (mean density of the block).\n", refDensity);
      fprintf (f, "forward.data.grav.nData             = %d\n", nData);
      fprintf (f, "forward.data.grav.dataGridFile      = %s/data_grav.csv\n", dir);
      fprintf (f, "forward.data.grav.useSyntheticModelForDataValues = 1\n");
      fprintf (f, "forward.data.grav.syntheticModelFile = %s/model_density.txt\n", dir);
   }
   if (doMagn)
   {
      fprintf (f, "forward.data.magn.nData             = %d\n", nData);
      fprintf (f, "forward.data.magn.dataGridFile      = %s/data_magn.csv\n", dir);
      fprintf (f, "forward.data.magn.useSyntheticModelForDataValues = 1\n");
      fprintf (f, "forward.data.magn.syntheticModelFile = %s/model_susceptibility.txt\n", dir);
   }

   section (f, "DEPTH WEIGHTING");
   fprintf (f, "forward.depthWeighting.type         = 2\n");
   if (doGrav)
      fprintf (f, "forward.depthWeighting.grav.power   = 2.0\n");
   if (doMagn)
      fprintf (f, "forward.depthWeighting.magn.power   = 3.0\n");

   section (f, "SENSITIVITY KERNEL");
   fprintf (f, "sensit.readFromFiles                = 0\n");
   fprintf (f, "sensit.folderPath                   = %s/OUTPUT_%s/SENSIT/\n", dir, tag);

   section (f, "MATRIX COMPRESSION");
   fprintf (f, "# 0-none, 1-Haar, 2-Daubechies D4.\n");
   fprintf (f, "forward.matrixCompression.type           = %d\n", comprType);
   fprintf (f, "forward.matrixCompression.rate           = %g\n", comprRate);

   section (f, "INVERSION parameters");
   fprintf (f, "inversion.nMajorIterations          = 3\n");
   fprintf (f, "inversion.nMinorIterations          = 100\n");
   fprintf (f, "inversion.writeModelEveryNiter      = 0\n");

   section (f, "MODEL DAMPING (m - m_prior)");
   if (doGrav)
      fprintf (f, "inversion.modelDamping.grav.weight  = 1.d-11\n");
   if (doMagn)
      fprintf (f, "inversion.modelDamping.magn.weight  = 1.d-8\n");
   fprintf (f, "inversion.modelDamping.normPower    = 2.d0\n");

   section (f, "JOINT INVERSION parameters");
   fprintf (f, "inversion.joint.grav.problemWeight  = %d\n", doGrav ? 1 : 0);
   fprintf (f, "inversion.joint.magn.problemWeight  = %d\n", doMagn ? 1 : 0);

   if (mode == 3)
   {
      section (f, "CROSS-GRADIENT constraints");
      fprintf (f, "inversion.crossGradient.weight        = 0.8e2\n");
      fprintf (f, "inversion.crossGradient.derivativeType = 1\n");
   }

   fclose (f);
   return TRUE;
}

/* ======================================================================
FUNCTION        exportTomofastFiles
DESCRIPTION
     asks for a folder name and writes the Tomofast-x input files for
     the current model into it
RETURNED
     TRUE on success
====================================================================== */
int
exportTomofastFiles (void)
{
   BLOCK_VIEW_OPTIONS *bv = (BLOCK_VIEW_OPTIONS *) getViewOptions ();
   FILE_SPEC fs;
   char dir[1100], path[1200];
   float ***rho = NULL, ***sus = NULL;
   double c, depth, originX, originY, originZ, bottomZ;
   double eastOff, northOff, surveyHeight, blockTop;
   double *pad = NULL, *dx = NULL, *dy = NULL, *coreTop = NULL, *colTop = NULL;
   double *xEdge = NULL, *yEdge = NULL;
   double maxTop, sensorZ, x, y, zc, zTop, zBot, v;
   int nx, ny, nz, nPad = 0, nxm, nym, nzm, i, j, k, ip, jp, cx, cy, idx;
   int useTopo, draped, ok = FALSE, maxPad = 1000, nData, comprType;
   double comprRate, refDensity = 0.0;
   FILE *fGrid = NULL, *fRho = NULL, *fSus = NULL, *fDg = NULL, *fDm = NULL;
   long nElements;

   if (!bv)
      return FALSE;

   c = bv->geophysicsCubeSize;
   nx = (int) (bv->lengthX / c);
   ny = (int) (bv->lengthY / c);
   nz = (int) (bv->lengthZ / c);
   if (c <= 0.0 || nx < 1 || ny < 1 || nz < 1)
   {
      xvt_dm_post_error ("Error, the block model has no cells to export.");
      return FALSE;
   }
   originX = bv->originX;  originY = bv->originY;  originZ = bv->originZ;
   depth = nz * c;
   bottomZ = originZ - depth;
   blockTop = originZ;
   eastOff = projectOptions.easting;
   northOff = projectOptions.northing;
   draped = geophysicsOptions.drapedSurvey;
   useTopo = (geologyOptions.useTopography || draped) && topographyMap
             && TopoRow > 0 && TopoCol > 0;
   /* topography shapes the mesh only when the model itself uses it */
   if (!geologyOptions.useTopography)
      useTopo = FALSE;
   surveyHeight = (geophysicsOptions.calculationAltitude == AIRBORNE)
                  ? geophysicsOptions.altitude : 0.0;

                                       /* ************************* */
                                       /* Choose (or create) the output directory */
   memset (&fs, 0, sizeof (fs));
   getDefaultDirectory (&(fs.dir));
   switch (xvt_dm_post_dir_select (&(fs.dir), "Export Tomofast-x files to ..."))
   {
      case FL_OK:
         break;
      case FL_BAD:
         xvt_dm_post_error ("Error getting directory name.");
         return FALSE;
      default:
         return FALSE;
   }
   strcpy (dir, fs.dir.path);
   toSlashes (dir);
   {
      FILE *test;
      sprintf (path, "%s/.tomofast_test", dir);
      if (!(test = fopen (path, "w")))
      {
         xvt_dm_post_error ("Error, could not write to directory %s", dir);
         return FALSE;
      }
      fclose (test);
      remove (path);
   }

                                       /* ************************* */
                                       /* Property volumes [z][x][y] */
   if (!(rho = calcBlockPropertiesData (DENSITY_PROP, originX, originY, originZ, c, nx, ny, nz)))
   {
      xvt_dm_post_error ("Error, not enough memory (or no model) to calculate density.");
      return FALSE;
   }
   if (!(sus = calcBlockPropertiesData (SUS_X_PROP, originX, originY, originZ, c, nx, ny, nz)))
   {
      destroy3DArray ((char ***) rho, nz, nx, ny);
      xvt_dm_post_error ("Error, not enough memory (or no model) to calculate susceptibility.");
      return FALSE;
   }

                                       /* ************************* */
                                       /* Mesh                      */
   if (!(pad = (double *) malloc (sizeof (double) * maxPad)))
   {
      xvt_dm_post_error ("Error, not enough memory to build the Tomofast-x mesh.");
      goto cleanup;
   }
   nPad = generatePadding (c, depth, pad, maxPad);
   nxm = nx + 2*nPad;
   nym = ny + 2*nPad;

   dx = (double *) malloc (sizeof (double) * nxm);
   dy = (double *) malloc (sizeof (double) * nym);
   xEdge = (double *) malloc (sizeof (double) * (nxm + 1));
   yEdge = (double *) malloc (sizeof (double) * (nym + 1));
   coreTop = (double *) malloc (sizeof (double) * nx * ny);
   colTop = (double *) malloc (sizeof (double) * nxm * nym);
   if (!dx || !dy || !xEdge || !yEdge || !coreTop || !colTop)
   {
      xvt_dm_post_error ("Error, not enough memory to build the Tomofast-x mesh.");
      goto cleanup;
   }

   for (i = 0; i < nxm; i++)
   {
      if (i < nPad)             dx[i] = pad[nPad - 1 - i];
      else if (i < nPad + nx)   dx[i] = c;
      else                      dx[i] = pad[i - nPad - nx];
   }
   for (j = 0; j < nym; j++)
   {
      if (j < nPad)             dy[j] = pad[nPad - 1 - j];
      else if (j < nPad + ny)   dy[j] = c;
      else                      dy[j] = pad[j - nPad - ny];
   }
   xEdge[0] = originX;
   for (i = 0; i < nPad; i++)
      xEdge[0] -= pad[i];
   yEdge[0] = originY;
   for (j = 0; j < nPad; j++)
      yEdge[0] -= pad[j];
   for (i = 0; i < nxm; i++)  xEdge[i+1] = xEdge[i] + dx[i];
   for (j = 0; j < nym; j++)  yEdge[j+1] = yEdge[j] + dy[j];

                                       /* surface under every core column;
                                       ** padding columns copy the nearest one */
   maxTop = blockTop;
   for (j = 0; j < ny; j++)
      for (i = 0; i < nx; i++)
      {
         coreTop[j*nx + i] = surfaceElevation (useTopo, originX + (i + 0.5)*c,
                                               originY + (j + 0.5)*c, blockTop);
         if (i == 0 && j == 0)
            maxTop = coreTop[0];
         else if (coreTop[j*nx + i] > maxTop)
            maxTop = coreTop[j*nx + i];
      }
   for (jp = 0; jp < nym; jp++)
      for (ip = 0; ip < nxm; ip++)
      {
         cx = ip - nPad;  if (cx < 0) cx = 0;  else if (cx > nx - 1) cx = nx - 1;
         cy = jp - nPad;  if (cy < 0) cy = 0;  else if (cy > ny - 1) cy = ny - 1;
         colTop[jp*nxm + ip] = coreTop[cy*nx + cx];
      }

                                       /* cells hang from the surface; enough
                                       ** layers to reach the bottom of the
                                       ** block under the highest column */
   nzm = (int) ceil ((maxTop - bottomZ) / c - 1.0e-6);
   if (nzm < nz)
      nzm = nz;
   nElements = (long) nxm * nym * nzm;

                                       /* ************************* */
                                       /* Reference density          */
   /* Gravity responds to density CONTRAST. Exporting absolute densities
   ** against zero-density padding would make Tomofast-x see the whole block
   ** as a slab of excess mass (a smooth dome with edge roll-off swamping the
   ** geology), so the density model is written relative to the mean density
   ** of the cells inside the Noddy block; padding is then simply background. */
   {
      double sumRho = 0.0;
      long countRho = 0;

      for (k = 0; k < nzm; k++)
         for (j = 0; j < ny; j++)
            for (i = 0; i < nx; i++)
            {
               zc = colTop[(j + nPad)*nxm + (i + nPad)] - (k + 0.5)*c;
               idx = (int) floor ((originZ - zc) / c);
               if (idx >= 0 && idx < nz)
               {
                  sumRho += (double) rho[nz-1-idx][i][ny-1-j];
                  countRho++;
               }
            }
      refDensity = countRho ? sumRho / (double) countRho : 0.0;
   }

                                       /* ************************* */
                                       /* Grid and model files      */
   sprintf (path, "%s/model_grid.txt", dir);
   fGrid = fopen (path, "w");
   sprintf (path, "%s/model_density.txt", dir);
   fRho = fopen (path, "w");
   sprintf (path, "%s/model_susceptibility.txt", dir);
   fSus = fopen (path, "w");
   if (!fGrid || !fRho || !fSus)
   {
      xvt_dm_post_error ("Error, could not create files in %s", dir);
      goto cleanup;
   }
   fprintf (fGrid, "%ld\n", nElements);
   fprintf (fRho, "%ld\n", nElements);
   fprintf (fSus, "%ld\n", nElements);

   for (k = 0; k < nzm; k++)
      for (jp = 0; jp < nym; jp++)
         for (ip = 0; ip < nxm; ip++)
         {
            zTop = colTop[jp*nxm + ip] - k*c;
            zBot = zTop - c;
            fprintf (fGrid, "%f %f %f %f %f %f %d %d %d\n",
                     xEdge[ip] + eastOff,  xEdge[ip+1] + eastOff,
                     yEdge[jp] + northOff, yEdge[jp+1] + northOff,
                     zBot, zTop, ip + 1, jp + 1, k + 1);

            cx = ip - nPad;
            cy = jp - nPad;
            v = 0.0;
            if (cx >= 0 && cx < nx && cy >= 0 && cy < ny)
            {
               zc = 0.5*(zTop + zBot);
               idx = (int) floor ((originZ - zc) / c);   /* layers down from the block top */
               if (idx >= 0 && idx < nz && zc <= colTop[jp*nxm + ip])
               {
                  /* Noddy's block arrays are [z][x][y] with z index nz-1 at
                  ** the top, x index 0 at the WEST edge and y index 0 at the
                  ** NORTH edge (calcBlockLayer in block.c: y location is
                  ** (ny-y-1)*cube + origin), while cy here counts from the
                  ** south edge. */
                  fprintf (fRho, "%.6g\n", (double) rho[nz-1-idx][cx][ny-1-cy] - refDensity);
                  fprintf (fSus, "%.8g\n", (double) sus[nz-1-idx][cx][ny-1-cy] * CGS_TO_SI);
                  continue;
               }
            }
            fprintf (fRho, "%.6g\n", v);
            fprintf (fSus, "%.8g\n", v);
         }
   fclose (fGrid);  fGrid = NULL;
   fclose (fRho);   fRho = NULL;
   fclose (fSus);   fSus = NULL;

                                       /* ************************* */
                                       /* Observation points        */
   nData = nx * ny;
   sprintf (path, "%s/data_grav.csv", dir);
   fDg = fopen (path, "w");
   sprintf (path, "%s/data_magn.csv", dir);
   fDm = fopen (path, "w");
   if (!fDg || !fDm)
   {
      xvt_dm_post_error ("Error, could not create data files in %s", dir);
      goto cleanup;
   }
   fprintf (fDg, "%d\n", nData);
   fprintf (fDm, "%d\n", nData);
   for (j = 0; j < ny; j++)
      for (i = 0; i < nx; i++)
      {
         x = originX + (i + 0.5)*c;
         y = originY + (j + 0.5)*c;
         if (draped)
            sensorZ = surfaceElevation (TRUE && topographyMap != NULL && TopoRow > 0,
                                        x, y, blockTop) + surveyHeight;
         else
            sensorZ = blockTop + surveyHeight;
         /* a sensor exactly on the top face of the cells below it is
         ** numerically awkward for Tomofast-x, so nudge it up slightly */
         if (fabs (sensorZ - coreTop[j*nx + i]) < TOMOFAST_MIN_SENSOR_HEIGHT)
            sensorZ = coreTop[j*nx + i] + TOMOFAST_MIN_SENSOR_HEIGHT;
         fprintf (fDg, "%f %f %f 0.0\n", x + eastOff, y + northOff, sensorZ);
         fprintf (fDm, "%f %f %f 0.0\n", x + eastOff, y + northOff, sensorZ);
      }
   fclose (fDg);  fDg = NULL;
   fclose (fDm);  fDm = NULL;

                                       /* ************************* */
                                       /* Parameter files           */
   comprType = chooseCompression (nData, nxm, nym, nzm, &comprRate);
   if (!writeParamFile (dir, "paramfile_grav.txt", "grav", 1, nxm, nym, nzm, nData,
                        geophysicsOptions.inclination, geophysicsOptions.declination,
                        geophysicsOptions.intensity, comprType, comprRate, refDensity)
    || !writeParamFile (dir, "paramfile_magn.txt", "magn", 2, nxm, nym, nzm, nData,
                        geophysicsOptions.inclination, geophysicsOptions.declination,
                        geophysicsOptions.intensity, comprType, comprRate, refDensity)
    || !writeParamFile (dir, "paramfile_joint.txt", "joint", 3, nxm, nym, nzm, nData,
                        geophysicsOptions.inclination, geophysicsOptions.declination,
                        geophysicsOptions.intensity, comprType, comprRate, refDensity))
   {
      xvt_dm_post_error ("Error, could not write the parameter files in %s", dir);
      goto cleanup;
   }

   ok = TRUE;
   if (comprType)
      xvt_dm_post_note ("Tomofast-x files written to %s\n\nMesh %d x %d x %d (core %d x %d, %d padding cells each side), %d data points.\nDensity is relative to the mean %.0f kg/m3.\nHaar matrix compression rate %.4g.",
                        dir, nxm, nym, nzm, nx, ny, nPad, nData, refDensity, comprRate);
   else
      xvt_dm_post_note ("Tomofast-x files written to %s\n\nMesh %d x %d x %d (core %d x %d, %d padding cells each side), %d data points.\nDensity is relative to the mean %.0f kg/m3.\nModel is small, so no matrix compression.",
                        dir, nxm, nym, nzm, nx, ny, nPad, nData, refDensity);

cleanup:
   if (fGrid) fclose (fGrid);
   if (fRho)  fclose (fRho);
   if (fSus)  fclose (fSus);
   if (fDg)   fclose (fDg);
   if (fDm)   fclose (fDm);
   free (pad);  free (dx);  free (dy);  free (xEdge);  free (yEdge);
   free (coreTop);  free (colTop);
   destroy3DArray ((char ***) rho, nz, nx, ny);
   destroy3DArray ((char ***) sus, nz, nx, ny);
   return ok;
}
