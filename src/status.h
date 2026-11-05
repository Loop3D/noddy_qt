/* ============================================================================
 * [XVT -> Qt PORT] This file is unchanged calculation/application logic.
 * Its XVT toolkit calls now run against the Qt5-backed compatibility layer
 * in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the original
 * proprietary XVT SDK. No calculation or algorithmic code in this file was
 * modified to accomplish this port -- only the underlying GUI toolkit
 * implementation, in qt_compat/, changed.
 * ============================================================================
 */
#ifndef STATUS_MESSAGES
#define STATUS_MESSAGES
                            /* Standard Messages that can be displayed in the status bar */
#define STATUS_DEFAULT               "Noddy Geophysical Modelling."
#define STATUS_BUILD_HISTORY         "Build an event History by inserting Events from the Toolbar."

#define STATUS_EVENT_MENU_HELP       "Click on an Event then Click in the History to place the Event"

#define STATUS_BLOCK_HELP            "Hold Left Button to Rotate, Right for Options, SHIFT to Zoom"
#define STATUS_3DSTRAT_HELP          "Hold Left Button to Rotate, SHIFT to Zoom"
#define STATUS_ANOMIMAGE_HELP        "Hold Left Button and Drag for Profile (hold SHIFT to label)"
#endif           
