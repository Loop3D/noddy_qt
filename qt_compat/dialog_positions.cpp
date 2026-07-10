/* ============================================================================
 * Dialog control POSITION overrides, reconstructed from the original help
 * manual's screenshots (Dropbox/noddy/noddy_help_lowercase/*.htm + *.GIF).
 * Layered on top of the widget-TYPE registry in dialog_registry.cpp (which
 * is auto-generated from nodInc.h and has no position data of its own).
 *
 * Entries are added incrementally, dialog by dialog, as each one's manual
 * screenshot is examined and its controls' pixel rectangles measured.
 * A dialog with no entry here just keeps the QFormLayout auto-flow
 * fallback in xvt_compat.cpp -- not every dialog has a manual screenshot.
 * ============================================================================
 */
#include "xvt_types.h"

/* typeOverride: W_NONE = no override, use dialog_registry.cpp's type
 * as-is; anything else replaces it (see xvt_compat.cpp's copy of this
 * struct for why -- e.g. EVENT_WINDOW's buttons get misclassified as
 * plain text edits by the nodInc.h-name-based registry generator).
 * labelOverride: nullptr = use dialog_registry.cpp's auto-generated
 * label as-is; set to real screenshot-confirmed text when the raw
 * nodInc.h-derived name would otherwise be user-visible and wrong (e.g.
 * a QGroupBox title showing literal "EVENT NAME GB" instead of "Event
 * Name"). Must stay in sync with xvt_compat.cpp's copy of this struct. */
struct DialogCtlPosition { int ctlId; short left, top, right, bottom; WIN_TYPE typeOverride = W_NONE; const char *labelOverride = nullptr; bool initiallyDisabled = false; bool hidden = false; bool multiSelect = false; bool editable = false; int radioGroup = 0; };
struct DialogPositionEntry {
    long resId;
    short width, height;
    const DialogCtlPosition *ctls;
    int numCtls;
    /* nullptr = use dialog_registry.cpp's auto-generated title (raw
     * nodInc.h resource name, e.g. "DEFINE COLOUR WINDOW") -- the app
     * itself never calls xvt_vobj_set_title for most of these dialogs, so
     * that raw name is genuinely what shows in the title bar unless
     * overridden here. */
    const char *titleOverride = nullptr;
};

/* ----------------------------------------------------------------------
 * Fold event dialog (foldtwin.c / eventlib.c FOLD case). The on-screen
 * "Fold" window is a fixed outer frame (event name box, 3D preview,
 * tab icons, Previous/Next/OK/Help/Cancel) with a swappable content area
 * where two sub-panel windows are stacked vertically: FFOLD_WINDOW (152,
 * "Form" groupbox) above FDSCL_WINDOW (153, "Scale" groupbox) -- see
 * eventlib.c's createPositionedWindow(FFOLD_WINDOW, ...) followed by
 * createPositionedWindow(FDSCL_WINDOW, ...) at newGroupYPos. Reconstructed
 * from noddy_help_lowercase/IMG00012.GIF (folds.htm's "Fold definition
 * window" screenshot, 576x361) -- coordinates below are LOCAL to each
 * sub-panel's own top-left origin, matching that panel's region within
 * the screenshot's right-hand content area.
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_152[] = {
    { 4,   0,  0, 210, 105, WC_GROUPBOX, "Form" },       /* FFORM GROUPBOX */
    { 2,  10, 20,  80,  35, WC_TEXT, "Fold Type:" },      /* FFORM TYPE LABEL */
    { 3,  85, 18, 200,  38, WC_LISTBUTTON },              /* FFORM TYPE (Sine Fold dropdown) */
    { 5,  10, 45, 140,  60, WC_CHECKBOX, "Single Fold" },  /* FFORM SINGLE */
    { 1,  10, 65, 200,  90, WC_PUSHBUTTON, "Show Profile..." },  /* FFORM PROFILE */
};

static const DialogCtlPosition g_pos_153[] = {
    { 1,   0,  0, 210, 115, WC_GROUPBOX, "Scale" },  /* FDSCL GROUPBOX */
    { 5,  10, 20,  40,  35, WC_TEXT, "Wave" },        /* FDSCL WAVE LABEL */
    { 2,  45, 20, 140,  35, WC_HSCROLL },             /* FDSCL WAVE BAR */
    { 8, 145, 20, 200,  35 },  /* FDSCL WAVE (value edit) */
    { 6,  10, 45,  40,  60, WC_TEXT, "Amp" },         /* FDSCL AMPLITUDE LABEL */
    { 3,  45, 45, 140,  60, WC_HSCROLL },             /* FDSCL AMPLITUDE BAR */
    { 10,145, 45, 200,  60 },  /* FDSCL AMPLITUDE (value edit) */
    { 7,  10, 70,  40,  85, WC_TEXT, "Cycl" },        /* FDSCL CYCL LABEL */
    { 4,  45, 70, 140,  85, WC_HSCROLL },             /* FDSCL CYCL BAR */
    { 9, 145, 70, 200,  85 },  /* FDSCL CYCL (value edit) */
};

/* ----------------------------------------------------------------------
 * Fault event dialog's "Form" tab (eventlib.c FAULT case): GMA_WINDOW (143,
 * Geometry/Movement/Alterations + Define Colour.../Show Profile... buttons)
 * stacked above SURFACE_WINDOW (165, Surface Shape: type dropdown + View...
 * button + X/Y/Z Dim rows -- also reused by Unconformity and Shear Zone).
 * Reconstructed from IMG00019.GIF (faults.htm, 576x361).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_143[] = {
    { 3,   0,  0, 270, 142, WC_GROUPBOX, "Form" },          /* GMA GROUPBOX */
    { 2,  10, 17,  65,  37, WC_TEXT, "Geometry" },           /* GMA GEOMETRY LABEL */
    { 1,  70, 17, 215,  37, WC_LISTBUTTON },                 /* GMA GEOMETRY (dropdown) */
    { 5,  10, 42,  65,  62, WC_TEXT, "Movement" },           /* GMA MOVEMENT LABEL */
    { 4,  70, 42, 215,  62, WC_LISTBUTTON },                 /* GMA MOVEMENT (dropdown) */
    { 7,  10, 67,  75,  87, WC_TEXT, "Alterations" },        /* GMA ALTERATION LABEL */
    { 6,  80, 67, 165,  87, WC_LISTBUTTON },                 /* GMA ALTERATION (dropdown) */
    { 8, 170, 67, 215,  87, WC_PUSHBUTTON, "Edit..." },             /* GMA ALTERATION EDIT */
    { 9,  10, 92, 135, 114, WC_PUSHBUTTON, "Define Colour..." },    /* GMA COLOR BUTTON */
    { 10,145, 92, 270, 114, WC_PUSHBUTTON, "Show Profile..." },     /* GMA PROFILE BUTTON */
};

static const DialogCtlPosition g_pos_165[] = {
    { 3,   0,  0, 270, 115, WC_GROUPBOX, "Surface Shape" },  /* SURFACE GORUPBOX [sic, matches nodInc.h typo] */
    { 2,  10, 15, 155,  35, WC_LISTBUTTON },                 /* SURFACE TYPE LIST ("Flat Plane" dropdown) */
    { 1, 160, 15, 270,  35, WC_PUSHBUTTON, "View..." },      /* SURFACE VIEW */
    { 4,  10, 45,  55,  60, WC_TEXT, "X Dim." },             /* SURFACE XDIM LABEL */
    { 5,  60, 45, 170,  60, WC_HSCROLL },                    /* SURFACE XDIM BAR */
    { 6, 175, 45, 270,  60 },  /* SURFACE XDIM (value edit) */
    { 7,  10, 70,  55,  85, WC_TEXT, "Y Dim." },             /* SURFACE YDIM LABEL */
    { 8,  60, 70, 170,  85, WC_HSCROLL },                    /* SURFACE YDIM BAR */
    { 9, 175, 70, 270,  85 },  /* SURFACE YDIM (value edit) */
    { 10, 10, 95,  55, 110, WC_TEXT, "Z Dim." },             /* SURFACE ZDIM LABEL */
    { 11, 60, 95, 170, 110, WC_HSCROLL },                    /* SURFACE ZDIM BAR */
    { 12,175, 95, 270, 110 },  /* SURFACE ZDIM (value edit) */
};

/* ----------------------------------------------------------------------
 * Unconformity event dialog's "Form" tab (eventlib.c UNCONFORMITY case):
 * ALT_WINDOW (144, just an Alterations dropdown + Edit... button) stacked
 * above the same shared SURFACE_WINDOW (165) as above. Reconstructed from
 * IMG00023.GIF (unc.htm, 576x361).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_144[] = {
    { 4,   0,  0, 270,  62, WC_GROUPBOX, "Form" },   /* ALT GROUPBOX */
    { 2,  10, 20,  75,  40, WC_TEXT, "Alterations" }, /* ALT ALTERATION LABEL */
    { 1,  80, 20, 165,  40, WC_LISTBUTTON },          /* ALT ALTERATION (dropdown) */
    { 3, 170, 20, 215,  40, WC_PUSHBUTTON, "Edit..." },  /* ALT ALTERATION EDIT */
};

/* ----------------------------------------------------------------------
 * Shear Zone event dialog's "Form" tab (eventlib.c SHEAR_ZONE case):
 * TGMA_WINDOW (154, Type/Geometry/Movement/Alterations + Show Profile...)
 * stacked above the shared SURFACE_WINDOW (165). Reconstructed from
 * IMG00026.GIF (sz.htm, 576x361).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_154[] = {
    { 3,   0,  0, 270, 157, WC_GROUPBOX, "Form" },     /* WIN 154 GROUPBOX 93 (outer "Form" frame) */
    { 9,  10, 17,  65,  37, WC_TEXT, "Type" },          /* TGMA TYPE LABEL */
    { 10, 70, 17, 215,  37, WC_LISTBUTTON },            /* TGMA TYPE (dropdown) */
    { 2,  10, 42,  65,  62, WC_TEXT, "Geometry" },      /* TGMA GEOMETRY LABEL */
    { 1,  70, 42, 215,  62, WC_LISTBUTTON },            /* TGMA GEOMETRY (dropdown) */
    { 5,  10, 67,  65,  87, WC_TEXT, "Movement" },      /* TGMA MOVEMENT LABEL */
    { 4,  70, 67, 215,  87, WC_LISTBUTTON },            /* TGMA MOVEMENT (dropdown) */
    { 7,  10, 92,  75, 112, WC_TEXT, "Alterations" },   /* TGMA ALTERATION LABEL */
    { 6,  80, 92, 165, 112, WC_LISTBUTTON },            /* TGMA ALTERATION (dropdown) */
    { 8, 170, 92, 215, 112, WC_PUSHBUTTON, "Edit..." },          /* TGMA ALTERATION EDIT */
    { 11, 10,120, 215, 142, WC_PUSHBUTTON, "Show Profile..." },  /* TGMA PROFILE BUTTON */
};

