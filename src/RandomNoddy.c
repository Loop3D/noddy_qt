/* ============================================================================
 * [Qt port ADDITION] todo.txt #41. Ported from a sibling Noddy source tree
 * (/home/mark/noddyverse-Tomofast-x_noddy/src/RandomNoddy.c), the random
 * geological history generator originally written for the "Noddyverse"
 * ML-training-dataset project. That tree is an independently-evolved
 * snapshot of the same codebase; porting it here required a few adaptations,
 * each flagged below where it happens:
 *
 *  - the reference file's ~700 lines of custom report_Random_* file-writing
 *    functions were dropped entirely. loadRandomHistory() below now builds
 *    its event list via get_win_info(win) -- the same accessor every other
 *    part of the app already uses -- instead of a hardcoded pointer straight
 *    at batchWindowInfo, so the app's own real report_status()/report_hist()
 *    etc. (reporter.c/putEvent.c) can write the generated history straight
 *    out, correctly, with no duplicate/parallel code needed. This also
 *    dropped a handful of reference functions that were already dead code
 *    there (never called): ReportRandomIcons, load_Random_icon_positions,
 *    loadRandomProfile, loadRandomGeneric, nthRandomObject,
 *    totalRandomObjects, and the whole report_Random_* family.
 *  - loadRandomHistory() also gained the same "make the new events visible"
 *    tail call sequence getHist.c's loadHistory() uses (reorderObjects +
 *    xvt_dwin_invalidate_rect + updateFloatingMenu), so this same function
 *    now works correctly from BOTH the interactive "Random History" File
 *    menu item and the batch-only -random command-line flag -- the
 *    reference file only ever supported the batch case.
 *  - xrshr128p_state_t *state -> a plain (non-pointer) state struct: every
 *    call site already passes &state, which only has the right type
 *    (xrshr128p_state_t*) if state itself isn't a pointer. As written, the
 *    reference code writes the 16-byte PRNG state through a 2-level
 *    pointer, corrupting 8 bytes of whatever memory happens to follow the
 *    `state` pointer variable.
 *  - spectralPaddingType = REFLECTION_PADDING -> RECLECTION_PADDING,
 *    matching this tree's nodStruc.h, which has always spelled that enum
 *    member with the letters transposed.
 *  - loadRandomProperties()'s unitName sprintf is now width-bounded
 *    (UNIT_NAME_LENGTH is 20 in this tree's nodStruc.h vs. 50 in the
 *    reference tree's -- unbounded, some lithology names would overflow it).
 *  - dropped the `historyWindow = win;` line and its `extern WINDOW
 *    historyWindow;` declaration: historyWindow is `static` inside
 *    builder.c in this tree (internal linkage only), so that assignment
 *    doesn't link here. It was a no-op in the reference build anyway --
 *    get_win_info() resolves to &batchWindowInfo whenever batchExecution is
 *    set, regardless of what historyWindow holds.
 *
 * No event-generation math/logic was changed from the reference file.
 * ============================================================================
 */
#include "stdio.h"
#include "stdlib.h"
#include "noddy.h"
#include "xvt.h"
#include "nodStruc.h"
#include "titles.h"
#include "time.h"
#include "string.h"
#include <sys/time.h>
#include <stdint.h>
/* [Qt port fix] sqrt/log/pow/sin/cos (loadRandomFault's dip, petrophysics's
** Box-Muller transform) return double -- without a prototype in scope,
** these fall back to an implicit int-returning declaration, which is
** undefined behaviour for a real double-returning libm call on x86-64
** (mismatched calling convention, not just a style warning). Neither
** noddy.h nor any of this file's other includes pull math.h in. */
#include <math.h>
#include "xoroshiro128plus.h"
#include "petrophysics.h"
#include "petrophy_defs.h"

#define DEBUG(X)
#define DEBUG1(X)

static int loadRandomHistory(int);

/* ********************************** */
/* External Globals used in this file */
extern PROJECT_OPTIONS projectOptions;
extern BLOCK_VIEW_OPTIONS *blockViewOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern double iscale; /* scaling factor for geology */
extern FILE_SPEC topoFileSpec;
extern double minTopoValue, maxTopoValue;

extern int batchExecution;
extern COLOR backgroundColor;
extern WINDOW_INFO batchWindowInfo;
void copyUndoAdditions(OBJECT*, int);

#define VERSION_TOLERANCE 0.01
typedef unsigned char String20[21];

xrshr128p_state_t state; /* random number generator seed updated after each call */
int unitNumber = 1; /* unique lithology code */
int Random = TRUE; /* Generate a random Noddy model */
char time_stamp[100]; /* microsecond timestamp */
extern int rocktypes[5]; /* lithology classes for petrophysics */

