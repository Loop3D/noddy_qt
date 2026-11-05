/* ============================================================================
 * [Qt port ADDITION] todo.txt #44 -- new sub-panel on the Geophysics
 * Calculation options tab (a checkbox + editable sigma that, when
 * checked, adds Gaussian noise to each voxel's density/susceptibility
 * -- see geophy.c's assignLayerInBlockModels). Modeled directly on
 * genwin.c (GENERATE_WINDOW), the simplest existing sibling panel in
 * this same tab's stack (optnlib.c's createOptions).
 * ============================================================================
 */

#include "xvt.h"
#include "xvtcm.h"
#include "nodInc.h"

/*
	Information about the window
*/
#define WIN_RES_ID NOISE_WINDOW
#define WIN_FLAGS 0x810L
#define WIN_CLASS ""
#define WIN_BORDER W_DOC

/*
	Handler for window NOISE_WINDOW ("Gaussian Noise")
*/
long XVT_CALLCONV1
#if XVT_CC_PROTO
NOISE_WINDOW_eh XVT_CALLCONV2 (WINDOW xdWindow, EVENT *xdEvent)
#else
NOISE_WINDOW_eh XVT_CALLCONV2 (xdWindow, xdEvent)
WINDOW xdWindow;
EVENT *xdEvent;
#endif
{
	short xdControlId = xdEvent->v.ctl.id;

	switch (xdEvent->type) {
	case E_UPDATE:
		{
		xvt_dwin_clear(xdWindow, (COLOR)xvt_vobj_get_attr(xdWindow, ATTR_BACK_COLOR));
		}
		break;
	case E_CLOSE:
		{
		xvt_vobj_destroy(xdWindow);
		}
		break;
	case E_CONTROL:
		/*
			User operated control in window.
		*/
		{
		switch(xdControlId) {
		case NOISE_ADD: /* "Add Gaussian Noise" */
			{
			WINDOW ctl_win = xvt_win_get_ctl(xdWindow, NOISE_ADD);
			BOOLEAN checked = (BOOLEAN) (!xvt_ctl_is_checked(ctl_win));
			xvt_ctl_set_checked(ctl_win, checked);
			xvt_vobj_set_enabled (xvt_win_get_ctl(xdWindow, NOISE_SIGMA), checked);
			}
			break;
		case NOISE_SIGMA: /* "Sigma" */
			{
			/*
				Edit control was operated -- no per-keystroke action
				needed, value is read back in saveOptions (optnlib.c).
			*/
			}
			break;
		case NOISE_SEED: /* "Seed" */
			{
			/*
				Edit control was operated -- no per-keystroke action
				needed, value is read back in saveOptions (optnlib.c).
			*/
			}
			break;
		default:
			break;
		}
		}
		break;
	default:
		break;
	}
	/* TAG BEGIN SPCL:Bottom */
#ifdef XVT_R40_TXEDIT_BEHAVIOR
	xvt_tx_process_event(xdWindow, xdEvent);
#endif
	/* TAG END SPCL:Bottom */
	return 0L;
}