/* ----------------------------------------------------------------------
 * Dyke event dialog's "Form" tab (eventlib.c DYKE case): FDYKE_WINDOW
 * (157, Dyke Intrusion type + Alterations + Merge Igneous Events checkbox
 * + Edit Properties... button). Reconstructed from IMG00030.GIF
 * (dyke.htm, 576x361).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_157[] = {
    { 4,   0,  0, 270, 132, WC_GROUPBOX, "Form" },              /* FDYKE GROUPBOX */
    { 2,  10, 20,  90,  40, WC_TEXT, "Dyke Intrusion :" },       /* FDYKE TYPE LABEL */
    { 3,  95, 20, 215,  40, WC_LISTBUTTON },                     /* FDYKE TYPE (dropdown) */
    { 6,  10, 47,  90,  67, WC_TEXT, "Alterations" },            /* FDYKE ALTERATION LABEL */
    { 5,  95, 47, 170,  67, WC_LISTBUTTON },                     /* FDYKE ALTERATION (dropdown) */
    { 7, 175, 47, 215,  67, WC_PUSHBUTTON, "Edit..." },             /* FDYKE ALTERATION EDIT */
    { 1,  10, 75, 215,  90, WC_CHECKBOX, "Merge Igneous Events" },  /* FDYKE MERGE */
    { 8,  10,100, 215, 122, WC_PUSHBUTTON, "Edit Properties..." },  /* FDYKE PROP BUTTON */
};

/* ----------------------------------------------------------------------
 * Plug event dialog's "Form" tab (eventlib.c PLUG case): FPLUG_WINDOW
 * (159, Plug Geometry type + Alterations + Merge Igneous Events checkbox
 * + Edit Properties... button) -- structurally identical layout to Dyke's
 * FDYKE_WINDOW above. Reconstructed from IMG00033.GIF (plug.htm, 576x361).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_159[] = {
    { 4,   0,  0, 270, 132, WC_GROUPBOX, "Form" },      /* FPLUG GROUPBOX */
    { 2,  10, 20,  90,  40, WC_TEXT, "Plug Geometry" },  /* FPLUG GEOMETRY LABEL */
    { 3,  95, 20, 215,  40, WC_LISTBUTTON },             /* FPLUG GEOMETRY (dropdown) */
    { 6,  10, 47,  90,  67, WC_TEXT, "Alterations" },    /* FPLUG ALTERATION LABEL */
    { 5,  95, 47, 170,  67, WC_LISTBUTTON },             /* FPLUG ALTERATION (dropdown) */
    { 7, 175, 47, 215,  67, WC_PUSHBUTTON, "Edit..." },             /* FPLUG ALTERATION EDIT */
    { 1,  10, 75, 215,  90, WC_CHECKBOX, "Merge Igneous Events" },  /* FPLUG MERGE */
    { 8,  10,100, 215, 122, WC_PUSHBUTTON, "Edit Properties..." },  /* FPLUG PROP BUTTON */
};

/* ----------------------------------------------------------------------
 * Stratigraphy event dialog (eventlib.c STRATIGRAPHY case, also reused
 * as Unconformity's "Layers" tab): STRATIGRAPHY_WINDOW (169) -- a layer
 * listbox with Edit/Move Up/Move Down/Add/Delete buttons alongside it.
 * Reconstructed from IMG00004.GIF (BaseEv.htm, 576x361).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_169[] = {
    /* ctlId 2/3/5/6 are W_NONE and ctlId 4 is WC_EDIT in the auto-generated
     * registry (none of their nodInc.h names contain a recognizable
     * "PUSHBUTTON" substring) -- without an explicit typeOverride here,
     * createControlWidget's fallback (registry type, or WC_EDIT if that's
     * also W_NONE) turned all five into blank QLineEdit text boxes instead
     * of buttons, i.e. no visible label at all (just an empty editable
     * box) -- matches a user report of "many buttons have no labels".
     * Same root cause likely affects other dialogs' hand-added position
     * entries that also omitted typeOverride for a button-shaped control;
     * this was the one confirmed via an actual screenshot. */
    { 7,  10, 10, 140,  25, WC_TEXT, "Stratigraphy Layers:" },  /* STRATIGRAPHY LABEL */
    { 1,  10, 30, 130, 240, WC_LBOX },  /* STRATIGRAPHY LIST (layer listbox) -- registry misclassified this W_NONE -> WC_EDIT (a single-line text box); real control is a scrollable, selectable layer list ("Top, Upper, U Middle, Middle, L Middle, Lower, Base" per the real screenshot) */
    { 4, 140, 30, 260,  50, WC_PUSHBUTTON, "Edit..." },
    { 5, 140, 60, 260,  80, WC_PUSHBUTTON, "Move Up" },
    { 6, 140, 90, 260, 110, WC_PUSHBUTTON, "Move Down" },
    { 2, 140,150, 260, 170, WC_PUSHBUTTON, "Add" },
    { 3, 140,180, 260, 200, WC_PUSHBUTTON, "Delete" },
};

/* ----------------------------------------------------------------------
 * Shared Position panel (used by Fold, Fault, Dyke, Plug, Import, Tilt):
 * XYZ_WINDOW (139) -- X/Y/Z labelled scrollbar+edit rows. Reconstructed
 * from IMG00045.GIF (import.htm's Form/Position tab, 576x361), where it
 * appears stacked below CORNER_WINDOW (162, Import's own "Form" controls:
 * Center/SWUpper Corner radio buttons + Select File to Import... button).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_162[] = {
    { 1,   0,  0, 270,  82, WC_GROUPBOX, "Form" },              /* CORNER GROUPBOX */
    { 2,  10, 15, 110,  35, WC_RADIOBUTTON, "Center" },          /* CORNER CENTER */
    { 3, 115, 15, 260,  35, WC_RADIOBUTTON, "SWUpper Corner" },  /* CORNER SWUPPER */
    { 4,  10, 45, 260,  67, WC_PUSHBUTTON, "Select File to Import..." },  /* CORNER IMPORT FILE */
};

static const DialogCtlPosition g_pos_139[] = {
    /* All LABEL/BAR fields below previously had no typeOverride, and the
     * auto-generated registry has them as W_NONE -> both silently fell
     * back to plain WC_EDIT text boxes (a LABEL rendering as an editable
     * empty box instead of static text, a BAR rendering as another
     * editable box instead of a functional slider) -- confirmed via a
     * systematic audit of every event dialog's Form/Scale/Position/
     * Orientation position table against dialog_registry.cpp, prompted by
     * a user report that "most don't have the correct widget types". */
    { 1,   0,  0, 270, 110, WC_GROUPBOX },              /* XYZ GB */
    { 2,  10, 20,  45,  35, WC_TEXT, "X (E)" },          /* XYZ X TEXT */
    { 8,  50, 20, 160,  35, WC_HSCROLL },                /* XYZ X BAR */
    { 5, 165, 20, 260,  35 },  /* XYZ X (value edit) */
    { 3,  10, 50,  45,  65, WC_TEXT, "Y (N)" },          /* XYZ Y TEXT */
    { 9,  50, 50, 160,  65, WC_HSCROLL },                /* XYZ Y BAR */
    { 7, 165, 50, 260,  65 },  /* XYZ Y (value edit) */
    { 4,  10, 80,  45,  95, WC_TEXT, "Z (Up)" },         /* XYZ Z TEXT */
    { 10, 50, 80, 160,  95, WC_HSCROLL },                /* XYZ Z BAR */
    { 6, 165, 80, 260,  95 },  /* XYZ Z (value edit) */
};

/* ----------------------------------------------------------------------
 * Define Colour dialog (colwin.c / DEFINE_COLOUR_WINDOW, resId 104):
 * "Colour Name" dropdown + swatch, big predefined-colours grid, "Custom
 * Colour" Red/Green/Blue sliders, and a custom-colours swatch row.
 * Reconstructed from IMG00070.GIF (colour.htm, approx 470x300).
 * ---------------------------------------------------------------------- */
/* ctl8/9/10 were misidentified in an earlier pass as a "Colour Name"
 * dropdown/swatch/custom-colours-row -- color.c's actual E_CONTROL switch
 * (checked directly) shows DEFINE_COLOUR_WINDOW_P_C12/13/14 are really the
 * OK/Cancel/Help buttons (each case has an explicit "OK"/"Cancel"/"Help..."
 * comment from the original XVT-Design generator), and were positioned
 * near the top instead of the bottom as a result. There is no separate
 * registry control for a "Colour Name" dropdown or preview swatch -- only
 * 14 controls exist total (verified against g_ctls_104) -- so that part
 * of the reference screenshot is likely custom-drawn by the app rather
 * than a discrete widget; not fabricated here. */
static const DialogCtlPosition g_pos_104[] = {
    { 12,   5,   5, 230,  70, WC_GROUPBOX, "Colour Name" },
    { 13,   5,  75, 150,  90, WC_TEXT, "Predefined Colours:" },  /* DEFINE COLOUR WINDOW TEXT 45 */
    /* Real XVT uses this purely as a data backing-store (selected-colour
     * title, SLIST of names) -- the visible/clickable "Predefined Colours"
     * grid is separately painted by nodGraph.c's drawDefinedColorGrid
     * straight onto this same on-screen rect (see DialogCtlPosition::hidden
     * doc comment in xvt_compat.cpp). Rendering it as a normal opaque
     * QListWidget hid the real swatch grid underneath it -- confirmed via
     * user screenshot showing only a sliver of the colour grid peeking out
     * above an opaque "Color 1"/"Color 2"/... text list. */
    { 11,   5,  95, 230, 270, W_NONE, nullptr, false, true },  /* DEFINE COLOUR WINDOW LISTEDIT 41 (colour grid, hidden -- see comment) */
    { 7,   240,  5, 460, 140, WC_GROUPBOX, "Custom Colour" },  /* WIN 104 GROUPBOX 11 */
    /* ctl4/5/6 are already WC_TEXT in the registry (so didn't need a
     * typeOverride), but had no labelOverride either -- fell back to the
     * raw auto-generated name ("WIN 104 TEXT 8" etc, truncated to "WIN 1"
     * at this narrow width), not the real "Red"/"Green"/"Blue" this
     * comment always claimed. A comment describing intent is not the same
     * as actually setting it -- confirmed via a user screenshot. */
    { 4,   250, 25, 285,  40, W_NONE, "Red" },    /* WIN 104 TEXT 8 */
    { 1,   290, 25, 440,  40 },  /* WIN 104 HSCROLL 5 (Red slider) */
    { 5,   250, 50, 285,  65, W_NONE, "Green" },  /* WIN 104 TEXT 9 */
    { 2,   290, 50, 440,  65 },  /* WIN 104 HSCROLL 6 (Green slider) */
    { 6,   250, 75, 285,  90, W_NONE, "Blue" },   /* WIN 104 TEXT 10 */
    { 3,   290, 75, 440,  90 },  /* WIN 104 HSCROLL 7 (Blue slider) */
    { 14,  240,150, 380, 165, WC_TEXT, "Custom Colours:" },  /* DEFINE COLOUR WINDOW TEXT 46 */
    {  8, 150,280, 230,302, WC_PUSHBUTTON, "OK" },       /* DEFINE COLOUR WINDOW P C12, real "OK" */
    {  9, 240,280, 320,302, WC_PUSHBUTTON, "Cancel" },   /* DEFINE COLOUR WINDOW P C13, real "Cancel" */
    { 10, 330,280, 410,302, WC_PUSHBUTTON, "Help..." },  /* DEFINE COLOUR WINDOW P C14, real "Help..." */
};