/* ======================================================================
 FUNCTION  RandomNoddy

 DESCRIPTION
 Batch-only entry point (-random command line flag, batchNod.c): generate
 a random history, write it to a timestamped .his file, and calculate a
 full block export plus gravity/magnetic anomalies from it. `output` is
 the shared base filename (extensions -- .his, .g00.., .mag, .grv -- are
 appended by the callees). `DataBase` is accepted for interface parity
 with the reference file but unused: that file's database-upload path
 (Postgres) was already commented out there and isn't part of this build.
 ====================================================================== */
int RandomNoddy(char *output, int DataBase) {
	char hisFileName[300];
	BLOCK_VIEW_OPTIONS *viewOptions;
	struct timeval start;

	(void) DataBase;

	gettimeofday(&start, NULL);
	sprintf(time_stamp, "%ld-%ld", (long) start.tv_sec, (long) start.tv_usec);

	initVariables();
	batchExecution = TRUE;

	readRandomHist(TRUE);   /* batch: also apply block/geo/gps/3d/proj/window-position options, see readRandomHist's own comment */

	sprintf(hisFileName, "%s.his", output);
	report_status(hisFileName);

	viewOptions = getViewOptions();
	doGeophysics(BLOCK_AND_ANOM, viewOptions, &geophysicsOptions, output,
			output, NULL, 0, NULL, NULL, NULL);

	return (TRUE);
}

/* [Qt port change] todo.txt #59 -- applyGlobalOptions gates everything
** below loadRandomHistory() itself: block/geology/geophysics/3D/project
** option structs and ALL 16 window positions (the latter also clobbering
** their names to generic "Window_02%d" strings, wiping the ones the app
** actually looks up e.g. via HISTORY_TITLE). The reference file always
** applied these unconditionally, which made sense for its original
** batch-only, throwaway-process use (RandomNoddy(), -random flag below,
** still passes TRUE) but not for the interactive "Random History" File
** menu item (mainMenu.c passes FALSE) -- user-reported: generating a
** random EVENT HISTORY was silently also resetting window sizes, the
** geophysics image colour scheme to pseudocolour, and switching on
** deformable remanence, none of which have anything to do with the
** events themselves. */
int readRandomHist(int applyGlobalOptions) {
	/* [Qt port change] todo.txt #41 follow-up: restored to the pre-"vitaliy"
	** fixed event count (that fork's srand/rand()-based 3-5 event count is
	** reverted, per the user's explicit request to use the original
	** behavior). */
	int numEvents = 5; /* number of random events, including base STRATIGRAPHY and first TILT */

	loadRandomHistory(numEvents);

	if (applyGlobalOptions)
	{
		loadRandomBlockOpts();
		loadRandomGeoOpts();
		loadRandomGpsOpts();
		loadRandom3dOpts();
		loadRandomProjOpts();
		load_Random_window_positions();
	}
	return (TRUE);
}

