# Modifications to the original Noddy code

This document lists every change made to the **original application logic**
(the ~1994-2009 `.c`/`.h` files) since the XVT→Qt5 porting effort began. It
does **not** cover `qt_compat/` itself, which is new code implementing a
Qt5-backed stand-in for the XVT toolkit API — see the "Qt compatibility
layer" section below for a summary of that instead, and `README.md`/
`CLAUDE.md` for how it fits together.

The goal throughout has been to leave calculation/application logic
untouched wherever possible — the port is meant to change only the GUI
toolkit underneath, not the geological/geophysical modelling itself. Every
place that goal was NOT fully met (a genuine bug fix, a new feature, a
default-value change, or anything else beyond a mechanical toolkit-call
swap) is marked inline in the source with a comment tag: `[Qt port fix]`,
`[Qt port ADDITION]`, `[Qt port change]`, `[Qt port CHANGE]`,
`[Qt port FIX]`, `[Qt port defensive fix]`, `[Qt port REMOVAL]`,
`[Qt port note]`, or `[Qt port, user-requested]`. This document is a
compiled index of those tags, grouped by file and function — grep the tag
text itself in the source for the full original comment and surrounding
code if you need more detail than the one-line summary here gives.

Many of these originated as specific numbered items in `todo.txt`
(referenced below as `todo #NN` where the source comment cites one); others
were bugs found and fixed opportunistically while porting or testing a
given window/feature.


## Qt compatibility layer (new code, not a "modification")

`qt_compat/` is an entirely new directory implementing the subset of the
XVT toolkit API this application actually calls, on top of real Qt5
widgets — every original `.c` file still calls the exact same
`xvt_win_get_ctl()`, `xvt_dm_post_error()`, etc. it always did; only what's
*underneath* those calls changed. Roughly:

- **`xvt_compat.cpp`** — the core adapter. `XvtWindow` (a plain `QWidget`
  subclass, deliberately not `Q_OBJECT`/moc-based) turns native Qt events
  into the app's `EVENT`/`WIN_EVENT_HANDLER` calling convention. Also
  implements the modal-dialog-disable stack (`pushModalDialog`/
  `popModalDialog`), the status bar, menu bar construction, and the
  drawing/backing-store model XVT's "draw anytime" semantics need.