/* ----------------------------------------------------------------------
 * Geology Display options panel (resId 116, one of the "Options" mega-
 * dialogs' selectable panels -- "View Direction" Declination/Azimuth/
 * Scale sliders and "Chair Diagram" checkbox + X/Y/Z Incut sliders.
 * Reconstructed from IMG00087.GIF (geoldisp.htm, 576x361). The dialog's
 * "Option Selection" dropdown, 3D preview, and OK/Help/Cancel/pushbuttons
 * (ids 1-3, 25, 26) are shared outer-frame furniture not confidently
 * identifiable from the generic nodInc.h names -- left on the fallback
 * stacking so they still appear, just not pixel-exact.
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_116[] = {
    { 4,   0,  0, 270, 105 },  /* WIN 116 GROUPBOX 50 ("View Direction") */
    { 8,  10, 20,  75,  38 },  /* WIN 116 TEXT 54 ("Declination") */
    { 5,  80, 20, 185,  38 },  /* WIN 116 HSCROLL 51 (Declination slider) */
    { 11,190, 20, 260,  38 },  /* WIN 116 EDIT 57 (Declination value) */
    { 9,  10, 45,  75,  63 },  /* WIN 116 TEXT 55 ("Azimuth") */
    { 6,  80, 45, 185,  63 },  /* WIN 116 HSCROLL 52 (Azimuth slider) */
    { 12,190, 45, 260,  63 },  /* WIN 116 EDIT 58 (Azimuth value) */
    { 10, 10, 70,  75,  88 },  /* WIN 116 TEXT 56 ("Scale") */
    { 7,  80, 70, 185,  88 },  /* WIN 116 HSCROLL 53 (Scale slider) */
    { 13,190, 70, 260,  88 },  /* WIN 116 EDIT 59 (Scale value) */
    { 14,  0,115, 270, 245 },  /* WIN 116 GROUPBOX 60 ("Chair Diagram") */
    { 18, 10,130, 200, 150 },  /* WIN 116 CHECKBOX 68 ("Use Chair Diagram") */
    { 15, 10,155,  75, 173 },  /* WIN 116 TEXT 61 ("X Incut") */
    { 19, 80,155, 185, 173 },  /* WIN 116 HSCROLL 73 */
    { 22,190,155, 260, 173 },  /* WIN 116 EDIT 76 */
    { 16, 10,180,  75, 198 },  /* WIN 116 TEXT 62 ("Y Incut") */
    { 20, 80,180, 185, 198 },  /* WIN 116 HSCROLL 74 */
    { 23,190,180, 260, 198 },  /* WIN 116 EDIT 77 */
    { 17, 10,205,  75, 223 },  /* WIN 116 TEXT 63 ("Z Incut") */
    { 21, 80,205, 185, 223 },  /* WIN 116 HSCROLL 75 */
    { 24,190,205, 260, 223 },  /* WIN 116 EDIT 78 */
};

/* ----------------------------------------------------------------------
 * EVENT_WINDOW (137, eventwin.c) -- the shared OUTER frame every event
 * type's dialog is built from (see builder.c's createEventWindow): Event
 * Name box + edit, a placeholder for the 3D preview canvas (the real
 * preview is a separate window createPreviewWindow() positions on top of
 * this control's rect, see EVENT_PREVIEW below), Previous/Next/OK/Help/
 * Cancel buttons, and an Option Selection dropdown. eventlib.c nests each
 * event type's actual parameter-group sub-panels (Fold's Form/Scale,
 * Fault's Form, etc, all already in this table above) inside this frame
 * starting at (groupXPos=150, groupYPos=16) -- those two numbers are
 * hardcoded in eventlib.c itself for Windows builds, not reconstructed
 * here, so this layout reserves that exact region for them. All 9 of
 * this window's controls were misclassified W_NONE (-> WC_EDIT, a plain
 * text box) by the nodInc.h-name-based registry generator since none of
 * their names contain a recognizable keyword like "PUSHBUTTON" --
 * typeOverride corrects each one to its real widget type. Labels/layout
 * cross-referenced against the shared frame visible in every event-type
 * screenshot (IMG00012.GIF etc, see the FOLD/FAULT/... entries above).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_137[] = {
    { 3,   8,  8, 140,  70, WC_GROUPBOX, "Event Name" },  /* EVENT NAME GB -- real label confirmed via user-provided Stratigraphy screenshot */
    { 2,  15, 28, 133,  48, WC_EDIT },        /* EVENT NAME (edit) */
    /* EVENT PREVIEW (3D canvas placeholder, no visible title in the real
     * app -- createPreviewWindow positions the real preview window
     * directly on top of this rect, and its own code already right/
     * bottom-anchors the Event-type/Block-type dropdowns and "On"
     * checkbox within it -- xvt_vobj_get_client_rect(previewWin,...)
     * then listRect.top = bottom-20 / listRect.left = right-N, see
     * builder.c's createPreviewWindow). Width is capped at ~140 by
     * eventlib.c's hardcoded groupXPos=150 (where the nested Stratigraphy/
     * Fold/etc content sub-panel starts) -- widening past that would
     * overlap it. Shaped landscape (wider than tall) within that width
     * cap by keeping height modest, was previously 132x215 (a tall
     * portrait strip, user-reported as visibly wrong shape). */
    { 6,   8, 80, 145, 195, WC_GROUPBOX, "" },
    { 7, 150,  0, 380,  14, WC_LISTBUTTON },  /* EVENT OPTION SELECTION (Form/Scale-style dropdown) */
    { 8,   8,305, 120, 327, WC_PUSHBUTTON, "Previous Event" },  /* EVENT PREVIOUS */
    { 9, 128,305, 230, 327, WC_PUSHBUTTON, "Next Event" },      /* EVENT NEXT */
    { 1, 330,305, 380, 327, WC_PUSHBUTTON, "OK" },              /* EVENT OK */
    { 5, 388,305, 438, 327, WC_PUSHBUTTON, "Help..." },         /* EVENT HELP */
    { 4, 446,305, 496, 327, WC_PUSHBUTTON, "Cancel" },          /* EVENT CANCEL */
};

/* ----------------------------------------------------------------------
 * OPTIONS_WINDOW (183, optnlib.c) -- the shared OUTER frame for the ten
 * "Options" mega-dialogs (Project, Block, Movie, Geology Display,
 * Section/Borehole, Volume/Surface, Geophysics Calculation/Survey/
 * Display, Windows), all opened via createCenteredWindow(OPTIONS_WINDOW,
 * ...) from mainMenu.c with a different OPTION_TYPE tag each time. Same
 * architecture as EVENT_WINDOW above: a fixed frame (Option Selection
 * dropdown, a 3D preview placeholder createPreviewWindow() positions the
 * real preview window on top of, OK/Help/Cancel) with ~30 distinct
 * swappable content sub-panels nested inside by optnlib.c's
 * createOptions()/setOptionsTab() -- e.g. Project's page stacks
 * LOCAL_REMOTE_WINDOW+UNIT_WINDOW (side by side) then PRINT_SCALE_WINDOW
 * then HELP_OPTION_WINDOW. Reconstructed from ten user-provided real
 * screenshots (one per Option Selection value) -- ONLY this shared outer
 * frame is done so far; the ~30 inner content sub-panels are NOT yet
 * reconstructed (still on QFormLayout auto-flow fallback), a
 * substantially larger follow-up task. Positions here are an
 * approximate read of the screenshots' proportions (same standard of
 * "close enough, not pixel-exact" as this file's other screenshot-based
 * entries), not exact -- unlike the NoddyNG.rc-derived entries below,
 * there's no real resource data backing these.
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_183[] = {
    /* Both the Option Selection groupbox+dropdown and the Preview
     * placeholder were previously wider than 150px, overlapping the
     * inner content sub-panels optnlib.c positions starting at
     * groupXPos=150 (see this session's eventlib.c/optnlib.c groupXPos
     * fix) -- the not-yet-reconstructed sub-panels' auto-flowed controls
     * rendered on top of/covering the right side of this dropdown,
     * matching a user report of it being "covered to the right by other
     * widgets". Narrowed to stay clear of x=150, same constraint
     * EVENT_WINDOW's own g_pos_137 entry already respects. */
    { 3,   8,  8, 145,  95, WC_GROUPBOX, "Option Selection" },  /* OPTIONS SELECTION GB */
    { 2,  15, 45, 140,  68, WC_LISTBUTTON },                     /* OPTIONS SELECTION (dropdown) */
    { 1,   8,100, 145, 400, WC_GROUPBOX, "" },  /* OPTIONS PREVIEW (3D canvas + Event/Block type dropdowns + On checkbox placeholder -- createPreviewWindow positions the real preview window on top of this rect) */
    /* ctlId 5/6 were swapped: optwin.c's real E_CONTROL switch (confirmed
     * from source) is unambiguous --
     * OPTIONS_WINDOW_PUSHBUTTON_107(4)="OK", _108(5)="Cancel",
     * _109(6)="Help...". Labeling ctlId 5 "Help..." and 6 "Cancel" (the
     * previous guess, made without checking optwin.c) meant clicking the
     * button visually labeled "Help..." actually fired the real Cancel
     * action and vice versa -- user-reported as "cancel and help buttons
     * are switched in function". Positions swapped too so the visible
     * L-to-R order stays OK/Help/Cancel, matching every reference
     * screenshot. Help is disabled per explicit request: displayHelp()
     * shells out to a hardcoded Windows browser command
     * (c:\winprgs\Noddy\iexplore %h, see the Project Options "Help
     * Viewer" panel) that has no equivalent in this port yet, so a live
     * Help button would just silently fail. */
    { 4, 430,392, 495, 412, WC_PUSHBUTTON, "OK" },                             /* OPTIONS WINDOW PUSHBUTTON 107 */
    { 6, 500,392, 560, 412, WC_PUSHBUTTON, "Help...", true },                 /* OPTIONS WINDOW PUSHBUTTON 109 */
    { 5, 565,392, 625, 412, WC_PUSHBUTTON, "Cancel" },                         /* OPTIONS WINDOW PUSHBUTTON 108 */
};

/* ----------------------------------------------------------------------
 * The following entries (140, 141, 142, 145, 146, 155, 156, 158, 160,
 * 161, 163, 164, 168) are reconstructed from a DIFFERENT, independent
 * source than the screenshot-based entries above: NoddyNG.rc, a later
 * MFC/C++ rewrite of Noddy found alongside the original XVT source tree
 * (C:\...\noddy_src\NODDY\NoddyNG\NoddyNG.rc). That rewrite is a totally
 * different GUI architecture (MFC property-page dialogs, IDC_* control
 * IDs) -- NOT a drop-in match for this XVT codebase -- but its dialog
 * captions and field groupings correspond 1:1 to this app's event-dialog
 * sub-panels (cross-checked against eventlib.c's createEventOptions,
 * which nails down exactly which XVT *_WINDOW resId backs which MFC page:
 * e.g. FAULT's "Position/Orientation" tab is XYZ_WINDOW(139) then
 * DDP_WINDOW(140) stacked, matching IDD_FAULT_PAGE2's "Position" and
 * "Orientation" groupboxes). Real pixel rects were extracted from the
 * MFC dialog's DIALOG/CONTROL statements and rebased so each panel's
 * topmost control starts near local y=0 (matching this file's existing
 * "coordinates are local to the sub-panel" convention); ctlId numbers
 * still come from THIS app's own dialog_registry.cpp/nodInc.h, matched
 * to the MFC fields by label text and field semantics, not by ID number
 * (the two ID schemes are unrelated). These are all dialogs that
 * previously had NO position entry here (pure QFormLayout fallback).
 * ---------------------------------------------------------------------- */