static int loadRandomHistory(numEvents)
	int numEvents; {
	WINDOW win = (WINDOW) getEventDrawingWindow();
	WINDOW_INFO *wip;
	OBJECT *p, *head = NULL, *last;
	int numEventsInFile, event;
	int type, type2;
	int baseHeight = 0, baseWidth = 0;
	struct timeval start;
	/* [Qt port change] todo.txt #69 -- name each random event after its
	** own type (e.g. "Fold_01") instead of the shared "Event_NN" counter,
	** so the History window's icon labels distinguish events at a
	** glance. One counter per OBJECTS shape, incremented only for events
	** of that type. */
	int typeCounters[GENERIC + 1] = { 0 };
	static const char *typeNames[GENERIC + 1] = {
		"Strat", "Fold", "Fault", "Unconformity", "Shear", "Dyke",
		"Plug", "Strain", "Tilt", "Foliation", "Lineation", "Import",
		"Stop", "Generic"
	};

	gettimeofday(&start, NULL);

	xrshr128p_init((uint64_t) (start.tv_sec * 1000000 + start.tv_usec), &state);

	wip = (WINDOW_INFO*) get_win_info(win);

	numEventsInFile = numEvents;
	{
		for (event = 0; event < numEventsInFile; event++) {
			if (event == 0)
				type2 = STRATIGRAPHY;
			else if (event == 1)
				type2 = TILT;
			else {
				/* [Qt port change] todo.txt #41 follow-up: restored to the
				** pre-"vitaliy" random event-type selection (that fork
				** hardcoded every non-STRATIGRAPHY/TILT event to PLUG). */
				type = (int) (xrshr128p_next(&state) % 10) + 1;
				if (type == 1 || type == 2)
					type2 = FOLD;
				else if (type == 3 || type == 4)
					type2 = FAULT;
				else if (type == 5 || type == 6)
					type2 = SHEAR_ZONE;
				else if (type == 7)
					type2 = UNCONFORMITY;
				else if (type == 8)
					type2 = DYKE;
				else if (type == 9)
					type2 = PLUG;
				else
					type2 = TILT;
			}

			/* malloc the basic structure */
			if (!(p = (OBJECT*) xvt_mem_zalloc(sizeof(OBJECT)))) {
				fprintf(stderr, "Not Enough Memory to load Events");
				return (FALSE);
			}
			p->shape = type2;
			p->drawEvent = TRUE;
			p->selected = TRUE;
			p->newObject = FALSE;
			p->next = NULL;

			switch (p->shape) {
			case STRATIGRAPHY: {
				STRATIGRAPHY_OPTIONS *options;

				/* we must always read the stratigraphy even
				 ** if we already have one so we can get past it */
				options = (STRATIGRAPHY_OPTIONS*) xvt_mem_zalloc(
						sizeof(STRATIGRAPHY_OPTIONS));
				if (!options) {
					fprintf(stderr,
							"Not Enough Memory to Load Stratigraphy Event");
					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomStratigraphy(options);

				wip->head = p;
				break;
			}
			case FOLD: {
				FOLD_OPTIONS *options;

				options = (FOLD_OPTIONS*) xvt_mem_zalloc(sizeof(FOLD_OPTIONS));
				if (!options) {
					fprintf(stderr, "Not Enough Memory to Load Fold Event");
					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomFold(options);
				break;
			}
			case FAULT: {
				FAULT_OPTIONS *options;

				options = (FAULT_OPTIONS*) xvt_mem_zalloc(
						sizeof(FAULT_OPTIONS));
				if (!options) {
					fprintf(stderr, "Not Enough Memory to Load Fault Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomFault(options);
				break;
			}
			case UNCONFORMITY: {
				UNCONFORMITY_OPTIONS *options;

				options = (UNCONFORMITY_OPTIONS*) xvt_mem_zalloc(
						sizeof(UNCONFORMITY_OPTIONS));
				if (!options) {

					fprintf(stderr,
							"Not Enough Memory to Load Unconformity Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomUnconformity(options);
				break;
			}
			case SHEAR_ZONE: {
				SHEAR_OPTIONS *options;

				options = (SHEAR_OPTIONS*) xvt_mem_zalloc(
						sizeof(SHEAR_OPTIONS));
				if (!options) {

					fprintf(stderr,
							"Not Enough Memory to Load Shear Zone Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomShearZone(options);
				break;
			}
			case DYKE: {
				DYKE_OPTIONS *options;

				options = (DYKE_OPTIONS*) xvt_mem_zalloc(sizeof(DYKE_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Dyke Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomDyke(options);
				break;
			}
			case PLUG: {
				PLUG_OPTIONS *options;

				options = (PLUG_OPTIONS*) xvt_mem_zalloc(sizeof(PLUG_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Plug Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomPlug(options);
				break;
			}
			case TILT: {
				TILT_OPTIONS *options;

				options = (TILT_OPTIONS*) xvt_mem_zalloc(sizeof(TILT_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Tilt Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomTilt(options);
				break;
			}
			case STRAIN: {
				STRAIN_OPTIONS *options;

				options = (STRAIN_OPTIONS*) xvt_mem_zalloc(
						sizeof(STRAIN_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Strain Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomStrain(options);
				break;
			}
			case LINEATION: {
				LINEATION_OPTIONS *options;

				options = (LINEATION_OPTIONS*) xvt_mem_zalloc(
						sizeof(LINEATION_OPTIONS));
				if (!options) {

					fprintf(stderr,
							"Not Enough Memory to Load Lineation Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomLineation(options);
				break;
			}
			case FOLIATION: {
				FOLIATION_OPTIONS *options;

				options = (FOLIATION_OPTIONS*) xvt_mem_zalloc(
						sizeof(FOLIATION_OPTIONS));
				if (!options) {

					fprintf(stderr,
							"Not Enough Memory to Load Foliation Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomFoliation(options);
				break;
			}
			case IMPORT: {
				IMPORT_OPTIONS *options;

				options = (IMPORT_OPTIONS*) xvt_mem_zalloc(
						sizeof(IMPORT_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Import Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				break;
			}
			case GENERIC: {
				GENERIC_OPTIONS *options;

				options = (GENERIC_OPTIONS*) xvt_mem_zalloc(
						sizeof(GENERIC_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Generic Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				break;
			}
			case STOP: {
				p->options = NULL;
				break;
			}
			default:
				break;
			}
			if (p) {
				sprintf(p->text, "%s_%02d", typeNames[p->shape], ++typeCounters[p->shape]);
				p->row = 1;
				p->column = numEvents + event + 1;
				p->bound.top = baseHeight;
				p->bound.bottom = p->bound.top + ICON_SIZE + 2;
				p->bound.left = event * GRID_WIDTH + baseWidth;
				p->bound.right = p->bound.left + ICON_SIZE + 2;
				if (head) /* keep a list of all the events read in */
				{
					last->next = p;
					last = p;
				} else {
					head = p;
					last = p;
				}
				copyUndoAdditions(p, FALSE);

			}

		}
	}

	/* [Qt port ADDITION] make the newly-generated events visible: same tail
	** sequence getHist.c's loadHistory() uses after reading a history file
	** in, so this works whether we were called interactively (Random
	** History menu item) or from batch (-random). */
	reorderObjects(win);
	if (!batchExecution)
		xvt_dwin_invalidate_rect(win, NULL);
	updateFloatingMenu(getFloatingMenuWindow(), get_win_info(win)->head,
			get_win_info(win)->tool);

	return (TRUE);
}

int loadRandomBlockOpts()

{
	char vname[100];

	sprintf(vname, "View1");
	/* [Qt port change] todo.txt #41 follow-up: restored to the pre-"vitaliy"
	** block view dimensions/cube sizes. */
	newViewOptions(vname, 0, 0, 4000, 4000, 4000, 4000, 20, 20);

	return (TRUE);
}

int loadRandomGeoOpts() {
	iscale = 10;
	geologyOptions.welllogDeclination = 0;
	geologyOptions.welllogDepth = 400;
	geologyOptions.welllogAngle = 0;

	geologyOptions.boreholeX = 0;
	geologyOptions.boreholeY = 0;
	geologyOptions.boreholeZ = 4000;
	geologyOptions.boreholeDecl = 0;
	geologyOptions.boreholeDip = 0;
	geologyOptions.boreholeLength = 4000;

	geologyOptions.sectionX = 0;
	geologyOptions.sectionY = 0;
	geologyOptions.sectionZ = 4000;
	geologyOptions.sectionDecl = 90;
	geologyOptions.sectionLength = 4000;
	geologyOptions.sectionHeight = 4000;

	geologyOptions.useTopography = FALSE;

	geologyOptions.topoScaleFactor = 1;
	geologyOptions.topoOffsetFactor = 0;
	geologyOptions.topoLowContour = 0;
	geologyOptions.topoContourInterval = 0;

	geologyOptions.calculateChairDiagram = FALSE;

	geologyOptions.chair_X = 0;
	geologyOptions.chair_Y = 0;
	geologyOptions.chair_Z = 0;

	return (TRUE);
}

int loadRandomGpsOpts() {
	geophysicsOptions.calculationRange = 12000.0;

	geophysicsOptions.declination = 0.0;

	geophysicsOptions.inclination = 90.0;
	geophysicsOptions.intensity = 50000.0;

	geophysicsOptions.fieldType = FIXED_FIELD;

	geophysicsOptions.xPos = 0.0;
	geophysicsOptions.yPos = 0.0;
	geophysicsOptions.zPos = 5000.0;

	geophysicsOptions.inclinationOri = 0.0;
	geophysicsOptions.inclinationChange = 0.0;
	geophysicsOptions.intensityOri = 0.0;
	geophysicsOptions.intensityChange = 0.0;
	geophysicsOptions.declinationOri = 0.0;
	geophysicsOptions.declinationChange = 0.0;

	geophysicsOptions.altitude = 100.0;

	geophysicsOptions.calculationAltitude = AIRBORNE;

	geophysicsOptions.calculationMethod = SPECTRAL;

	/* [Qt port fix] todo.txt #41: reference spells this REFLECTION_PADDING;
	** this tree's nodStruc.h has always spelled the enum member
	** RECLECTION_PADDING (letters transposed) -- using the real spelling. */
	geophysicsOptions.spectralPaddingType = RECLECTION_PADDING;

	geophysicsOptions.spectralFence = 50;
	geophysicsOptions.spectralPercent = 100;

	geophysicsOptions.constantBoxDepth = 0.0;
	geophysicsOptions.cleverBoxRatio = 1.0;

	geophysicsOptions.deformableRemanence = TRUE;

	geophysicsOptions.deformableAnisotropy = FALSE;

	geophysicsOptions.magneticVectorComponents = FALSE;

	geophysicsOptions.projectVectorsOntoField = TRUE;

	geophysicsOptions.padWithRealGeology = TRUE;

	geophysicsOptions.drapedSurvey = FALSE;

	return (TRUE);
}

int loadRandom3dOpts()

{
	threedViewOptions.declination = 150.0;
	threedViewOptions.azimuth = 30.0;
	threedViewOptions.scale = 1.0;

	threedViewOptions.offset_X = 1.0;
	threedViewOptions.offset_Y = 1.0;
	threedViewOptions.offset_Z = 1.0;

	threedViewOptions.fillType = 3;
	return (TRUE);
}

int loadRandomProjOpts() {
	int red, green, blue;

	projectOptions.susceptibilityUnits = SI;

	projectOptions.geophysicalCalc = 2;

	projectOptions.calculationType = LOCAL_JOB;

	projectOptions.lengthScale = 0;

	projectOptions.printScalingFactor = 1.0;
	projectOptions.imageScalingFactor = 10.0;

	projectOptions.newWindowEachStage = FALSE;

	red = 255;
	green = 255;
	blue = 255;

	backgroundColor = XVT_MAKE_COLOR(red, green, blue);

	sprintf(projectOptions.internetAddress, "");
	sprintf(projectOptions.accountName, "");
	sprintf(projectOptions.noddyPath, "");

	sprintf(projectOptions.helpPath, "");

	projectOptions.movieFramesPerEvent = 3;
	projectOptions.moviePlaySpeed = 10.0;
	projectOptions.movieType = 0;

	projectOptions.gravityDisplay.clippingType = RELATIVE_CLIPPING;
	projectOptions.gravityDisplay.minClip = 0.0;
	projectOptions.gravityDisplay.maxClip = 100.0;
	projectOptions.gravityDisplay.display = PSEUDO_COLOR;
	projectOptions.gravityDisplay.numContours = 25;

	projectOptions.magneticsDisplay.clippingType == RELATIVE_CLIPPING;
	projectOptions.magneticsDisplay.minClip = 0.0;
	projectOptions.magneticsDisplay.maxClip = 100.0;
	projectOptions.magneticsDisplay.display = PSEUDO_COLOR;
	projectOptions.magneticsDisplay.numContours = 25;

	projectOptions.easting = 0.0;
	projectOptions.northing = 0.0;

	return (TRUE);
}

int load_Random_window_positions() {
	register int winNum;
	int numWindows;
	RCT position;
	WINDOW win;

	numWindows = 16;
	for (winNum = 0; winNum < numWindows; winNum++) {
		sprintf(winPositionOptions.winPositions[winNum].name, "Window_02%d",
				winNum);
		winPositionOptions.winPositions[winNum].x = 60;
		winPositionOptions.winPositions[winNum].y = 60;
		winPositionOptions.winPositions[winNum].width = 500;
		winPositionOptions.winPositions[winNum].height = 300;

	}

	if (!batchExecution) {
		if (getWindowStartPosition(HISTORY_TITLE, &position.left, &position.top,
				&position.right, &position.bottom, NULL, NULL)) {
			if (win = (WINDOW) getEventDrawingWindow())
				xvt_vobj_move (win, &position);
		}

	}

	return (TRUE);
}

int loadRandomFold(options)
	FOLD_OPTIONS *options; {

	if (xrshr128p_next(&state) % 2 > 0)
		options->type = SINE_FOLD;
	else
		options->type = FOURIER_FOLD;

	options->singleFold = FALSE;

	options->positionX = 4000.0 * xrshr128p_next_double(&state);
	options->positionY = 0.0;
	options->positionZ = 0.0;

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state);
	options->axisPitch = 90.0 * xrshr128p_next_double(&state);

	options->wavelength = 1000.0 + 10000.0 * xrshr128p_next_double(&state);
	options->amplitude = 200 + 4800.0 * xrshr128p_next_double(&state);
	options->cycle = 500 + 9500.0 * xrshr128p_next_double(&state);

	convrt(options->dip, options->dipDirection, options->axisPitch, TRUE);
	rotset(options->dip, options->dipDirection, options->axisPitch,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);

	return (TRUE);
}

int loadRandomDyke(options)
	DYKE_OPTIONS *options; {
	double dipDirection;

	options->type = DILATION_DYKE;

	options->mergeEvents = 0;

	options->positionX = 4000.0 * xrshr128p_next_double(&state);
	options->positionY = 4000.0 * xrshr128p_next_double(&state);
	options->positionZ = 4000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state);
	options->slipPitch = 90.0 * xrshr128p_next_double(&state);

	options->slipLength = 0;
	options->width = 100 + 300.0 * xrshr128p_next_double(&state);

	options->alterationZones = NONE_ALTERATION;

	loadRandomProperties(-2, &(options->properties));

	if (options->dip == 90.0)
		dipDirection = options->dipDirection + 180.0;
	else
		dipDirection = options->dipDirection;

	convrt(options->dip, dipDirection, 90.0, TRUE);
	rotset(options->dip, dipDirection, 90.0, options->rotationMatrix.forward,
			options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomFault(options)
	FAULT_OPTIONS *options; {
	double pitch, dip;

	options->type = FAULT_EVENT;

	options->geometry = TRANSLATION;

	options->movement = BOTH;

	options->positionX = 2000.0 + 2000.0 * xrshr128p_next_double(&state);
	options->positionY = 2000.0 + 2000.0 * xrshr128p_next_double(&state);
	options->positionZ = 2000.0 + 2000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * sqrt(xrshr128p_next_double(&state));
	options->pitch = 90.0 * xrshr128p_next_double(&state);

	options->slip = 2000.0 * xrshr128p_next_double(&state);
	options->rotation = 0.0;

	options->amplitude = 100.0;
	options->radius = 100.0;
	options->xAxis = 0.0;
	options->yAxis = 0.0;
	options->zAxis = 0.0;
	options->cylindricalIndex = 0.0;
	options->profilePitch = 0.0;

	loadRandomColor(&(options->color));

	if (options->geometry == ROTATION)
		pitch = 0.0;
	else
		pitch = options->pitch;

	if (options->dip == 90.0)
		dip = 89.999;
	else
		dip = options->dip;

	convrt(dip, options->dipDirection, pitch, TRUE);
	rotset(dip, options->dipDirection, pitch, options->rotationMatrix.forward,
			options->rotationMatrix.reverse);
	plane(&options->plane.a, &options->plane.b, &options->plane.c,
			&options->plane.d, options->positionX, options->positionY,
			options->positionZ);
	getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);
	return (TRUE);
}

int loadRandomShearZone(options)
	SHEAR_OPTIONS *options; {
	double dip, pitch;

	if (xrshr128p_next(&state) % 2 > 0)
		options->type = SHEAR_ZONE_EVENT;
	else
		options->type = KINK_EVENT;

	options->geometry = TRANSLATION;

	options->movement = BOTH;

	options->positionX = 2000.0 + 2000.0 * xrshr128p_next_double(&state);
	options->positionY = 2000.0 + 2000.0 * xrshr128p_next_double(&state);
	options->positionZ = 2000.0 + 2000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state);
	options->pitch = 90.0 * xrshr128p_next_double(&state);

	options->slip = 2000.0 * xrshr128p_next_double(&state);
	options->rotation = 0.0;

	options->amplitude = 100.0;
	options->width = 100.0 + 1900.0 * xrshr128p_next_double(&state);
	options->radius = 100.0;
	options->xAxis = 0.0;
	options->yAxis = 0.0;
	options->zAxis = 0.0;
	options->cylindricalIndex = 0.0;
	options->profilePitch = 0.0;

	loadRandomColor(&(options->color));

	if (options->geometry == ROTATION)
		pitch = 0.0;
	else
		pitch = options->pitch;

	if (options->dip == 90.0)
		dip = 89.999;
	else
		dip = options->dip;

	convrt(dip, options->dipDirection, pitch, TRUE);
	rotset(dip, options->dipDirection, pitch, options->rotationMatrix.forward,
			options->rotationMatrix.reverse);
	plane(&options->plane.a, &options->plane.b, &options->plane.c,
			&options->plane.d, options->positionX, options->positionY,
			options->positionZ);
	getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);
	return (TRUE);
}

int loadRandomLineation(options)
	LINEATION_OPTIONS *options; {
	options->plungeDirection = 0.0;
	options->plunge = 0.0;

	convrt(90.0 - options->plunge, 180.0 + options->plungeDirection, 0.0, TRUE);
	rotset(90.0 - options->plunge, 180.0 + options->plungeDirection, 0.0,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomFoliation(options)
	FOLIATION_OPTIONS *options; {
	options->dipDirection = 0.0;
	options->dip = 0.0;

	convrt(options->dip, options->dipDirection, 0.0, TRUE);
	rotset(options->dip, options->dipDirection, 0.0,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomPlug(options)
	PLUG_OPTIONS *options; {
	double pitch;
	int plugit;

	/* [Qt port change] todo.txt #41 follow-up: restored to the pre-"vitaliy"
	** random plug-type selection (that fork hardcoded every plug to
	** ELLIPSOIDAL_PLUG). */
	plugit = xrshr128p_next(&state) % 4;
	if (plugit == 0)
		options->type = CYLINDRICAL_PLUG;
	else if (plugit == 1)
		options->type = CONE_PLUG;
	else if (plugit == 2)
		options->type = PARABOLIC_PLUG;
	else
		options->type = ELLIPSOIDAL_PLUG;

	options->mergeEvents = 0;

	options->positionX = 1000.0 + 3000.0 * xrshr128p_next_double(&state);
	options->positionY = 1000.0 + 3000.0 * xrshr128p_next_double(&state);
	options->positionZ = 1000.0 + 3000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	/* [Qt port change] todo.txt #41 follow-up: restored to the pre-"vitaliy"
	** random dip (that fork hardcoded every plug's dip to 90.0). */
	options->dip = 90.0 * xrshr128p_next_double(&state);
	options->axisPitch = 90.0 * xrshr128p_next_double(&state);

	options->radius = 2000.0 * xrshr128p_next_double(&state);
	options->apicalAngle = 10.0 + 80.0 * xrshr128p_next_double(&state);
	options->BValue = 200 + 4800.0 * xrshr128p_next_double(&state);
	options->axisA = 200 + 800.0 * xrshr128p_next_double(&state);
	options->axisB = 200 + 800.0 * xrshr128p_next_double(&state);
	options->axisC = 200 + 800.0 * xrshr128p_next_double(&state);

	if (options->type != ELLIPSOIDAL_PLUG)
		pitch = 0.0;
	else
		pitch = options->axisPitch;

	/* [Qt port note] todo.txt #41 follow-up: NOT reverted like the other
	** "vitaliy" spots -- the reference file's commented-out alternative
	** here was `options->alterationZones = TRUE;`, which isn't a valid
	** value for this ALTERATION_ZONES-typed field (unlike loadRandomDyke's
	** identically-valued, untagged `NONE_ALTERATION` a little earlier in
	** this file), so it reads as an abandoned experiment rather than a
	** real prior state to restore. Left as NONE_ALTERATION. */
	options->alterationZones = NONE_ALTERATION;

	loadRandomProperties(-1, &(options->properties));

	convrt(options->dip - 90.0, options->dipDirection, pitch, TRUE);
	rotset(options->dip - 90.0, options->dipDirection, pitch,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomTilt(options)
	TILT_OPTIONS *options; {
	int i, j;
	double angle;
	double axis[2][4], mata[4][4];

	options->positionX = 2000.0;
	options->positionY = 2000.0;
	options->positionZ = 1000.0;

	options->rotation = -90+(90.0 * xrshr128p_next_double(&state));
	options->plungeDirection = 360.0 * xrshr128p_next_double(&state);
	options->plunge = 90.0 * xrshr128p_next_double(&state);

	angle = options->rotation * 0.0174532;
	paxis(options->plunge, options->plungeDirection, axis);
	irotate(mata, axis[1][1], axis[1][2], axis[1][3], angle);

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			options->rotationMatrix[i][j] = mata[i + 1][j + 1];
		}
	}
	return (TRUE);
}

int loadRandomStrain(options)
	STRAIN_OPTIONS *options; {
	int i, j;
	double tensor[4][4], inverseTensor[4][4];

	options->tensor[0][0] = 1.0;
	options->tensor[1][0] = 0.0;
	options->tensor[2][0] = 0.0;
	options->tensor[0][1] = 0.0;
	options->tensor[1][1] = 1.0;
	options->tensor[2][1] = 0.0;
	options->tensor[0][2] = 0.0;
	options->tensor[1][2] = 0.0;
	options->tensor[2][2] = 1.0;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			tensor[j + 1][i + 1] = options->tensor[i][j];
		}
	}
	if (!f01aaf(tensor, inverseTensor)) /* matrix inversion */
		return (FALSE);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			options->inverseTensor[i][j] = inverseTensor[i + 1][j + 1];
		}
	}
	return (TRUE);
}

int loadRandomUnconformity(options)
	UNCONFORMITY_OPTIONS *options; {
	options->positionX = 2000 + 1000.0 * xrshr128p_next_double(&state);
	options->positionY = 2000 + 2000.0 * xrshr128p_next_double(&state);
	options->positionZ = 3000 + 1000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state)
			* xrshr128p_next_double(&state);

	options->alterationZones = NONE_ALTERATION;

	options->surfaceType = FLAT_SURFACE;

	loadRandomStratigraphy(&(options->stratigraphyDetails));

	convrt(options->dip, options->dipDirection, 0.0, TRUE);
	plane(&options->plane.a, &options->plane.b, &options->plane.c,
			&options->plane.d, options->positionX, options->positionY,
			options->positionZ);
	rotset(options->dip, options->dipDirection, 0.0,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);

	return (TRUE);
}

int loadRandomStratigraphy(options)
	STRATIGRAPHY_OPTIONS *options; {
	int i;

	/* [Qt port change] todo.txt #41 follow-up: restored to the pre-"vitaliy"
	** random layer count (2-6; that fork hardcoded every stratigraphy to a
	** single layer). */
	options->numLayers = (xrshr128p_next(&state) % 5) + 2;
	if (options->properties)
		xvt_mem_free((char* ) options->properties);
	if (!(options->properties = (LAYER_PROPERTIES*) xvt_mem_zalloc(
			options->numLayers * sizeof(LAYER_PROPERTIES))))
		return (FALSE);

	for (i = 0; i < options->numLayers; i++) {
		loadRandomProperties(i, &(options->properties[i]));
	}

	return (TRUE);
}

int loadRandomColor(options)
	NODDY_COLOUR *options; {

	options->red = (unsigned char) xrshr128p_next(&state) % 256;
	options->green = (unsigned char) xrshr128p_next(&state) % 256;
	options->blue = (unsigned char) xrshr128p_next(&state) % 256;

	return (TRUE);
}

int petrophysics(int litho, double *density, double *magsus) {
	double U, V, X, Y;
	int lowmag;

	U = xrshr128p_next_double(&state);
	V = xrshr128p_next_double(&state);

	X = sqrt(-2.0 * log(U)) * cos(2.0 * 3.1415927 * V); /* where U & V are random numbers between 0 & 1 */
	Y = sqrt(-2.0 * log(U)) * sin(2.0 * 3.1415927 * V); /* produces pairs of independent normally
	                                                     ** distributed random numbers using the
	                                                     ** Box-Muller transform */

	*density = X * (PPHYS_ROCK[litho].density_sd) + PPHYS_ROCK[litho].density_mean;

	if (PPHYS_ROCK[litho].bimodal == 1) {
		lowmag = (int) xrshr128p_next(&state) % 2;

		if (lowmag == 0)
			*magsus = pow(10.0,
					(Y * PPHYS_ROCK[litho].magsus_sd / 2)
							+ (PPHYS_ROCK[litho].magsus_mean - 0.75));
		else
			*magsus = pow(10.0,
					(Y * PPHYS_ROCK[litho].magsus_sd / 2)
							+ (PPHYS_ROCK[litho].magsus_mean + 0.75));
	} else
		*magsus = pow(10.0,
				Y * (PPHYS_ROCK[litho].magsus_sd) + PPHYS_ROCK[litho].magsus_mean);

	return (TRUE);
}

int loadRandomProperties(layer, options)
	int layer;LAYER_PROPERTIES *options; {
	static int cum_height = 0;
	double density, magsus;
	static int stratcode;
	int lithocode;

	if (layer == -2) /* dyke */
	{
		lithocode = (xrshr128p_next(&state) % rocktypes[0]);
		petrophysics(lithocode, &density, &magsus);
	} else if (layer == -1) /* plug */
	{
		/* [Qt port change] todo.txt #41 follow-up: restored to the
		** pre-"vitaliy" random plug lithology (that fork hardcoded every
		** plug to granite). */
		lithocode = (xrshr128p_next(&state) % rocktypes[1]) + rocktypes[0];
		petrophysics(lithocode, &density, &magsus);
	} else /* strat */
	{
		/* [Qt port change] todo.txt #41 follow-up: restored to the
		** pre-"vitaliy" random Met/Volc/Sed stratigraphy class (that fork
		** hardcoded every stratigraphy to Sed). */
		if (layer == 0)
			stratcode = (xrshr128p_next(&state) % 3);
		if (stratcode == 0) /* Met */
		{
			lithocode = (xrshr128p_next(&state) % rocktypes[2]) + rocktypes[0]
					+ rocktypes[1];
			petrophysics(lithocode, &density, &magsus);
		} else if (stratcode == 1) /* Volc */
		{
			lithocode = (xrshr128p_next(&state) % rocktypes[3]) + rocktypes[0]
					+ rocktypes[1] + rocktypes[2];
			petrophysics(lithocode, &density, &magsus);

		} else /* Sed */
		{
			/* [Qt port change] todo.txt #41 follow-up: restored to the
			** pre-"vitaliy" random Sediment lithology (that fork hardcoded
			** every Sed layer to limestone). */
			lithocode = (xrshr128p_next(&state) % rocktypes[4]) + rocktypes[0]
					+ rocktypes[1] + rocktypes[2] + rocktypes[3];
			petrophysics(lithocode, &density, &magsus);

		}
	}

	/* [Qt port fix] todo.txt #41: UNIT_NAME_LENGTH is 20 in this tree's
	** nodStruc.h (vs. 50 in the reference tree) -- bound the lithology-name
	** portion so "<name>_<NN>" can never overflow LAYER_PROPERTIES.unitName
	** (14 + '_' + 2 digits + NUL = 18, safely under 20). */
	sprintf((void*) options->unitName, "%.14s_%02d",
			PPHYS_ROCK[lithocode].lithoname, unitNumber++);

	if (layer == 0)
	{
		/* [Qt port fix] todo.txt #72 -- cum_height is `static`, so without
		** resetting it here it kept accumulating across separate Random
		** History invocations in the same session (each new stratigraphy's
		** layers piling on top of the previous one's already-huge heights)
		** instead of starting fresh -- eventually the top layer's height
		** dwarfed everything else, so the block model showed only the
		** lowest/base layer. */
		cum_height = 0;
		options->height = -31000;
	}
	else {
		options->height = 50 + (950.0 * xrshr128p_next_double(&state));
		cum_height += options->height;
		options->height = cum_height;
	}
	options->applyAlterations = TRUE;

	options->density = density;
	options->anisotropicField = 0;
	options->sus_X = magsus;
	options->sus_Y = options->sus_X;
	options->sus_Z = options->sus_X;
	options->sus_dip = 0;
	options->sus_dipDirection = 0;
	options->sus_pitch = 0;
	options->remanentMagnetization = 0;
	options->inclination = 0;
	options->angleWithNorth = 0;
	options->strength = 0;

	loadRandomColor(&(options->color));

	/* calculate and store the forward and
	 ** reverse rotation matricies for remenance */
	convrt(90.0 - options->inclination, 180.0 + options->angleWithNorth, 0.0,
	TRUE);
	rotset(90.0 - options->inclination, 180.0 + options->angleWithNorth, 0.0,
			options->remRotationMatrix.forward,
			options->remRotationMatrix.reverse);

	convrt(options->sus_dip, options->sus_dipDirection, options->sus_pitch,
	TRUE);
	rotset(options->sus_dip, options->sus_dipDirection, options->sus_pitch,
			options->aniRotationMatrix.forward,
			options->aniRotationMatrix.reverse);
	return (TRUE);
}
