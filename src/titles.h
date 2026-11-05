/* ============================================================================
 * [XVT -> Qt PORT] This file is unchanged calculation/application logic.
 * Its XVT toolkit calls now run against the Qt5-backed compatibility layer
 * in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the original
 * proprietary XVT SDK. No calculation or algorithmic code in this file was
 * modified to accomplish this port -- only the underlying GUI toolkit
 * implementation, in qt_compat/, changed.
 * ============================================================================
 */
#define BLOCK_WINDOW_TITLE    "Block Diagram"
#define BLOCK_WINDOW_POS_X    60
#define BLOCK_WINDOW_POS_Y    60
#define BLOCK_WINDOW_WIDTH    500
#define BLOCK_WINDOW_HEIGHT   300

#define MOVIE_WINDOW_TITLE    "Movie"
#define MOVIE_WINDOW_POS_X    60
#define MOVIE_WINDOW_POS_Y    60
#define MOVIE_WINDOW_WIDTH    500
#define MOVIE_WINDOW_HEIGHT   390

#define WELL_LOG_TITLE        "Well Log"
#define WELL_LOG_POS_X        60
#define WELL_LOG_POS_Y        60
#define WELL_LOG_WIDTH        400
#define WELL_LOG_HEIGHT       430

#define SECTION_TITLE         "Section"
#define SECTION_POSITION_X         60
#define SECTION_POSITION_Y         60
#define SECTION_WIDTH         490
#define SECTION_WIN_HEIGHT        325

#define TOPOGRAPHY_TITLE "Topography Map"
#define TOPOGRAPHY_POS_X      60
#define TOPOGRAPHY_POS_Y      60
#define TOPOGRAPHY_WIDTH      490
#define TOPOGRAPHY_HEIGHT     375

#define TOPOGRAPHY_3D_TITLE "3D Topography Map"
#define TOPOGRAPHY_3D_POS_X   60
#define TOPOGRAPHY_3D_POS_Y   60
#define TOPOGRAPHY_3D_WIDTH   490
#define TOPOGRAPHY_3D_HEIGHT  375

#define STRATIGRAPHY_3D_TITLE "3D Stratigraphy"
#define STRATIGRAPHY_3D_POS_X   60
#define STRATIGRAPHY_3D_POS_Y   60
#define STRATIGRAPHY_3D_WIDTH   490
#define STRATIGRAPHY_3D_HEIGHT  375

#define LINEMAP_TITLE         "Line Map"
#define LINEMAP_POS_X         60
#define LINEMAP_POS_Y         60
#define LINEMAP_WIDTH         490
#define LINEMAP_HEIGHT        375

#define PROFILE_FROM_IMAGE_TITLE "Profile - From Image"
#define PROFILE_TITLE         "Profile"
#define PROFILE_POS_X         60
#define PROFILE_POS_Y         60
#define PROFILE_WIDTH         490
#define PROFILE_HEIGHT        600

#define LINEMAP_PLOT_TITLE    "Sterographic Projections"
#define LINEMAP_PLOT_POS_X    60
#define LINEMAP_PLOT_POS_Y    60
#define LINEMAP_PLOT_WIDTH    430
#define LINEMAP_PLOT_HEIGHT   430

#define STRATIGRAPHY_TITLE    "Stratigraphic Column"
#define STRATIGRAPHY_POS_X    60
#define STRATIGRAPHY_POS_Y    60
#define STRATIGRAPHY_WIDTH    230
#define DYKE_PLUG_WIDTH       140
#define STRATIGRAPHY_HEIGHT   400

#define IMAGE_TITLE           "Image"
#define IMAGE_POS_X           30
#define IMAGE_POS_Y           30

#define CONTOUR_TITLE         "Contour"
#define CONTOUR_POS_X           30
#define CONTOUR_POS_Y           30

#define TOOLBAR_TITLE         "Toolbar"
#define TOOLBAR_POS_X           30
#define TOOLBAR_POS_Y           30
#define TOOLBAR_WIDTH           -1
#define TOOLBAR_HEIGHT          -1

#define HISTORY_TITLE         "History"
#define HISTORY_POS_X           30
/* Below TOOLBAR_POS_Y (30) plus the toolbar's own height (a single
 * auto-sized icon row, title bar and frame included) so a fresh install
 * with no saved window-position file doesn't start with History
 * partially covering the Toolbar (matches a user report: "Start Noddy
 * with History window not partially covering toolbar"). */
#define HISTORY_POS_Y           170
#define HISTORY_WIDTH           400
#define HISTORY_HEIGHT          300

#define LEGEND_TITLE          "Legend"
#define LEGEND_POS_X            30
#define LEGEND_POS_Y            70
#define LEGEND_WIDTH            400
#define LEGEND_HEIGHT           400