/* DDP_WINDOW (140): Dip Dir./Dip/Pitch -- shared "Orientation" sub-panel
 * for Fault, Shear Zone, Dyke and Plug's Position/Orientation tabs.
 * Source: IDD_FAULT_PAGE2 (identical in KINK_PAGE3/DYKE_PAGE2/PLUG_PAGE2). */
static const DialogCtlPosition g_pos_140[] = {
    { 1,   7,  0, 198,  54, WC_GROUPBOX, "Orientation" },  /* WIN 140 GROUPBOX 42 */
    { 5,  18,  9,  52,  17, WC_TEXT, "Dip Dir." },          /* WIN 140 TEXT 46 */
    { 2,  52,  9, 158,  18, WC_HSCROLL },                   /* DDP DDIR BAR */
    { 8, 158,  7, 193,  19 },  /* DDP DDIR (value edit) */
    { 6,  19, 24,  52,  32, WC_TEXT, "Dip" },               /* WIN 140 TEXT 47 */
    { 3,  52, 24, 158,  33, WC_HSCROLL },                   /* DDP DIP BAR */
    { 10,158, 22, 193,  34 },  /* DDP DIP (value edit) */
    { 7,  17, 40,  52,  48, WC_TEXT, "Pitch" },             /* DDP PITCH LABEL */
    { 4,  52, 40, 158,  49, WC_HSCROLL },                   /* DDP PITCH BAR */
    { 9, 158, 38, 193,  50 },  /* DDP PITCH (value edit) */
};

/* TRACE_BUTTON_WINDOW (168): the lone "Trace From Image..." button
 * stacked below XYZ_WINDOW+DDP_WINDOW on Fault/Shear's Position tab. */
static const DialogCtlPosition g_pos_168[] = {
    /* Comment here always claimed the real label ("Trace From Image...")
     * but the struct literal never actually set labelOverride, so it fell
     * back to the raw auto-generated ctlId name "TRACE BUTTON" -- shown
     * truncated ("RACE BUTTO") in this narrow box, matching a user
     * screenshot circling this exact button. Also widened -- 74px was
     * sized for "Trace Button", not the real, longer label. */
    { 1,   5,  0, 200,  16, WC_PUSHBUTTON, "Trace from Image..." },
};

/* ROTATION_WINDOW (141): Tilt event's "Scale" sub-panel (single Rotation
 * Around Axis slider). Source: IDD_TILT_PAGE1's third groupbox. */
static const DialogCtlPosition g_pos_141[] = {
    { 1,   7,  0, 198,  33, WC_GROUPBOX, "Scale" },          /* ROTATION WINDOW GROUPBOX */
    { 3,   7, 10,  51,  28, WC_TEXT, "Rotation" },           /* ROTATION ROT LABEL */
    { 2,  51, 14, 157,  23, WC_HSCROLL },                    /* ROTATION ROT BAR */
    { 4, 157, 12, 192,  24 },  /* ROTATION ROT (value edit) */
    { 5,   7, 28,  51,  33, WC_TEXT, "Around Axis" },        /* ROTATION WINDOW TEXT 51 */
};

/* PP_WINDOW (142): Plunge Dir./Plunge -- Lineation's sole "Orientation"
 * tab and Tilt's second (axis-orientation) sub-panel. Source:
 * IDD_LINEATION_PAGE1. */
static const DialogCtlPosition g_pos_142[] = {
    { 1,   7,  0, 198,  40, WC_GROUPBOX, "Orientation" },   /* PP GROUPBOX */
    { 4,  13,  9,  52,  17, WC_TEXT, "Plunge Dir." },        /* PP PDIR LABEL */
    { 2,  52,  9, 158,  18, WC_HSCROLL },                    /* PP PDIR BAR */
    { 6, 158,  7, 193,  19 },  /* PP PDIR (value edit) */
    { 5,  19, 24,  52,  32, WC_TEXT, "Plunge" },             /* PP PLUNGE LABEL */
    { 3,  52, 24, 158,  33, WC_HSCROLL },                    /* PP PLUNGE BAR */
    { 7, 158, 22, 193,  34 },  /* PP PLUNGE (value edit) */
};

/* DDD_WINDOW (145): Dip Dir./Dip -- Foliation's sole "Orientation" tab
 * and Unconformity's second (orientation) sub-panel. Source:
 * IDD_FOLIATION_PAGE1. */
static const DialogCtlPosition g_pos_145[] = {
    { 1,   7,  0, 198,  40, WC_GROUPBOX, "Orientation" },  /* DDD GROUPBOX */
    { 4,  18,  9,  52,  17, WC_TEXT, "Dip Dir." },          /* DDD DDIR LABEL */
    { 2,  52,  9, 158,  18, WC_HSCROLL },                   /* DDD DDIR BAR */
    { 6, 158,  7, 193,  19 },  /* DDD DDIR (value edit) */
    { 5,  19, 24,  52,  32, WC_TEXT, "Dip" },               /* DDD DIP LABEL */
    { 3,  52, 24, 158,  33, WC_HSCROLL },                   /* DDD DIP BAR */
    { 7, 158, 22, 193,  34 },  /* DDD DIP (value edit) */
};

/* MATRIX_WINDOW (146): Strain event's 3x3 deformation tensor grid (its
 * only tab). Source: IDD_STRAIN_PAGE1. */
static const DialogCtlPosition g_pos_146[] = {
    { 10,  7,  0, 198,  94, WC_GROUPBOX },  /* MATRIX GROUPBOX ("Tensor") */
    { 1,  22, 19,  62,  33 },  /* MATRIX TOP LEFT */
    { 2,  80, 19, 120,  33 },  /* MATRIX TOP CENTER */
    { 3, 139, 19, 179,  33 },  /* MATRIX TOP RIGHT */
    { 4,  22, 45,  62,  59 },  /* MATRIX CENTER LEFT */
    { 5,  80, 45, 120,  59 },  /* MATRIX CENTER CENTER */
    { 6, 139, 45, 179,  59 },  /* MATRIX CENTER RIGHT */
    { 7,  22, 72,  62,  86 },  /* MATRIX BOTTOM LEFT */
    { 8,  80, 72, 120,  86 },  /* MATRIX BOTTOM CENTER */
    { 9, 139, 72, 179,  86 },  /* MATRIX BOTTOM RIGHT */
};

/* FSCL_WINDOW (155): Fault's "Scale" tab (Rotation/Slip/Amplitude/
 * Radius/Cyl Index/Prof Pitch). Source: IDD_FAULT_PAGE3. */
static const DialogCtlPosition g_pos_155[] = {
    { 4,   7,  0, 198, 101, WC_GROUPBOX, "Scale" },  /* FSCL GROUPBOX */
    { 6,  18,  9,  52,  17, WC_TEXT, "Rotation" },    /* FSCL ROTATION LABEL */
    { 5,  52,  9, 158,  18, WC_HSCROLL },             /* FSCL ROTATION BAR */
    { 7, 158,  7, 193,  19 },  /* FSCL ROTATION (value edit) */
    { 2,  19, 24,  52,  32, WC_TEXT, "Slip" },        /* FSCL SLIP LABEL */
    { 1,  52, 24, 158,  33, WC_HSCROLL },             /* FSCL SLIP BAR */
    { 3, 158, 22, 193,  34 },  /* FSCL SLIP (value edit) */
    { 9,  17, 40,  52,  48, WC_TEXT, "Amplitude" },   /* FSCL AMPLITUDE LABEL */
    { 8,  52, 40, 158,  49, WC_HSCROLL },             /* FSCL AMPLITUDE BAR */
    { 10,158, 38, 193,  50 },  /* FSCL AMPLITUDE (value edit) */
    { 12, 17, 55,  51,  63, WC_TEXT, "Radius" },      /* FSCL RADIUS LABEL */
    { 11, 51, 55, 157,  64, WC_HSCROLL },             /* FSCL RADIUS BAR */
    { 13,157, 53, 192,  65 },  /* FSCL RADIUS (value edit) */
    { 15, 18, 70,  51,  78, WC_TEXT, "Cyl Index" },   /* FSCL CYL LABEL */
    { 14, 51, 70, 157,  79, WC_HSCROLL },             /* FSCL CYL BAR */
    { 16,157, 68, 192,  80 },  /* FSCL CYL (value edit) */
    { 18, 16, 86,  51,  94, WC_TEXT, "Prof.Pitch" },  /* FSCL PP LABEL */
    { 17, 51, 86, 157,  95, WC_HSCROLL },             /* FSCL PP BAR */
    { 19,157, 84, 192,  96 },  /* FSCL PP (value edit) */
};

/* SSCL_WINDOW (156): Shear Zone's "Scale" tab -- same fields as FSCL
 * plus a Width row. Source: IDD_KINK_PAGE4. */
static const DialogCtlPosition g_pos_156[] = {
    { 4,   7,  0, 198, 118, WC_GROUPBOX, "Scale" },  /* SSCL WINDOW GROUPBOX 54 */
    { 6,  18,  9,  52,  17, WC_TEXT, "Rotation" },    /* SSCL ROTATION LABEL */
    { 5,  52,  9, 158,  18, WC_HSCROLL },             /* SSCL ROTATION BAR */
    { 7, 158,  7, 193,  19 },  /* SSCL ROTATION (value edit) */
    { 2,  19, 24,  52,  32, WC_TEXT, "Slip" },        /* SSCL SLIP LABEL */
    { 1,  52, 24, 158,  33, WC_HSCROLL },             /* SSCL SLIP BAR */
    { 3, 158, 22, 193,  34 },  /* SSCL SLIP (value edit) */
    { 9,  17, 40,  52,  48, WC_TEXT, "Amplitude" },   /* SSCL AMPLITUDE LABEL */
    { 8,  52, 40, 158,  49, WC_HSCROLL },             /* SSCL AMPLITUDE BAR */
    { 10,158, 38, 193,  50 },  /* SSCL AMPLITUDE (value edit) */
    { 21, 17, 55,  51,  63, WC_TEXT, "Width" },       /* SSCL WIDTH LABEL */
    { 20, 51, 55, 157,  64, WC_HSCROLL },             /* SSCL WIDTH BAR */
    { 22,157, 53, 192,  65 },  /* SSCL WIDTH (value edit) */
    { 12, 17, 71,  51,  79, WC_TEXT, "Radius" },      /* SSCL RADIUS LABEL */
    { 11, 51, 71, 157,  80, WC_HSCROLL },             /* SSCL RADIUS BAR */
    { 13,157, 69, 192,  81 },  /* SSCL RADIUS (value edit) */
    { 15, 18, 86,  51,  94, WC_TEXT, "Cyl Index" },   /* SSCL CYL LABEL */
    { 14, 51, 86, 157,  95, WC_HSCROLL },             /* SSCL CYL BAR */
    { 16,157, 84, 192,  96 },  /* SSCL CYL (value edit) */
    { 18, 16,102,  51, 110, WC_TEXT, "Prof.Pitch" },  /* SSCL PP LABEL */
    { 17, 51,102, 157, 111, WC_HSCROLL },             /* SSCL PP BAR */
    { 19,157,100, 192, 112 },  /* SSCL PP (value edit) */
};

/* SW_WINDOW (158): Dyke's "Scale" tab (Slip/Width). Source:
 * IDD_DYKE_PAGE3. */