- **`dialog_positions.cpp`** / **`dialog_registry.cpp`** — per-window
  control layout tables, reconstructed (since the original compiled XVT
  `.uid` resource file doesn't exist in this tree) from the Noddy reference
  manual's screenshots and cross-checked against each window's own
  event-handler code.
- **`menu_registry.cpp`** — the application's menu tree, similarly
  reconstructed.
- **`xvt_types.h`** / **`xvt.h`** — XVT's type/struct layouts and function
  declarations, verified against XVT's own Portability Toolkit and Motif
  platform documentation.
- **`win_console.cpp`** — a small separate translation unit for hiding the
  console window on Windows (kept separate from `xvt_compat.cpp` to avoid
  `windows.h` macro collisions with Qt headers).

None of this is a "modification to an original function" in the sense the
rest of this document covers — it's the toolkit implementation the ported
app now runs against. See `CLAUDE.md` for the fuller architecture writeup.


## Modifications to original application files

### batchNod.c
- **(file-level, top of file)**: Added unconditional `#include <sys/time.h>` needed for the new `-random` flag's timestamped output filename, not just the old XOLWS/MTFWS TIMER macro.
- **batchNoddy()**: Added a `-random` flag that generates a random history, writes it as a timestamped .his file, and computes a full block export plus gravity/magnetic anomalies without requiring an input history file.

### blklayop.c
- **LAYER_DISPLAY_WINDOW_eh()**: Added a `LAYER_SHOW_TOPO` case for the new "Show Topo" checkbox (Display Type popup, right-click on a Block Diagram), just toggling its checked state on click — the actual cube filtering happens on OK via nodLib2.c's `saveBlockImageOptions()` (todo #87).

### block.c
- **calcBlockPropertiesData()**: Added a call to `resetGaussianNoiseRngIfSeeded()` (geophy.c) as a follow-up to the Gaussian noise feature.

### builder.c
- **createPreviewWindow()**: Split the preview placeholder into two separate sibling windows (canvas + control row) because the shared window's E_UPDATE handler unconditionally redrew the full 3D/block preview over the row's controls, clobbering them despite correct geometry.
- **createPreviewWindow()**: Preview Type combo now defaults to "Block" instead of the untouched default "Plane" selection, per user request.
- **windowEventHandler()**: Toolbar height made 6px shorter (`ICON_SIZE-2` instead of `ICON_SIZE+4`) so icons fit snugly without visible slack below them (todo #86).

### calc3d.c
- **(before create3dStratMap, in the block-diagram sizing code)**: Added an explicit error message when `geologyCubeSize` is too large relative to the model's dimensions (collapsing xMax/yMax/zMax to 1), since `allSurface` silently produced zero triangles while the grid still drew, misleadingly appearing to finish instantly with no geology.

### DoBlock.c
- **renderBlockDiagram() / shadeCubeColor()**: Added `shadeCubeColor()` and used it to replace the block diagram cube face shading's raw `channel*SHADE_1`/`channel*SHADE_2` multiply-and-pack-into-`XVT_MAKE_COLOR` (unclamped -- a channel pushed past 255 silently wrapped modulo 256, corrupting bright/saturated layer colours) with a real clamped +/-10% variation (`SHADE_1`/`SHADE_2` changed from 1.2/0.8 to 1.1/0.9 to match), fixing the "colour triplet logic...poorly handled" user report (todo #88).

### eventlib.c
- **createEventOptions()**: Widened `groupXPos` from 150 to 320 so the EVENT_PREVIEW placeholder can be a full-size preview pane, since the old width capped it too narrow to show its Preview-Type/On controls.
- **createEventOptions()**: Bumped `groupYPos` from 16 to 22 to clear EVENT_OPTION_SELECTION's own height, which grew from 14 to 20px per user request (todo #80).

### fcnProto.h
- **createPopupWindow()**: Widened `appData` parameter from `long` to `intptr_t` to avoid pointer truncation on 64-bit builds.
- **createPositionedWindow()**: Same `long` → `intptr_t` widening of `appData` for 64-bit pointer safety.
- **createCenteredWindow()**: Same `long` → `intptr_t` widening of `appData`, confirmed fixing a real pointer-truncation crash on native 64-bit Windows.
- **saveSurfaceOrientations()**: New function declaration added for a new Geology menu feature (todo #43, lineEvnt.c).
- **readRandomHist() / RandomNoddy()**: New declarations added for a ported random-history generator ("Noddyverse" dataset tool); `readRandomHist`'s extra bool arg controls whether reference-generator side effects on app settings are applied (batch vs interactive use, todo #59).

### geophy.c
- **gaussianNoiseDraw() (state setup)**: Added a dedicated RNG state for optional Gaussian noise on density/susceptibility voxels, lazily seeded once per process (todo #44).
- **gaussianNoiseMultiplier()**: Added function computing a noise multiplier from `gaussianNoiseSigmaPercent`, clamped at 0 so noise can never flip density/susceptibility negative (user-requested safeguard).
- **resetGaussianNoiseRngIfSeeded()**: Added function to reseed the noise RNG when a non-zero seed is set, ensuring reproducible noise sequences across separate invocations (e.g. preview vs. final calculation).
- **assignLayerInBlockModels()**: Applied the Gaussian noise multiplier to density, magnetic susceptibility, and all three Sus X/Y/Z anisotropy axis arrays (the latter needed too since Block Diagram's Sus X/Y/Z display reads those arrays, not magSusData) (todo #44).
- **doGeophysics()**: Extended the "cannot calculate by index" condition to also cover `addGaussianNoise`, since the indexed calculation path bypasses the noised density/susceptibility arrays and would silently ignore the noise setting.
- **doGeophysics()**: Called `resetGaussianNoiseRngIfSeeded()` at the start so a configured noise seed takes effect for this calculation (todo #44 follow-up).
- **doGeophysics()**: Fixed an early-return path after a `calcBlockLayer` failure to call `finishLongJob()` before returning, matching an analogous nodLib2.c fix, so the status-bar modal stack doesn't stay stuck non-empty (todo #83-adjacent).

### getHist.c
- **loadGpsOpts()**: Added version-gated loading of the Gaussian noise enabled/sigma-percent fields (version > 8.00).
- **loadGpsOpts()**: Added version-gated loading of the Gaussian noise seed field (version > 8.01), as a follow-up.

### lineEvnt.c
- **drawSectionSymbol()**: Symbol colour now uses solid white on solid-colour maps/sections instead of an inverse-RGB negative, since the old logic ignored the varying fill background and only contrasted against a flat dark gray.
- **SaveOrientations()**: Added writing of the already-computed X/Y/Z columns to the orientation export file, which were previously kept in memory for redraw but never written out.
- **saveSurfaceOrientations()**: New "Save Surface Orientations" feature (Geology menu) added entirely as new code, sweeping a grid and computing bedding dip/dip-direction via the same calculation chain as a manual map click, without touching existing calculation code (todo #43).
- **saveSurfaceOrientations()**: Fixed the surface-sweep to sample real topography elevation at each X/Y point when a topography is loaded (falling back to the flat originZ surface outside its extent), instead of always using the flat surface as before.

### mainMenu.c
- **do_TASK_MENUBAR()**: Added handling for new "Random History" menu item, clearing the current history and generating a new one via ported RandomNoddy.c (todo #41).
- **do_TASK_MENUBAR()**: After generating a random history, added a call to `tidyObjects()` to snap event icon positions to the grid, since `readRandomHist()` assigns grid positions but doesn't set on-screen icon coordinates (todo #59).
- **do_TASK_MENUBAR()**: Added handling for new "Save Surface Orientations..." Geology menu item, not part of the original .rc resource (todo #43).
- **do_TASK_MENUBAR()**: Removed the "Vertical Derivatives" submenu and its associated handlers (First/Second Gravity/Magnetics, calcVertDerivImage) since the menu items no longer exist (todo #71).

### noddy.c
- **main()**: Recognise a bare `-random` argument (argc==2) as a batch trigger, matching existing `-h`/`-?` handling, so `noddy -random` alone runs batch instead of falling through to interactive mode.
- **main()**: Hide the console window on the interactive/GUI path (via a separate `win_console.cpp` translation unit to avoid windows.h macro collisions) so noddy behaves like a normal Windows app.

### nodGraph.c
- **backgroundColor (global)**: Default geology display background colour changed from black to white.
- **drawSelectedColorGrid()**: Same unclamped-shading bug and fix as DoBlock.c's `renderBlockDiagram()`/`shadeCubeColor()` -- this is the "Define Colour" dialog's own base/brighter/darker preview swatches (`SHADE_1`/`SHADE_2` changed 1.2/0.8 -> 1.1/0.9, with real per-channel clamping instead of an unclamped multiply feeding straight into `XVT_MAKE_COLOR`) (todo #88).

### nodInc.h
- **NOISE_WINDOW_eh / win_218_eh**: Added new event-handler declaration for a new Geophysics Calculation options noise sub-panel (todo #44, noisewin.c).
- **TASK_MENUBAR_GEOLOGY_SAVE_SURFACE_ORIENTATIONS**: Added new menu tag (value 1200, deliberately outside the real .rc-derived tag range) for a new Geology menu item (todo #43).
- **TASK_MENUBAR_1_RANDOM_HISTORY**: Added new menu tag (value 1201, same safe range) for a new File menu item (todo #41).
- **NOISE_WINDOW**: Added new dialog/window ID (218) for the Geophysics noise sub-panel (todo #44).
- **LAYER_SHOW_TOPO**: Added new ctlId (20) for the "Show Topo" checkbox in LAYER_DISPLAY_WINDOW's Display Type popup (todo #87).

### nodLib1.c
- **pasteObjects() (PASTE_OFFSET constant)**: Widened icon-spacing offset from 40 to 64 for Paste/Duplicate placement, since 40 still overlapped the icon+title label per a user screenshot.
- **projectOptions/geophysicsOptions default initializer**: Changed default `calculationMethod` from SPATIAL to SPECTRAL per user-requested default (todo #55).
- **geophysicsOptions default initializer**: Changed default `spectralPaddingType` from RAMP_PADDING to RECLECTION_PADDING per user-requested default (todo #55).
- **createPopupWindow() prototype**: Changed parameter type from `long` to `intptr_t` to match the widened `PTR_LONG` convention used elsewhere in the Qt compat layer.
- **updateMenuOptions()**: Added enabling of the new "Save Surface Orientations" menu item based on `historyPresent` (todo #43).
- **restoreWindowSize()**: Fixed "Default Size" to also account for a per-window menu bar's height (via the window's own client rect top offset) so content isn't squeezed under/behind the bar on windows like Section/Map.
- **setupPageForPrinter()**: Fixed `xvt_print_create` call to pass a `long*` instead of `int*`, avoiding a silent stack overflow on 64-bit Linux/Mac where `long` is 8 bytes.
- **printPicture()**: Same `long*` vs `int*` fix as `setupPageForPrinter()` for the `xvt_print_create` call.
- **getObjectsFromClipboard()**: Added explicit cast for `xvt_cb_get_data`'s `PICTURE` return type, needed after `PICTURE` was widened from `long` to `intptr_t` to stop clipboard pointer truncation/crashes on 64-bit Windows.
- **pasteObjects() (pasteAsDuplicate flag)**: Added a static flag set by `duplicateSelectedObjects()` so the shared `pasteObjects()` function can place results differently for Paste vs Duplicate.
- **pasteObjects()**: Replaced the original XVT synthesized-mouse-drag placement (never fully ported) with explicit placement logic: Paste lands right of the chain end, Duplicate lands directly below the duplicated object(s), per user-requested behavior.
- **pasteObjects()**: Added cascading push-down logic so a Duplicate's new copy doesn't land on top of existing objects already occupying that space.
- **pasteObjects()**: Added a `reorderObjects()` call after insertion so pasted/duplicated objects are sorted into visual left-to-right list order, fixing wrong connector lines and calculation-order desync.

### nodLib2.c
- **saveBlockImageOptions()**: Added a missing `finishLongJob()` to match an `initLongJob()` call, fixing a nesting-stack leak in the status-bar long-job UI that left the main menu/other windows permanently disabled after picking a property with specific layers selected (todo #83-adjacent).
- **initLongJob()**: Replaced the old modal JOB_STATUS_WINDOW popup and manual "disable all other windows" loop with `xvt_begin_long_job_ui()`, using the shared modal-dialog stack for equivalent protection while keeping the status bar (in TASK_WIN) usable and properly nestable (todo #83).
- **finishLongJob()**: Correspondingly replaced the old re-enable-all-windows logic with `xvt_end_long_job_ui()`, which only re-enables windows once the outermost job/dialog on the shared stack closes (todo #83).
- **filterBlockDataAboveTopo()**: New function (todo #87) -- when the "Show Topo" checkbox is set, marks any cube above the loaded topography surface at its X/Y as `COLOR_INVALID` (the same sentinel "Turn Off layers not wanted" already uses), so `renderBlockDiagram()` skips drawing it. Clamps each cube-center X/Y into the topography grid's own extent (to the center of the nearest edge cell, with a small safety margin past the exact half-cell boundary -- landing exactly on it still overflowed `getTopoValueAtLoc`'s internal index rounding by one) instead of skipping unfiltered whenever a column's raw X/Y fell outside the topo grid, which had left the whole north/east edge of the model uncut.
- **saveBlockImageOptions()**: Reads the "Show Topo" checkbox into `diagram->showTopo`, and calls `filterBlockDataAboveTopo()` after `diagram->blockData` is (re)computed for both the Layered and property-view cases (todo #87).
- **loadBlockImageOptions()**: Syncs the "Show Topo" checkbox from `diagram->showTopo` when the Display Type popup opens (todo #87).
- **updateBlockImageOptions()**: Enables/disables the "Show Topo" checkbox alongside `layersGB` (Layered + property views, i.e. every display type with real cube data) -- initially wired to the wrong flag (`lutGB`, property views only), which left it wrongly disabled for the Layered view (todo #87, user report).

### nodLib3.c
- **initAlterationZones()**: Changed default alteration profile "Alteration Distance" (maxX) from 0.0 to 500.0, since a 0.0 default silently made new alteration profiles have no effect until manually edited (todo #51, user-requested).
- **processCommandLine()**: Added `-random` command-line flag to trigger generating a random event history instead of reading one from a file (todo #41).
- **initProject()**: Changed default `calculationMethod` from SPATIAL to SPECTRAL and `spectralPaddingType` from RAMP_PADDING to RECLECTION_PADDING (user-requested defaults, todo #55).
- **initProject()**: Initialized new Gaussian-noise option defaults (`addGaussianNoise = FALSE`, sigma 1%, seed 0) added for todo #44.

### nodStruc.h
- **SECTION_DATA (solidColorMap field)**: Added field remembering whether a map/section pixmap was rendered solid-colour or line-style, so overlay orientation symbols can pick a contrasting colour.
- **DEFAULT_VIEW_GEOL_CUBE / DEFAULT_VIEW_GEOP_CUBE**: Changed default block cube size from 200.0m to 100.0m for both geology and geophysics (todo #57).
- **GEOPHYSICS_OPTIONS (addGaussianNoise/gaussianNoiseSigmaPercent fields)**: Added fields to apply relative Gaussian noise (stored as a percentage) to density/susceptibility voxel values during volume assignment (todo #44).
- **GEOPHYSICS_OPTIONS (gaussianNoiseSeed field)**: Added a seed field where 0 means time-seeded random noise and any non-zero value reseeds the RNG for reproducible noise across runs.
- **(X11/OpenGL include block)**: Excluded the unconditional X11/Xt include (only truly needed for OpenGL-only code) when `XVTWS == 0` (Qt), since it caused hard compile errors on native Windows/MinGW builds lacking X11.
- **BLOCK_DIAGRAM_DATA (showTopo field)**: Added field recording whether the "Show Topo" checkbox is set for this diagram, so cubes above the loaded topography surface are filtered out of the cube-based (Layered/property) display types (todo #87).

### nodwork1.c
- **update3dPreview()**: Fast slider/scrollbar drags could deliver overlapping calls whose `object` pointer got corrupted between calls; added a re-entrancy guard that coalesces pending updates instead of dropping or corrupting them.
- **update3dPreview()**: `object` param sporadically arrived truncated/mangled in transit (Qt signal-delivery race); re-fetches and cross-checks against `xvt_vobj_get_data(prevWin)` to detect and correct low-32-bit-match corruption.
- **updateLineMap()**: Skips creating a per-window menu bar (was `MENU_BAR_2`), matching the same no-menubar treatment given to Block Diagram windows.
- **updateSection()**: Re-enabled the previously-disabled per-window menu bar (Symbol/Event1/Event2/Define menu, todo #36) now that `xvt_vobj_get_client_rect` correctly accounts for menu-bar height.
- **updateSection()**: Fixed window mispositioning/undersizing on redraw by using only the SIZE (not offset position) from the menu-bar-adjusted client rect when calling `xvt_vobj_move`.
- **updateSection()**: Fixed progressive window shrinking on repeated resizes by adding the menu-bar height back to the size before `xvt_vobj_move`, compensating for the content widget's own embedded menu bar.

### nodwork2.c
- **lineMapEventHandler()**: Reordered E_UPDATE to draw symbols onto the pixmap before blitting to the window (defensive correctness fix, matching sectionEventHandler's identical fix), though the actual reported vanishing-symbols bug was traced elsewhere (todo #47).
- **sectionEventHandler()**: Fixed E_SIZE to zero-origin the resize rect since the client rect's `.top` is offset by this window's own menu bar height, preventing the window from progressively shrinking/going blank on resize.
- **sectionEventHandler()**: Fixed E_SIZE's `xvt_vobj_move` call to add back the menu bar height, since the content widget includes its own menu bar and was otherwise sized short each resize, causing progressive collapse (todo #36).
- **sectionEventHandler()**: On E_FOCUS, rebuild the combined Symbol/Event menu via `createLineMapMenubar` since it gets reverted off TASK_MENUBAR when the window loses MDI focus (todo #39).
- **sectionEventHandler()**: Fixed the E_FOCUS menu rebuild to save/restore `count`/`symbol`/`event1`/`event2`/`define` around the `createLineMapMenubar` call, since that function unconditionally zeroes them, previously discarding placed symbols on every refocus (todo #47).
- **readGeophys()**: Changed `else if` to two independent `if` checks for min/max value tracking, since the original only updated `maxValue` when the `minValue` check failed, causing max to sometimes stay stuck near its sentinel value.

### noisewin.c
- **(file banner, NOISE_WINDOW_eh)**: Entirely new Gaussian Noise sub-panel added to the Geophysics Calculation options tab, modeled on the existing GENERATE_WINDOW panel, adding a checkbox + sigma control feeding geophy.c's `assignLayerInBlockModels` (todo #44).

### optnlib.c
- **optionTabs index defines**: Shifted `GEOPHYSICS_SURVEY_INDEX`/`GEOPHYSICS_DISPLAY_INDEX`/`WINDOW_POSITION_INDEX` by +1 since GEOPHYSICS_CALC's tab group grew from 5 to 6 `setOptionsTab` calls (added NOISE_WINDOW) (todo #44).
- **createOptions()**: Widened `groupXPos` from 150 to 320 (matching an identical eventlib.c fix) so the Options preview pane has room for its real ~293x272 size.
- **createOptions()**: Added positioning logic for the new NOISE_WINDOW tab, placed below the alternate RANGE_WINDOW/PADDING_WINDOW pair (todo #44).
- **createOptions()**: Restacked Deform Field/Sus Units/Draped Survey into a single column below Altitude instead of a 2-column layout, since the combined row was wider than the dialog actually renders in this port, truncating Deform Field (todo #53, user-requested).
- **loadGeophysicsCalcOptions()**: Added loading of the new Gaussian noise checkbox/sigma/seed controls from `geophysicsOptions` (todo #44).
- **saveGeophysicsCalcOptions()**: Added saving of the new Gaussian noise checkbox/sigma/seed controls back to `geophysicsOptions` (todo #44).
- **loadVolumeSurfaceOptions()**: Fixed the "all layers" checkbox to be initialized from the real `threedViewOptions.allLayers` state instead of always starting unchecked, which previously clobbered layer visibility and caused blank 3D Triangulation renders (todo #61).

### petrophy_defs.h
- **PPHYS_ROCK[] (Granite, Limestone entries)**: Restored Granite and Limestone petrophysics values to their pre-"vitaliy" originals, reverting an unwanted data change.

### plot.c
- **dipddir()**: Same solid-white-vs-inverse-negative colour fix as lineEvnt.c's `drawSectionSymbol`, applied to dip/dip-direction label drawing.

### prevwin.c
- **PREVIEW_WINDOW_eh()**: Changed the app-data field from `long` to `intptr_t` to stop object pointers being truncated on LLP64 Windows.

### proflib.c
- **profilePointSelect()**: Added NULL check for `profOptions`, which can be NULL if an alteration-profile's data was never allocated (e.g. loaded from an older .his file), preventing a NULL-deref.
- **profilePointMove()**: Same defensive NULL check on `profOptions` as `profilePointSelect()`.
- **profilePointDrawMoving()**: Same defensive NULL check on `profOptions` as `profilePointSelect()`.
- **profilePointAdd()**: Same defensive NULL check on `profOptions` as `profilePointSelect()`.
- **profilePointDelete()**: Refuse to delete the last remaining profile point, fixing a NULL-deref crash when repeatedly deleting points down to zero in the Fold/Alteration-zone profile editors.

### RandomNoddy.c
- **(file banner)**: Ported the random-history generator from a sibling Noddy source tree; adapted it to use `get_win_info()` so it writes via the app's own report functions, fixed a double-pointer PRNG state corruption bug, corrected `RECLECTION_PADDING` spelling, bounded `unitName` sprintf to this tree's shorter `UNIT_NAME_LENGTH`, and dropped a dead `historyWindow` assignment.
- **(top, includes)**: Added explicit `<math.h>` include so `sqrt/log/pow/sin/cos` get real double-returning prototypes instead of undefined-behaviour implicit-int declarations on x86-64.
- **readRandomHist()**: Made `applyGlobalOptions` actually gate block/geology/geophysics/3D/project option resets and window-position clobbering, since the interactive "Random History" menu item shouldn't silently reset window sizes/geophysics defaults (only batch mode should).
- **loadRandomHistory()**: Restored fixed 5-event count (reverting a fork's random 3-5 count) per user request; also names each event after its type/counter (e.g. "Fold_01") instead of a generic "Event_NN" counter for clearer History window labels.
- **loadRandomHistory()**: Restored the original random event-type selection logic instead of a fork's hardcoded "always PLUG"; also added a tail call sequence (`reorderObjects`/invalidate/`updateFloatingMenu`) so newly generated events become visible both interactively and in batch mode.
- **loadRandomBlockOpts()**: Restored the original random block view dimensions/cube sizes instead of a fork's altered values.
- **loadRandomGpsOpts()**: Fixed `spectralPaddingType` to use this tree's correctly-spelled `RECLECTION_PADDING` enum member instead of the reference's `REFLECTION_PADDING`.
- **loadRandomPlug()**: Restored original random plug-type selection (reverting a fork's hardcoded "always ELLIPSOIDAL_PLUG") and restored random dip generation (reverting a fork's hardcoded 90.0 dip).
- **loadRandomPlug()**: Left `alterationZones` as `NONE_ALTERATION` (not reverted like sibling spots) since the reference's alternative value was invalid for this enum type, so it looked like an abandoned experiment rather than a real prior state.
- **loadRandomStratigraphy()**: Restored original random layer count (2-6) instead of a fork's hardcoded single layer.
- **loadRandomProperties()**: Restored original random plug lithology, random Met/Volc/Sed stratigraphy class selection, and random Sediment lithology, reverting a fork's hardcoded granite/Sed/limestone choices.
- **loadRandomProperties()**: Bounded the lithology-name portion of the `unitName` sprintf to 14 chars so it can't overflow this tree's 20-byte `UNIT_NAME_LENGTH` field (vs. 50 in the reference tree).
- **loadRandomProperties()**: Reset the `static cum_height` variable at the start of each new stratigraphy so layer heights don't keep accumulating across separate Random History invocations in the same session.

### reporter.c
- **report_gps_opts()**: Added output of the Gaussian noise settings (enabled flag, sigma percent, seed) to the report file, version-gated to match getHist.c's `loadGpsOpts` loading logic (todo #44).

### topoOp.c
- **TOPO_OPTIONS_WINDOW_eh()**: Fixed OK/Cancel to sync the "Use Topography" checkbox via a handle passed as window app data, replacing a stale control-ID lookup (`WIN_116_CHECKBOX_68`) left over from a pre-tabbed-Options-dialog UI that never actually found the checkbox.

### utopowin.c
- **USE_TOPOGRAPHY_WINDOW_eh()**: Passes the real "Use Topography" checkbox handle through as the new window's app data so TOPO_OPTIONS_WINDOW's OK/Cancel handlers can look it up directly, replacing the stale lookup path.

### xoroshiro128plus.h
- **splitmix64() / header-level**: Marked the RNG functions `static` so the header can safely be included from a second .c file (geophy.c) without multiple-definition link errors, with no behavior change for existing single-TU usage.


## Where to look for more detail

Every entry above corresponds to an inline comment in the source tagged
`[Qt port ...]` — search for that tag in the named file/function for the
original, fuller explanation (often including the specific user report or
symptom that prompted the fix). `todo.txt` tracks the still-open and
completed numbered work items referenced above as `todo #NN`.
