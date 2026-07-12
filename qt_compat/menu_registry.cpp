/* ============================================================================
 * XVT TASK_MENUBAR menu tree, parsed DIRECTLY from the real XVT-Design-
 * generated Windows resource file (100 MENU block in noddy.rc, supplied
 * by the user from a separately-archived commercial-release source tree)
 * -- see scratchpad/gen_menu_from_rc.py, AUTO-GENERATED, do not hand-edit.
 * This replaces an earlier version that reconstructed the tree from
 * mainMenu.c's case-comment text plus guessed intermediate labels (some
 * of which turned out wrong, e.g. "Vertical Derivatives > First/Second"
 * was guessed as "Borehole Survey > Generated/Imported") -- this version
 * is ground truth: exact labels, exact nesting, exact tag values, all
 * taken directly from the resource file rather than inferred.
 *
 * Leaf tags are literal integers copied straight from the .rc file (no
 * macro dependency needed, unlike the previous TASK_MENUBAR_<path> form).
 * Node paths are synthetic dotted position strings, not resource IDs.
 * ============================================================================
 */
#include "xvt_types.h"
#include "nodInc.h"

struct MenuNodeEntry { const char *path; const char *parentPath; const char *label; long tag; bool isLeaf; bool initiallyDisabled = false; };

extern const MenuNodeEntry g_menuTree[] = {
    { "1", nullptr, "File", 0, false },
    { "1.1", "1", "New History", 1144, true },
    { "1.2", "1", "Read History...", 1033, true },
    { "1.3", "1", "Save History", 1082, true },
    { "1.4", "1", "Save History As...", 1026, true },
    { "1.5", "1", "Save as Defaults", 1098, true },
    { "1.6", "1", "Load Reference Images", 0, false },
    /* Not ported/functional -- disabled per explicit request rather than
     * left silently broken. */
    { "1.6.1", "1.6", "Geological...", 1129, true, true },
    { "1.6.2", "1.6", "Gravity...", 1130, true },
    { "1.6.3", "1.6", "Magnetics...", 1131, true },
    { "1.7", "1", "Movie", 0, false },
    { "1.7.1", "1.7", "Create...", 1083, true },
    { "1.7.2", "1.7", "Load...", 1140, true },
    { "1.8", "1", "Read Picture...", 1030, true },
    { "1.9", "1", "Save Picture...", 1031, true },
    { "1.10", "1", "Save Orientations...", 1032, true },
    /* [Qt port ADDITION] todo.txt #43/#56 -- NEW menu item, not part of
     * the real XVT-Design .rc file this tree was otherwise parsed from
     * (see TASK_MENUBAR_GEOLOGY_SAVE_SURFACE_ORIENTATIONS, nodInc.h,
     * for why tag 1200 -- name kept despite moving out of Geology).
     * Originally added under Geology (#43), moved here per #56 (user:
     * "Move 'Save surface Orientations' menu item to File menu just
     * below 'save orintations'"). Dispatched in mainMenu.c, calculated
     * in lineEvnt.c's saveSurfaceOrientations(). */
    { "1.10.1", "1", "Save Surface Orientations...", 1200, true },
    { "1.11", "1", "Save Profile...", 1093, true },
    { "1.12", "1", "Page Setup...", 1024, true },
    { "1.13", "1", "Print Page", 1071, true },
    { "1.14", "1", "About Noddy...", 1074, true },
    { "1.15", "1", "Exit", 1023, true },
    { "2", nullptr, "Edit", 0, false },
    { "2.1", "2", "Undo", 1009, true },
    { "2.2", "2", "Cut", 1010, true },
    { "2.3", "2", "Copy", 1075, true },
    { "2.4", "2", "Paste", 1008, true },
    { "2.5", "2", "Clear", 1015, true },
    { "2.6", "2", "Duplicate", 1020, true },
    { "2.7", "2", "Select All", 1021, true },
    { "2.8", "2", "Tidy Window", 1061, true },
    { "2.9", "2", "Project Options...", 1148, true },
    { "2.10", "2", "Block Options...", 1150, true },
    { "2.11", "2", "Movie Options...", 1151, true },
    { "2.12", "2", "Geology Display Options...", 1046, true },
    { "2.13", "2", "Geology Section/Borehole Options...", 1152, true },
    { "2.14", "2", "Geology Volume/Surface Options...", 1153, true },
    { "2.15", "2", "Geophysics Calculation Options...", 1145, true },
    { "2.16", "2", "Geophysics Survey Options...", 1146, true },
    { "2.17", "2", "Geophysics Display Options...", 1147, true },
    { "2.18", "2", "Window Options...", 1149, true },
    { "3", nullptr, "Geology", 0, false },
    { "3.1", "3", "Block Diagram", 1027, true },
    { "3.2", "3", "Map", 0, false },
    { "3.2.1", "3.2", "Solid Colours...", 1053, true },
    { "3.2.2", "3.2", "Lines...", 1054, true },
    { "3.3", "3", "Topo Map", 1029, true },
    { "3.4", "3", "Section", 0, false },
    { "3.4.1", "3.4", "Solid Colours...", 1055, true },
    { "3.4.2", "3.4", "Lines...", 1056, true },
    { "3.5", "3", "Stratigraphic Column", 1070, true },
    { "3.6", "3", "Borehole", 0, false },
    { "3.6.1", "3.6", "Generate", 0, false },
    { "3.6.1.1", "3.6.1", "3D", 1113, true },
    { "3.6.1.2", "3.6.1", "Schematic", 1114, true },
    { "3.6.2", "3.6", "Import", 0, false },
    { "3.6.2.1", "3.6.2", "3D...", 1115, true },
    { "3.6.2.2", "3.6.2", "Schematic...", 1116, true },
    { "3.7", "3", "3D Topo", 1049, true },
    { "3.8", "3", "3D Triangulation", 1050, true },
    { "3.9", "3", "Plot Orientations", 1069, true },
    { "4", nullptr, "Geophysics", 0, false },
    { "4.1", "4", "Calculate Anomalies", 0, false },
    { "4.1.1", "4.1", "Anomalies...", 1043, true },
    { "4.1.2", "4.1", "Block and Anomalies...", 1044, true },
    { "4.1.3", "4.1", "Anomalies from Block...", 1045, true },
    { "4.1.4", "4.1", "Borehole", 0, false },
    { "4.1.4.1", "4.1.4", "Generated", 1118, true },
    { "4.1.4.2", "4.1.4", "Imported...", 1119, true },
    { "4.1.5", "4.1", "XYZ Point Data", 0, false },
    { "4.1.5.1", "4.1.5", "Import From File...", 1091, true },
    { "4.1.5.2", "4.1.5", "Generate...", 1092, true },
    { "4.2", "4", "Vertical Derivatives", 0, false },
    { "4.2.1", "4.2", "First", 0, false },
    { "4.2.1.1", "4.2.1", "Gravity...", 1123, true },
    { "4.2.1.2", "4.2.1", "Magnetics...", 1124, true },
    { "4.2.2", "4.2", "Second", 0, false },
    { "4.2.2.1", "4.2.2", "Gravity...", 1125, true },
    { "4.2.2.2", "4.2.2", "Magnetics...", 1126, true },
    { "4.3", "4", "Difference with Reference", 0, false },
    { "4.3.1", "4.3", "Gravity...", 1135, true },
    { "4.3.2", "4.3", "Magnetics...", 1136, true },
    { "4.4", "4", "Display Image", 0, false },
    { "4.4.1", "4.4", "Gravity...", 1063, true },
    { "4.4.2", "4.4", "Magnetics...", 1064, true },
    { "4.4.3", "4.4", "Magnetics X...", 1102, true },
    { "4.4.4", "4.4", "Magnetics Y...", 1103, true },
    { "4.4.5", "4.4", "Magnetics Z...", 1104, true },
    { "4.5", "4", "Display Profile", 0, false },
    { "4.5.1", "4.5", "Gravity...", 1067, true },
    { "4.5.2", "4.5", "Magnetics...", 1068, true },
    { "5", nullptr, "Tools", 0, false },
    { "5.1", "5", "Import Block", 0, false },
    { "5.1.1", "5.1", "Dicer...", 1088, true },
    { "5.1.2", "5.1", "Vulcan...", 1087, true },
    { "5.1.3", "5.1", "Old Noddy Block...", 1089, true },
    { "5.2", "5", "Import Image", 0, false },
    { "5.2.1", "5.2", "ASCII...", 1132, true },
    { "5.2.2", "5.2", "Column...", 1127, true },
    { "5.3", "5", "Export Block", 0, false },
    { "5.3.1", "5.3", "Noddy Block Model...", 1072, true },
    { "5.3.2", "5.3", "Tabulated Columns...", 1022, true },
    { "5.3.3", "5.3", "DXF - 3D Faces", 1105, true },
    { "5.3.4", "5.3", "DXF - Polylines", 1106, true },
    { "5.3.5", "5.3", "Vulcan", 1107, true },
    { "5.4", "5", "Export Image", 0, false },
    { "5.4.1", "5.4", "ERMappper", 0, false },
    { "5.4.1.1", "5.4.1", "Gravity...", 1095, true },
    { "5.4.1.2", "5.4.1", "Magnetics...", 1096, true },
    { "5.4.1.3", "5.4.1", "Line Map...", 1100, true },
    { "5.4.2", "5.4", "Geosoft", 0, false },
    { "5.4.2.1", "5.4.2", "Gravity...", 1109, true },
    { "5.4.2.2", "5.4.2", "Magnetics...", 1110, true },
    { "5.5", "5", "Edit Rock Database...", 1099, true },
    { "6", nullptr, "Windows", 0, false },
    { "6.1", "6", "Cascade", 1079, true },
    { "6.2", "6", "Tile", 1078, true },
    { "6.3", "6", "Default Size", 1084, true },
    { "6.4", "6", "Remove All", 1080, true },
    { "7", nullptr, "Help", 0, false },
    { "7.1", "7", "Contents", 1154, true },
    { "6.99", "6", "Legend", TASK_MENUBAR_77_18, true },
};
extern const int g_menuTreeCount = (int)(sizeof(g_menuTree)/sizeof(g_menuTree[0]));