static const DialogCtlPosition g_pos_158[] = {
    { 1,   7,  0, 198,  39, WC_GROUPBOX, "Scale" },  /* SW GROUPBOX */
    { 4,  18,  9,  52,  17, WC_TEXT, "Slip" },        /* SW SLIP LABEL */
    { 2,  52,  9, 158,  18, WC_HSCROLL },             /* SW SLIP BAR */
    { 6, 158,  7, 193,  19 },  /* SW SLIP (value edit) */
    { 5,  19, 24,  52,  32, WC_TEXT, "Width" },       /* SW WIDTH LABEL */
    { 3,  52, 24, 158,  33, WC_HSCROLL },             /* SW WIDTH BAR */
    { 7, 158, 22, 193,  34 },  /* SW WIDTH (value edit) */
};

/* PSCL_WINDOW (160): Plug's first "Scale" sub-panel (Radius/Angle/
 * BValue). Source: IDD_PLUG_PAGE3's top groupbox. */
static const DialogCtlPosition g_pos_160[] = {
    { 1,   7,  0, 198,  52, WC_GROUPBOX, "Scale" },  /* PSCL GROUPBOX */
    { 4,  18,  9,  52,  17, WC_TEXT, "Radius" },      /* PSCL RADIUS LABEL */
    { 2,  52,  9, 158,  18, WC_HSCROLL },             /* PSCL RADIUS BAR */
    { 6, 158,  7, 193,  19 },  /* PSCL RADIUS (value edit) */
    { 5,  19, 24,  52,  32, WC_TEXT, "Angle" },       /* PSCL ANGLE LABEL */
    { 3,  52, 24, 158,  33, WC_HSCROLL },             /* PSCL ANGLE BAR */
    { 7, 158, 22, 193,  34 },  /* PSCL ANGLE (value edit) */
    { 9,  18, 37,  51,  45, WC_TEXT, "B Value" },     /* PSCL BVALUE LABEL */
    { 8,  51, 37, 157,  46, WC_HSCROLL },             /* PSCL BVALUE BAR */
    { 10,157, 35, 192,  47 },  /* PSCL BVALUE (value edit) */
};

/* AXIS_WINDOW (161): "Scale (Ellipsoid)" X/Y/Z axis lengths -- shared by
 * Fault, Shear Zone and Plug's ellipsoid-scale tab. Source:
 * IDD_FAULT_PAGE4 (identical in KINK_PAGE5 and PLUG_PAGE3's 2nd panel). */
static const DialogCtlPosition g_pos_161[] = {
    { 1,   7,  0, 198,  54, WC_GROUPBOX, "Scale (Axis of Ellipsoid)" },  /* AXIS GROUPBOX */
    { 5,  18,  9,  52,  17, WC_TEXT, "X Axis" },      /* AXIS X LABEL */
    { 2,  52,  9, 158,  18, WC_HSCROLL },             /* AXIS X BAR */
    { 8, 158,  7, 193,  19 },  /* AXIS X (value edit) */
    { 6,  19, 24,  52,  32, WC_TEXT, "Y Axis" },      /* AXIS Y LABEL */
    { 3,  52, 24, 158,  33, WC_HSCROLL },             /* AXIS Y BAR */
    { 10,158, 22, 193,  34 },  /* AXIS Y (value edit) */
    { 7,  17, 40,  52,  48, WC_TEXT, "Z Axis" },      /* AXIS Z LABEL */
    { 4,  52, 40, 158,  49, WC_HSCROLL },             /* AXIS Z BAR */
    { 9, 158, 38, 193,  50 },  /* AXIS Z (value edit) */
};

/* ISCL_WINDOW (164) + IPROP_WINDOW (163): Import's "Scale/Properties"
 * tab -- Block Size slider, then a Rock Properties/layer dropdown with a
 * transparency edit. Source: IDD_IMPORT_PAGE2. */
static const DialogCtlPosition g_pos_164[] = {
    { 1,   7,  0, 198,  24, WC_GROUPBOX, "Block Size" },  /* ISCL GROUPBOX */
    { 4,  18,  9,  52,  17, WC_TEXT, "Size" },             /* ISCL BSIZE LABEL */
    { 3,  52,  9, 158,  18, WC_HSCROLL },                  /* ISCL BSIZE BAR */
    { 2, 158,  7, 193,  19 },  /* ISCL BSIZE (value edit) */
};

static const DialogCtlPosition g_pos_163[] = {
    { 1,   7,  0, 198,  50, WC_GROUPBOX, "Rock Properties" },  /* IPROP GOURPBOX [sic, matches nodInc.h typo] */
    { 2,  47, 14, 158,  28, WC_LISTBUTTON },  /* IPROP PROPERTIES (rock/layer type dropdown) */
    { 3,  54, 35, 128,  42, WC_TEXT, "Transparent Layers 0 +" },  /* IPROP TRANSPARENT LABEL */
    { 4, 130, 33, 153,  46 },                 /* IPROP TRANSPARENT (value edit) */
};

/* ----------------------------------------------------------------------
 * JOB_STATUS_WINDOW (124, nodLib2.c's initLongJob/finishLongJob) -- the
 * "Calculation Status" progress dialog shown during long calculations
 * (block diagrams, geophysics, etc). Registry has 2 generic auto-named
 * text controls ("WIN 124 TEXT 1"/"2", unclear original purpose) plus
 * the 3 semantically-real ones the app actually drives
 * (JOB_MESSAGE/JOB_PERCENTAGE/JOB_STATUS_CANCEL, see initLongJob) -- with
 * no position/type overrides, all 5 fell onto the generic QFormLayout
 * auto-flow stack looking like uniform boxy "spurious" edit fields (user
 * report). Repurposed the two generic text controls as static prefix
 * labels ("Message:"/"Percentage:") for a clean two-row layout, matching
 * the user's explicit ask: just the message, the percentage, and a
 * cancel button, nothing else.
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_124[] = {
    { 1,  10, 12,  85,  28, WC_TEXT, "Message:" },      /* WIN 124 TEXT 1 */
    { 4,  90, 12, 290,  28, WC_TEXT },                  /* JOB MESSAGE (e.g. "Calculating Block Diagram...") */
    { 2,  10, 36,  85,  52, WC_TEXT, "Percentage:" },   /* WIN 124 TEXT 2 */
    { 3,  90, 36, 160,  52, WC_TEXT },                  /* JOB PERCENTAGE */
    { 5, 100, 64, 200,  86, WC_PUSHBUTTON, "Cancel" },  /* JOB STATUS CANCEL */
};

/* ----------------------------------------------------------------------
 * OPTIONS_WINDOW's "Project" tab (optnlib.c's createOptions() PROJECT_OP
 * case) stacks 4 sub-windows starting at groupXPos=150 (the real
 * WINWS/WIN32WS constant, confirmed in optnlib.c -- NOT this port's
 * earlier narrower placeholder): LOCAL_REMOTE_WINDOW(199) and
 * UNIT_WINDOW(200) side by side, then PRINT_SCALE_WINDOW(201) and
 * HELP_OPTION_WINDOW(217) stacked below spanning both. optnlib.c positions
 * each one automatically relative to the PREVIOUS window's actual size
 * (xvt_rect_get_width/height on the returned posRect), so only each
 * sub-window's OWN size/control layout needs to be right here -- the
 * stacking itself is stock app logic. Reconstructed from
 * noddy_help_lowercase/IMG00092.GIF (projop.htm's "Options" screenshot,
 * Option Selection = "Project"). */
static const DialogCtlPosition g_pos_199[] = {
    { 1,   0,  0, 180,  75, WC_GROUPBOX, "Calculations" },  /* LOCAL REMOTE GB */
    { 3,  15, 25, 170,  42, WC_RADIOBUTTON, "Local" },       /* LOCAL REMOTE LOCAL */
    { 2,  15, 48, 170,  65, WC_RADIOBUTTON, "Remote" },      /* LOCAL REMOTE REMOTE */
};
static const DialogCtlPosition g_pos_200[] = {
    { 3,   0,  0, 180,  75, WC_GROUPBOX, "Length Unit" },   /* UNIT WINDOW GB */
    { 1,  15, 25, 170,  42, WC_RADIOBUTTON, "Metres" },      /* UNIT METERS */
    { 2,  15, 48, 170,  65, WC_RADIOBUTTON, "Feet" },        /* UNIT FEET */
};
static const DialogCtlPosition g_pos_201[] = {
    { 4,   0,  0, 360,  60, WC_GROUPBOX, "Printing" },       /* PRINT SCALE GB */
    { 1,  15, 28, 160,  44, WC_TEXT, "Scale Printing by" },  /* PRINT SCALE LABEL1 */
    { 3, 165, 26, 205,  44, WC_EDIT },                       /* PRINT SCALE (value) */
    { 2, 210, 28, 260,  44, WC_TEXT, "times." },             /* PRINT SCALE LABEL2 */
};
static const DialogCtlPosition g_pos_217[] = {
    { 1,   0,  0, 360, 110, WC_GROUPBOX, "Help Viewer" },    /* HELP OPTION GROUPBOX */
    { 2,  15, 30,  80,  46, WC_TEXT, "Command" },            /* HELP VIEWER LABEL */
    { 3,  85, 26, 350,  44, WC_EDIT },                       /* HELP VIEWER (command line) */
    { 4,  15, 55, 350,  90, WC_TEXT,
      "Use %h to specify the html file on the command line" }, /* HELP VIEWER INFO */
};

/* ----------------------------------------------------------------------
 * OPTIONS_WINDOW's "Geology Display" tab (optnlib.c GEOLOGY_DISPLAY_OP
 * case): THREED_VIEW_WINDOW(185), CHAIR_WINDOW(184), NEWWIN_WINDOW(186),
 * BACKGROUND_WINDOW(214) stacked vertically. Reconstructed from
 * noddy_help_lowercase/IMG00087.GIF (geoldisp.htm, Option Selection =
 * "Geology Display"). */
static const DialogCtlPosition g_pos_185[] = {
    { 10,  0,  0, 360,  90, WC_GROUPBOX, "View Direction" },  /* THREED VIEW GB */
    {  1, 15, 20,  90,  36, WC_TEXT, "Declination" },          /* THREED DECL LABEL */
    {  4, 95, 20, 270,  36, WC_HSCROLL },                      /* THREED DECL BAR */
    {  7,275, 20, 320,  36, WC_EDIT },                         /* THREED DECL */
    {  2, 15, 44,  90,  60, WC_TEXT, "Azimuth" },              /* THREED AZIMUTH LABEL */
    {  5, 95, 44, 270,  60, WC_HSCROLL },                      /* THREED AZIMUTH BAR */
    {  8,275, 44, 320,  60, WC_EDIT },                         /* THREED AZIMUTH */
    {  3, 15, 68,  90,  84, WC_TEXT, "Scale" },                /* THREED SCALE LABEL */
    {  6, 95, 68, 270,  84, WC_HSCROLL },                      /* THREED SCALE BAR */
    {  9,275, 68, 320,  84, WC_EDIT },                         /* THREED SCALE */
    { 11,325, 68, 345,  84, WC_TEXT, "%" },                    /* THREED SCALE PERCENT LABEL */
};
static const DialogCtlPosition g_pos_184[] = {
    {  1,  0,  0, 360, 116, WC_GROUPBOX, "Chair Diagram" },   /* CHAIR WINDOW GROUPBOX 51 */
    { 11, 15, 20, 240,  36, WC_CHECKBOX, "Use Chair Diagram" }, /* CHAIR USE */
    {  5, 15, 44,  70,  60, WC_TEXT, "X Incr" },               /* CHAIR X LABEL */
    {  2, 75, 44, 270,  60, WC_HSCROLL },                      /* CHAIR X BAR */
    {  8,275, 44, 320,  60, WC_EDIT },                         /* CHAIR X */
    {  6, 15, 68,  70,  84, WC_TEXT, "Y Incr" },               /* CHAIR Y LABEL */
    {  3, 75, 68, 270,  84, WC_HSCROLL },                      /* CHAIR Y BAR */
    { 10,275, 68, 320,  84, WC_EDIT },                         /* CHAIR Y */
    {  7, 15, 92,  70, 108, WC_TEXT, "Z Incr" },               /* CHAIR Z LABEL */
    {  4, 75, 92, 270, 108, WC_HSCROLL },                      /* CHAIR Z BAR */
    {  9,275, 92, 320, 108, WC_EDIT },                         /* CHAIR Z */
};
static const DialogCtlPosition g_pos_186[] = {
    {  1,  0,  0, 230,  20, WC_CHECKBOX, "New window each time" }, /* NEWWIN EACHTIME */
};
static const DialogCtlPosition g_pos_214[] = {
    {  1,  0,  0, 160,  26, WC_PUSHBUTTON, "Background Color..." }, /* BACKGROUND BUTTON */
};

/* ----------------------------------------------------------------------
 * OPTIONS_WINDOW's "Block" tab (optnlib.c BLOCK_VIEW_OP case):
 * VIEW_NAME_WINDOW(203), ORIGIN_WINDOW(205), DIMENSION_WINDOW(206) stacked
 * vertically, then CUBE_SIZE_WINDOW(204) and FALSE_ORIGIN_WINDOW(208) side
 * by side, then USE_TOPOGRAPHY_WINDOW(207) below. Reconstructed from
 * noddy_help_lowercase/IMAGE4.GIF (blockop.htm, Option Selection =
 * "Block"). */
static const DialogCtlPosition g_pos_203[] = {
    { 3,   0,  0, 360,  45, WC_GROUPBOX, "View Selection" }, /* VIEW NAME GB */
    { 4,  15, 20, 220,  38, WC_LISTBUTTON },                 /* VIEW NAME LIST (dropdown) */
    { 1, 225, 20, 290,  38, WC_PUSHBUTTON, "New..." },       /* VIEW NAME NAME */
    { 2, 295, 20, 355,  38, WC_PUSHBUTTON, "Delete" },       /* VIEW NAME DELETE */
};
static const DialogCtlPosition g_pos_205[] = {
    { 1,   0,  0, 360,  90, WC_GROUPBOX, "Origin" },  /* ORIGIN GB */
    { 5,  15, 20,  70,  36, WC_TEXT, "X (E)" },        /* ORIGIN X LABEL */
    { 2,  75, 20, 270,  36, WC_HSCROLL },              /* ORIGIN X BAR */
    { 8, 275, 20, 320,  36, WC_EDIT },                 /* ORIGIN X */
    { 6,  15, 44,  70,  60, WC_TEXT, "Y (N)" },        /* ORIGIN Y LABEL */
    { 3,  75, 44, 270,  60, WC_HSCROLL },              /* ORIGIN Y BAR */
    {10, 275, 44, 320,  60, WC_EDIT },                 /* ORIGIN Y */
    { 7,  15, 68,  70,  84, WC_TEXT, "Z (Up)" },       /* ORIGIN Z LABEL */
    { 4,  75, 68, 270,  84, WC_HSCROLL },              /* ORIGIN Z BAR */
    { 9, 275, 68, 320,  84, WC_EDIT },                 /* ORIGIN Z */
};
static const DialogCtlPosition g_pos_206[] = {
    { 1,   0,  0, 360,  90, WC_GROUPBOX, "Dimensions" }, /* DIMENSION GB */
    { 5,  15, 20,  70,  36, WC_TEXT, "X (E)" },           /* DIMENSION X LABEL */
    { 2,  75, 20, 270,  36, WC_HSCROLL },                 /* DIMENSION X BAR */
    { 8, 275, 20, 320,  36, WC_EDIT },                    /* DIMENSION X */
    { 6,  15, 44,  70,  60, WC_TEXT, "Y (N)" },           /* DIMENSION Y LABEL */
    { 3,  75, 44, 270,  60, WC_HSCROLL },                 /* DIMENSION Y BAR */
    {10, 275, 44, 320,  60, WC_EDIT },                    /* DIMENSION Y */
    { 7,  15, 68,  70,  84, WC_TEXT, "Z (Up)" },          /* DIMENSION Z LABEL */
    { 4,  75, 68, 270,  84, WC_HSCROLL },                 /* DIMENSION Z BAR */
    { 9, 275, 68, 320,  84, WC_EDIT },                    /* DIMENSION Z */
};
static const DialogCtlPosition g_pos_204[] = {
    { 1,   0,  0, 175,  75, WC_GROUPBOX, "Cube Size" },   /* CUBE SIZE GB */
    /* Real XVT's E_CONTROL handler for these (csizewin.c) reads the EDIT
     * sub-event fields (v.edit.focus_change/.active), confirming they're
     * meant to be editable combos (predefined choices + free-typed
     * values), not closed-choice dropdowns -- user-requested. */
    { 4,  15, 25,  90,  41, WC_TEXT, "Geology" },          /* CUBE SIZE GEOLOGY LABEL */
    { 2,  95, 22, 160,  40, WC_LISTBUTTON, nullptr, false, false, false, true },  /* CUBE SIZE GEOLOGY */
    { 5,  15, 49,  90,  65, WC_TEXT, "Geophysics" },       /* CUBE SIZE GEOPHYSICS LABEL */
    { 3,  95, 46, 160,  64, WC_LISTBUTTON, nullptr, false, false, false, true },  /* CUBE SIZE GEOPHYSICS */
};
static const DialogCtlPosition g_pos_208[] = {
    { 1,   0,  0, 175,  75, WC_GROUPBOX, "False Origin" }, /* FALSE ORIGIN GB */
    { 4,  15, 25,  80,  41, WC_TEXT, "Easting" },           /* FALSE ORIGIN EASTING LABEL */
    { 2,  85, 22, 160,  40, WC_EDIT },                      /* FALSE ORIGIN EASTING */
    { 5,  15, 49,  80,  65, WC_TEXT, "Northing" },          /* FALSE ORIGIN NORTHING LABEL */
    { 3,  85, 46, 160,  64, WC_EDIT },                      /* FALSE ORIGIN NORTHING */
};
static const DialogCtlPosition g_pos_207[] = {
    { 1,   0,  0, 200,  20, WC_CHECKBOX, "Use Topography" }, /* USE TOPOGRAPHY */
};

/* ----------------------------------------------------------------------
 * OPTIONS_WINDOW's "Volume/Surface" tab (optnlib.c VOLUME_SURFACE_OP
 * case): VOLUME_WINDOW(216) then SURFACE_SELECTION_WINDOW(215) stacked
 * vertically. Reconstructed from noddy_help_lowercase/IMG00088.GIF
 * (volsurf.htm, Option Selection = "Volume/Surface"). */
static const DialogCtlPosition g_pos_216[] = {
    { 2,   0,  0, 360,  55, WC_GROUPBOX, "Default Block Diagram" }, /* VOLUME GB */
    { 1,  15, 22, 220,  42, WC_PUSHBUTTON, "Edit Volume Options..." }, /* VOLUME EDIT */
};
static const DialogCtlPosition g_pos_215[] = {
    { 5,   0,  0, 360, 230, WC_GROUPBOX, "Surface Selection" }, /* SURFACE SELECTION GB */
    { 4,  15, 25, 100,  41, WC_TEXT, "Output Type:" },           /* SURFACE SELECTION TYPE LABEL */
    { 3, 105, 22, 240,  40, WC_LISTBUTTON },                     /* SURFACE SELECTION TYPE (dropdown) */
    { 2,  15, 48, 180,  64, WC_CHECKBOX, "All Surfaces" },       /* SURFACE SELECTION ALL */
    /* optnlib.c reads this with xvt_list_is_sel() per-row, i.e. an
     * arbitrary subset of layers is meant to be selectable at once --
     * user-reported as "can only select one strat layer at a time". */
    { 1,  15, 70, 345, 220, WC_LBOX, nullptr, false, false, true },  /* SURFACE SELECTION LIST */
};

/* ----------------------------------------------------------------------
 * OPTIONS_WINDOW's "Movie" tab (optnlib.c MOVIE_OP case): just
 * MOVIE_WINDOW(209). Reconstructed from noddy_help_lowercase/IMG00089.GIF
 * (movieop.htm, Option Selection = "Movie"). */
static const DialogCtlPosition g_pos_209[] = {
    {  8,   0,  0, 360,  80, WC_GROUPBOX, "Generation" },              /* MOVIE GENERATION GB */
    { 10,  15, 25, 110,  41, WC_TEXT, "Movie Type:" },                  /* MOVIE GENERATION TYPE LABEL */
    {  9, 115, 22, 270,  40, WC_LISTBUTTON },                           /* MOVIE GENERATION TYPE */
    {  1,  15, 50, 220,  66, WC_TEXT, "Number of Frames per Event:" },  /* MOVIE GENERATION FRAMES LABEL */
    {  2, 225, 50, 260,  66, WC_EDIT },                                 /* MOVIE GENERATION FRAMES */
    { 11,   0, 85, 360, 165, WC_GROUPBOX, "Replay" },                   /* MOVIE REPLAY GB */
    {  3,  15,105, 300, 121, WC_TEXT, "Speed of On Screen Replay (Delay):" }, /* MOVIE REPLAY SPEED LABEL */
    {  4,  15,128, 180, 144, WC_HSCROLL },                              /* MOVIE REPLAY SPEED BAR */
    {  7, 185,128, 225, 144, WC_EDIT },                                 /* MOVIE REPLAY SPEED */
    {  5,  15,146,  50, 160, WC_TEXT, "Fast" },                         /* MOVIE REPLAY FAST LABEL */
    {  6, 150,146, 190, 160, WC_TEXT, "Slow" },                         /* MOVIE REPLAY SLOW LABEL */
};

/* ----------------------------------------------------------------------
 * ROCK_WINDOW (148, rockwin.c) -- the "Rock Properties" dialog, opened via
 * "Edit..." on any stratigraphy/rock layer. Reconstructed from a real
 * screenshot: Geology (Name/Alter/Width/Height), Density, Susceptibility
 * (Anisotropy + Magnetic Susceptibility X/Y/Z with paired Dip/DipDir/
 * Pitch), Remanence (Declination/Inclination/Intensity + Define
 * Colour... button), OK/Help/Cancel.
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_148[] = {
    /* Checkboxes/labels positioned right after a groupbox's own top edge
     * (previously ~15px gap) rendered overlapping the groupbox's title
     * text -- controls here are flat siblings of the dialog, not truly
     * parented inside the groupbox widget, so nothing reserves the
     * groupbox's own title-text height automatically; matches a user
     * screenshot circling the Alter checkbox (too cramped next to the
     * dropdown) and the Anisotropy/Remanence checkboxes (overlapping
     * their groupbox titles). Widened Alter and added top clearance. */
    { 32,  8,  8, 205, 110, WC_GROUPBOX, "Geology" },
    {  5, 15, 25,  50,  41, WC_TEXT, "Name" },
    { 31, 55, 22, 148,  40, WC_LISTBUTTON },
    { 36,152, 25, 202,  41, WC_CHECKBOX, "Alter" },
    { 37, 15, 50, 100,  66, WC_TEXT, "Width" },
    { 38,105, 48, 195,  66, WC_EDIT },
    {  2, 15, 78, 100,  94, WC_TEXT, "Height" },
    {  1,105, 76, 195,  94, WC_EDIT },
    {  8,215,  8, 410,  70, WC_GROUPBOX, "Density" },
    {  4,225, 30, 275,  46, WC_TEXT, "Density" },
    {  3,280, 28, 395,  46, WC_EDIT },
    { 10,  8,115, 205, 270, WC_GROUPBOX, "Susceptibility" },
    { 11, 15,138, 195, 154, WC_CHECKBOX, "Anisotropy" },
    { 16, 15,158, 195, 172, WC_TEXT, "Magnetic Susceptibility" },
    /* Dip/Dip Dir/Pitch labels previously all shared the same 25px-wide
     * box (sized only for the shortest, "Dip") -- "Dip Dir" and "Pitch"
     * were clipped/overlapping their own value field, matching a user
     * screenshot showing a garbled "Pitcl" and a blank-looking Dip Dir
     * row. Widened the label column and narrowed the value fields to fit
     * within the Susceptibility groupbox's right edge (205). */
    { 19, 15,176,  35, 192, WC_TEXT, "X" },
    { 15, 40,174, 105, 192, WC_EDIT },
    { 28,108,176, 160, 192, WC_TEXT, "Dip" },
    { 25,163,174, 195, 192, WC_EDIT },
    { 20, 15,198,  35, 214, WC_TEXT, "Y" },
    { 17, 40,196, 105, 214, WC_EDIT },
    { 29,108,198, 160, 214, WC_TEXT, "Dip Dir" },
    { 26,163,196, 195, 214, WC_EDIT },
    { 21, 15,220,  35, 236, WC_TEXT, "Z" },
    { 18, 40,218, 105, 236, WC_EDIT },
    { 30,108,220, 160, 236, WC_TEXT, "Pitch" },
    { 27,163,218, 195, 236, WC_EDIT },
    { 24,215, 80, 410, 270, WC_GROUPBOX, "Remanence" },
    { 12,225,103, 395, 119, WC_CHECKBOX, "Remanence" },
    { 13,225,128, 290, 144, WC_TEXT, "Declination" },
    { 22,295,126, 390, 144, WC_EDIT },
    { 14,225,150, 290, 166, WC_TEXT, "Inclination" },
    { 23,295,148, 390, 166, WC_EDIT },
    {  9,225,172, 290, 188, WC_TEXT, "Intensity" },
    {  6,295,170, 390, 188, WC_EDIT },
    {  7,225,200, 390, 222, WC_PUSHBUTTON, "Define Colour..." },
    { 33,100,280, 180, 302, WC_PUSHBUTTON, "OK" },
    { 35,190,280, 270, 302, WC_PUSHBUTTON, "Help..." },
    { 34,280,280, 360, 302, WC_PUSHBUTTON, "Cancel" },
};

/* ----------------------------------------------------------------------
 * ROCK_DATABASE_WIN (135) -- "Edit Rock Database" dialog. Reconstructed
 * from a real screenshot. ctlId 1/9/10 are generic auto-named pushbuttons
 * in the registry with no semantic label recovered -- assumed OK/Help/
 * Cancel by numbering convention (ctl1 first = OK, matching every other
 * dialog in this codebase; 9/10 = Help/Cancel, matching the visible L-to-R
 * order in the screenshot) since no source confirms it directly. ctlId 8
 * ("Edit Rock...") is WC_EDIT in the registry (misclassified, same
 * "PUSHBUTTON substring missing from the name" issue as other dialogs).
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_135[] = {
    { 2,   8,  8, 175, 270, WC_GROUPBOX, "Current Database" },
    { 3,  15, 25, 168, 262, WC_LBOX },
    { 6, 185, 10, 325,  32, WC_PUSHBUTTON, "New Rock" },
    { 7, 185, 40, 325,  62, WC_PUSHBUTTON, "Delete Rock" },
    { 8, 185, 78, 325, 100, WC_PUSHBUTTON, "Edit Rock ..." },
    { 4, 185,120, 325, 142, WC_PUSHBUTTON, "Import ..." },
    { 5, 185,150, 325, 172, WC_PUSHBUTTON, "Export ..." },
    { 1,  95,285, 175, 307, WC_PUSHBUTTON, "OK" },
    /* ctlId 9 is ROCK_DATABASE_WIN_PUSHBUTTON_39, whose E_CONTROL case in
     * rockprop.c is really "Cancel"; ctlId 10 is PUSHBUTTON_40, really
     * "Help...". These were previously mislabeled the other way round, so
     * clicking the button labeled "Help..." actually closed the dialog
     * (Cancel) and vice versa -- user-reported as "help and cancel button
     * functions switched". */
    { 9, 185,285, 255, 307, WC_PUSHBUTTON, "Cancel" },
    { 10,265,285, 335, 307, WC_PUSHBUTTON, "Help..." },
};

/* ----------------------------------------------------------------------
 * LAYER_DISPLAY_WINDOW (177, blklayop.c) -- the "Display Type" popup shown
 * on right/middle-click in a Block Diagram window (nodwork2.c's
 * createPopupWindow(..., LAYER_DISPLAY_WINDOW, ...), see the E_MOUSE_DOWN
 * handler). Reconstructed from a real screenshot: Display Type dropdown,
 * Layers list + "All Layers" checkbox, Lut Editing (Stretch Over Range,
 * Reset/Edit.../Calc Real Values, Range/Max/Min), OK/Help/Cancel. The
 * per-layer colour swatch column visible in the screenshot is custom-
 * drawn (no corresponding registry ctlId), not reconstructed here.
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_177[] = {
    /* ctl9 MUST be a real WC_GROUPBOX, not just a text label: nodLib2.c's
     * getBlockImageColorBarPos() calls xvt_vobj_get_outer_rect() on it
     * directly to compute the per-layer colour-scale bar's position
     * (`barPos->left = displayTypeRect.right + 5`, `barPos->right =
     * layerListRect.left - 5`). The real bug behind the solid-green
     * background turned out to be xvt_dwin_draw_rect/oval/roundrect
     * reading a single COLOR field that was ALSO being used as the
     * window's ATTR_BACK_COLOR (see qt_compat/xvt_compat.cpp's
     * XvtObj::winBackColor split) -- drawBlockImageColorScale's per-layer
     * xvt_dwin_set_draw_ctools calls left that field holding the LAST
     * layer's colour, which the next E_UPDATE's xvt_dwin_clear then used
     * to fill the WHOLE window. Fixed at the port layer, not here.
     * Independently, the 10px gap this groupbox's right edge (165) left
     * before the Layers groupbox's left edge (originally 175) collapsed
     * getBlockImageColorBarPos()'s computed strip to zero width after its
     * +5/-5 margins (170 to 170) -- matches the separate "colour strip is
     * too narrow" complaint. Widened the gap to 30px (Layers now starts
     * at 195) so the strip has a real ~20px width; dialog width bumped
     * 405->425 to keep the Layers list/checkbox from being squeezed. */
    {  9,  8,  8, 165,  50, WC_GROUPBOX, "Display Type" },
    {  2, 15, 26, 158,  44, WC_LISTBUTTON },
    {  7,195,  8, 415, 145, WC_GROUPBOX, "Layers" },
    {  8,203, 25, 405, 41, WC_CHECKBOX, "All Layers (Solid Block)" },
    /* nodLib2.c's updateBlockImageOptions() reads this with xvt_list_is_
     * sel() per-row (`allLayers || xvt_list_is_sel(layerList, layer)`) --
     * an arbitrary subset of layers is meant to be selectable when "All
     * Layers" is unchecked. */
    {  1,203, 45, 405, 138, WC_LBOX, nullptr, false, false, true },
    { 10,  8, 50, 165, 270, WC_GROUPBOX, "Lut Editing" },
    { 19, 15, 70, 158,  88, WC_LISTBUTTON },
    {  3, 15, 95,  85, 115, WC_PUSHBUTTON, "Reset" },
    {  4, 90, 95, 158, 115, WC_PUSHBUTTON, "Edit..." },
    {  5, 15,122, 158, 142, WC_PUSHBUTTON, "Calc Real Values" },
    {  6, 15,150,  60, 166, WC_TEXT, "Range:" },
    { 18, 65,150, 158, 166, WC_TEXT },
    { 11, 15,172,  50, 188, WC_TEXT, "Max" },
    { 13, 55,170, 158, 188, WC_EDIT },
    { 12, 15,195,  50, 211, WC_TEXT, "Min" },
    { 14, 55,193, 158, 211, WC_EDIT },
    { 15,150,285, 230, 307, WC_PUSHBUTTON, "OK" },
    { 16,240,285, 320, 307, WC_PUSHBUTTON, "Help..." },
    { 17,330,285, 395, 307, WC_PUSHBUTTON, "Cancel" },
};

/* ----------------------------------------------------------------------
 * PROFILE_WINDOW (105, profile.c) -- the point-editing graph opened via
 * "Show Profile..."/"Edit..." buttons in several event Form tabs (Fold's
 * Wave/Amp/Cycl, a stratigraphy layer's density, etc). Reconstructed from
 * a real screenshot: a large custom-drawn graph canvas + zoom scrollbar,
 * "Properties" (Line/Curve/Square join-style radios), a "Current Graph:"
 * selector, X/Y cursor readouts, Delete Point/Delete All, OK/Help/Cancel.
 * ctlId 1/2/3 are generic auto-named pushbuttons, assumed OK/Help/Cancel
 * by numbering + visible L-to-R order convention (no source confirms
 * this directly). ctlId 15 ("Options..." button) position is a rough
 * guess -- not clearly identifiable in the reference screenshot.
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_105[] = {
    { 10,  8,  8, 330, 270, WC_GROUPBOX, "" },  /* graph canvas placeholder (custom-drawn) */
    { 16,  8,272, 330, 288, WC_HSCROLL },
    { 15,438,  8, 455,  25, WC_PUSHBUTTON, "..." },
    {  4,340,  8, 455, 110, WC_GROUPBOX, "Properties" },
    {  5,348, 25, 450,  41, WC_RADIOBUTTON, "Line [Alt.]" },
    {  6,348, 48, 450,  64, WC_RADIOBUTTON, "Curve [Alt.]" },
    {  7,348, 71, 450,  87, WC_RADIOBUTTON, "Square [Rep.]" },
    { 11,340,120, 455, 136, WC_TEXT, "Current Graph:" },
    { 12,340,138, 455, 156, WC_LISTBUTTON },
    { 13,340,170, 455, 186, WC_TEXT },
    { 14,340,188, 455, 204, WC_TEXT },
    {  8,  8,295, 120, 317, WC_PUSHBUTTON, "Delete Point" },
    {  9,128,295, 240, 317, WC_PUSHBUTTON, "Delete All" },
    {  1,250,295, 320, 317, WC_PUSHBUTTON, "OK" },
    {  2,330,295, 395, 317, WC_PUSHBUTTON, "Help..." },
    {  3,400,295, 460, 317, WC_PUSHBUTTON, "Cancel" },
};

/* ----------------------------------------------------------------------
 * FILE_FORMAT_WINDOW (182, filefwin.c) -- "Export Tabulated Columns..."
 * file-format dialog: Property dropdown, Order (X/Y/Z Dimension: 1st/2nd/
 * 3rd radio rows + Single Value per Line checkbox), Direction (X/Y/Z
 * Dimension: two-way radio rows), OK/Help/Cancel. Reconstructed from a
 * real screenshot. Real XVT/Qt radio buttons are exclusive by shared
 * parent widget when not otherwise grouped -- this dialog has 6
 * independent radio groups (3 in Order, 3 in Direction), so each row is
 * given its own radioGroup id (see DialogCtlPosition::radioGroup) to keep
 * them from all becoming one giant mutually-exclusive set.
 * ---------------------------------------------------------------------- */
static const DialogCtlPosition g_pos_182[] = {
    { 28,   8,   8, 220,  50, WC_GROUPBOX, "Property" },
    {  27, 15,  25, 210,  43, WC_LISTBUTTON },
    {  24,230,   8, 320,  26, WC_PUSHBUTTON, "OK" },
    {  26,230,  30, 320,  48, WC_PUSHBUTTON, "Help..." },
    {  25,230,  52, 320,  70, WC_PUSHBUTTON, "Cancel" },

    {  1,   8,  58, 320, 195, WC_GROUPBOX, "Order" },
    {  2,  15,  78, 100,  94, WC_TEXT, "X Dimension:" },
    {  5, 105,  78, 155,  94, WC_RADIOBUTTON, "1st", false, false, false, false, 1 },
    {  6, 160,  78, 210,  94, WC_RADIOBUTTON, "2nd", false, false, false, false, 1 },
    {  7, 215,  78, 265,  94, WC_RADIOBUTTON, "3rd", false, false, false, false, 1 },
    {  3,  15, 100, 100, 116, WC_TEXT, "Y Dimension:" },
    {  8, 105, 100, 155, 116, WC_RADIOBUTTON, "1st", false, false, false, false, 2 },
    {  9, 160, 100, 210, 116, WC_RADIOBUTTON, "2nd", false, false, false, false, 2 },
    { 10, 215, 100, 265, 116, WC_RADIOBUTTON, "3rd", false, false, false, false, 2 },
    {  4,  15, 122, 100, 138, WC_TEXT, "Z Dimension:" },
    { 11, 105, 122, 155, 138, WC_RADIOBUTTON, "1st", false, false, false, false, 3 },
    { 12, 160, 122, 210, 138, WC_RADIOBUTTON, "2nd", false, false, false, false, 3 },
    { 13, 215, 122, 265, 138, WC_RADIOBUTTON, "3rd", false, false, false, false, 3 },
    { 29,  15, 152, 260, 168, WC_CHECKBOX, "Single Value per Line" },

    { 14,   8, 202, 320, 300, WC_GROUPBOX, "Direction" },
    { 15,  15, 222, 100, 238, WC_TEXT, "X Dimension:" },
    { 18, 105, 222, 205, 238, WC_RADIOBUTTON, "East - West", false, false, false, false, 4 },
    { 19, 210, 222, 310, 238, WC_RADIOBUTTON, "West - East", false, false, false, false, 4 },
    { 16,  15, 244, 100, 260, WC_TEXT, "Y Dimension:" },
    { 20, 105, 244, 205, 260, WC_RADIOBUTTON, "North-South", false, false, false, false, 5 },
    { 21, 210, 244, 310, 260, WC_RADIOBUTTON, "South-North", false, false, false, false, 5 },
    { 17,  15, 266, 100, 282, WC_TEXT, "Z Dimension:" },
    { 22, 105, 266, 205, 282, WC_RADIOBUTTON, "Up-Down", false, false, false, false, 6 },
    { 23, 210, 266, 310, 282, WC_RADIOBUTTON, "Down-Up", false, false, false, false, 6 },
};

extern const DialogPositionEntry g_dialogPositions[] = {
    { 137, 500, 335, g_pos_137, (int)(sizeof(g_pos_137)/sizeof(g_pos_137[0])) },
    { 152, 210, 105, g_pos_152, (int)(sizeof(g_pos_152)/sizeof(g_pos_152[0])) },
    { 153, 210, 115, g_pos_153, (int)(sizeof(g_pos_153)/sizeof(g_pos_153[0])) },
    { 143, 270, 142, g_pos_143, (int)(sizeof(g_pos_143)/sizeof(g_pos_143[0])) },
    { 165, 270, 115, g_pos_165, (int)(sizeof(g_pos_165)/sizeof(g_pos_165[0])) },
    { 144, 270,  62, g_pos_144, (int)(sizeof(g_pos_144)/sizeof(g_pos_144[0])) },
    { 154, 270, 157, g_pos_154, (int)(sizeof(g_pos_154)/sizeof(g_pos_154[0])) },
    { 157, 270, 132, g_pos_157, (int)(sizeof(g_pos_157)/sizeof(g_pos_157[0])) },
    { 159, 270, 132, g_pos_159, (int)(sizeof(g_pos_159)/sizeof(g_pos_159[0])) },
    { 169, 270, 262, g_pos_169, (int)(sizeof(g_pos_169)/sizeof(g_pos_169[0])) },
    { 162, 270,  82, g_pos_162, (int)(sizeof(g_pos_162)/sizeof(g_pos_162[0])) },
    { 139, 270, 110, g_pos_139, (int)(sizeof(g_pos_139)/sizeof(g_pos_139[0])) },
    /* Width was 470 -- nodGraph.c's drawSelectedColorGrid() draws its 3
     * "shade preview" blocks (colour Up/Selected/Down) starting 5px to the
     * right of the "Custom Colour" groupbox's outer rect (right edge 460),
     * each ~colorSize (~43px, groupbox height/3) wide, i.e. out to ~x=508 --
     * past the old 470-wide window, so they were almost entirely clipped
     * off the visible dialog instead of appearing beside the groupbox as
     * intended (user-reported as needing repositioning so they don't
     * overlap/get lost against the Custom Colour widgets). Widened to give
     * them their own unclipped space; every other control stays within
     * x<=460 so nothing else moves. */
    { 104, 520, 315, g_pos_104, (int)(sizeof(g_pos_104)/sizeof(g_pos_104[0])), "Define Colour" },
    { 116, 280, 250, g_pos_116, (int)(sizeof(g_pos_116)/sizeof(g_pos_116[0])) },
    { 140, 205,  54, g_pos_140, (int)(sizeof(g_pos_140)/sizeof(g_pos_140[0])) },
    { 168, 205,  20, g_pos_168, (int)(sizeof(g_pos_168)/sizeof(g_pos_168[0])) },
    { 141, 205,  40, g_pos_141, (int)(sizeof(g_pos_141)/sizeof(g_pos_141[0])) },
    { 142, 205,  40, g_pos_142, (int)(sizeof(g_pos_142)/sizeof(g_pos_142[0])) },
    { 145, 205,  40, g_pos_145, (int)(sizeof(g_pos_145)/sizeof(g_pos_145[0])) },
    { 146, 205,  94, g_pos_146, (int)(sizeof(g_pos_146)/sizeof(g_pos_146[0])) },
    { 155, 205, 101, g_pos_155, (int)(sizeof(g_pos_155)/sizeof(g_pos_155[0])) },
    { 156, 205, 118, g_pos_156, (int)(sizeof(g_pos_156)/sizeof(g_pos_156[0])) },
    { 158, 205,  39, g_pos_158, (int)(sizeof(g_pos_158)/sizeof(g_pos_158[0])) },
    { 160, 205,  52, g_pos_160, (int)(sizeof(g_pos_160)/sizeof(g_pos_160[0])) },
    { 161, 205,  54, g_pos_161, (int)(sizeof(g_pos_161)/sizeof(g_pos_161[0])) },
    { 164, 205,  24, g_pos_164, (int)(sizeof(g_pos_164)/sizeof(g_pos_164[0])) },
    { 163, 205,  50, g_pos_163, (int)(sizeof(g_pos_163)/sizeof(g_pos_163[0])) },
    { 183, 630, 420, g_pos_183, (int)(sizeof(g_pos_183)/sizeof(g_pos_183[0])) },
    { 124, 300, 100, g_pos_124, (int)(sizeof(g_pos_124)/sizeof(g_pos_124[0])) },
    { 199, 180,  75, g_pos_199, (int)(sizeof(g_pos_199)/sizeof(g_pos_199[0])) },
    { 200, 180,  75, g_pos_200, (int)(sizeof(g_pos_200)/sizeof(g_pos_200[0])) },
    { 201, 360,  60, g_pos_201, (int)(sizeof(g_pos_201)/sizeof(g_pos_201[0])) },
    { 217, 360, 110, g_pos_217, (int)(sizeof(g_pos_217)/sizeof(g_pos_217[0])) },
    { 185, 360,  90, g_pos_185, (int)(sizeof(g_pos_185)/sizeof(g_pos_185[0])) },
    { 184, 360, 116, g_pos_184, (int)(sizeof(g_pos_184)/sizeof(g_pos_184[0])) },
    { 186, 230,  20, g_pos_186, (int)(sizeof(g_pos_186)/sizeof(g_pos_186[0])) },
    { 214, 160,  26, g_pos_214, (int)(sizeof(g_pos_214)/sizeof(g_pos_214[0])) },
    { 203, 360,  45, g_pos_203, (int)(sizeof(g_pos_203)/sizeof(g_pos_203[0])) },
    { 205, 360,  90, g_pos_205, (int)(sizeof(g_pos_205)/sizeof(g_pos_205[0])) },
    { 206, 360,  90, g_pos_206, (int)(sizeof(g_pos_206)/sizeof(g_pos_206[0])) },
    { 204, 175,  75, g_pos_204, (int)(sizeof(g_pos_204)/sizeof(g_pos_204[0])) },
    { 208, 175,  75, g_pos_208, (int)(sizeof(g_pos_208)/sizeof(g_pos_208[0])) },
    { 207, 200,  20, g_pos_207, (int)(sizeof(g_pos_207)/sizeof(g_pos_207[0])) },
    { 216, 360,  55, g_pos_216, (int)(sizeof(g_pos_216)/sizeof(g_pos_216[0])) },
    { 215, 360, 230, g_pos_215, (int)(sizeof(g_pos_215)/sizeof(g_pos_215[0])) },
    { 209, 360, 165, g_pos_209, (int)(sizeof(g_pos_209)/sizeof(g_pos_209[0])) },
    { 148, 420, 310, g_pos_148, (int)(sizeof(g_pos_148)/sizeof(g_pos_148[0])) },
    { 135, 340, 320, g_pos_135, (int)(sizeof(g_pos_135)/sizeof(g_pos_135[0])) },
    { 177, 425, 315, g_pos_177, (int)(sizeof(g_pos_177)/sizeof(g_pos_177[0])), "Display Type" },
    { 105, 465, 325, g_pos_105, (int)(sizeof(g_pos_105)/sizeof(g_pos_105[0])) },
    { 182, 335, 305, g_pos_182, (int)(sizeof(g_pos_182)/sizeof(g_pos_182[0])), "File Format" },
};
extern const int g_dialogPositionsCount =
    (int)(sizeof(g_dialogPositions) / sizeof(g_dialogPositions[0]));
