/* ============================================================================
 * XVT -> Qt COMPATIBILITY LAYER  (flagged file: see qt_compat/README.md)
 *
 * Per-control tooltip text, looked up by (dialog resId, ctlId) in
 * xvt_compat.cpp's createControlWidget(). Unlike dialog_registry.cpp and
 * dialog_positions.cpp, none of this is recoverable from any header or
 * resource file -- it's hand-written directly against each dialog's real
 * geological/functional meaning, verified against that dialog's own event
 * handler .c file (e.g. foldtwin.c for FFOLD WINDOW/152) and, for the
 * shared event-parameter dialogs (XYZ/DDP/DDD/AXIS/MATRIX/etc), against
 * eventlib.c's per-event-type tab assembly (which dialog resId is used for
 * which OBJECT->shape) and the OBJECT struct field layout in nodStruc.h,
 * rather than guessed from the control's often-generic auto-generated
 * label (e.g. "WIN 116 HSCROLL 51"). A control not yet covered here simply
 * gets no tooltip -- see the resId groups below for what's done so far.
 *
 * User-requested (2-3 sentence tooltips explaining what each parameter
 * does in geological terms, for every dialog in the app): organized
 * dialog-by-dialog, grouped with a banner comment naming the dialog
 * (resId) each block belongs to so future additions/edits can find the
 * right spot. Repeated parameters (e.g. a scrollbar + its edit box + its
 * static caption) intentionally share the identical tooltip text -- all
 * three widgets represent the same value.
 * ============================================================================
 */

struct TooltipEntry { long resId; int ctlId; const char *text; };

extern const TooltipEntry g_tooltips[] = {

/* ===== 137: EVENT WINDOW -- the shell dialog every geological event
 * (Fold, Fault, Stratigraphy, ...) opens into; hosts the Form/Position/
 * Orientation/Scale sub-pages below as tabs. See eventlib.c. ===== */
{ 137, 1, "Applies the current settings on every\npage of this dialog to the event and\ncloses it, updating the model." },
{ 137, 2, "The name of this event as it appears in\nthe History panel and event list -- a\nlabel only, it has no effect on the\ncalculated geology." },
{ 137, 3, "The name of this event as it appears in\nthe History panel and event list -- a\nlabel only, it has no effect on the\ncalculated geology." },
{ 137, 4, "Closes the dialog without applying any\nchanges made since it was opened." },
{ 137, 5, "Opens help for this event type (not\navailable in this build)." },
{ 137, 6, "Shows a live 3D preview of the model\nwith this event's current settings\napplied, updating as you change\nparameters." },
{ 137, 7, "Switches between this event's parameter\npages -- e.g. Form (shape),\nPosition/Orientation, and Scale -- each\ncovering a different aspect of the same\nevent." },
{ 137, 8, "Moves to the previous event in the\nhistory so its parameters can be\nreviewed or edited without closing this\ndialog." },
{ 137, 9, "Moves to the next event in the history\nso its parameters can be reviewed or\nedited without closing this dialog." },

/* ===== 139: XYZ WINDOW -- shared "Position" tab (event's reference
 * point in model coordinates), used by Fold, Fault, Unconformity, Shear
 * Zone, Dyke, Plug, Tilt (rotation axis point) and Import. ===== */
{ 139, 1, "Groups the X, Y and Z position fields --\nthe model-space coordinates of this\nevent's reference point." },
{ 139, 2, "The event's position along the model's\nEast-West (X) axis, in metres from the\nmodel origin." },
{ 139, 3, "The event's position along the model's\nNorth-South (Y) axis, in metres from the\nmodel origin." },
{ 139, 4, "The event's position along the vertical\n(Z) axis, in metres, positive upward\nfrom the model origin." },
{ 139, 5, "The event's position along the model's\nEast-West (X) axis, in metres from the\nmodel origin." },
{ 139, 6, "The event's position along the vertical\n(Z) axis, in metres, positive upward\nfrom the model origin." },
{ 139, 7, "The event's position along the model's\nNorth-South (Y) axis, in metres from the\nmodel origin." },
{ 139, 8, "The event's position along the model's\nEast-West (X) axis, in metres from the\nmodel origin." },
{ 139, 9, "The event's position along the model's\nNorth-South (Y) axis, in metres from the\nmodel origin." },
{ 139, 10, "The event's position along the vertical\n(Z) axis, in metres, positive upward\nfrom the model origin." },

/* ===== 140: DDP WINDOW -- shared "Orientation" tab (Dip/Dip-Direction/
 * Pitch), used by Fold, Fault, Shear Zone, Dyke and Plug -- surfaces
 * that also carry a lineation (fold hinge, slip direction) needing a
 * pitch, unlike DDD's dip/dip-direction-only pair below. ===== */
{ 140, 1, "Groups the dip, dip direction and pitch\nfields describing this surface's\norientation." },
{ 140, 2, "Dip direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the surface dips towards -- the\ndirection water would run if poured onto\nit." },
{ 140, 3, "Dip: the angle, in degrees, between this\nsurface and the horizontal, measured\nperpendicular to strike -- 0 is\nflat-lying, 90 is vertical." },
{ 140, 4, "Pitch: the angle, in degrees, between a\nlineation on this surface (e.g. a fold\nhinge or slip direction) and the\nsurface's strike line, measured within\nthe surface itself." },
{ 140, 5, "Dip direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the surface dips towards." },
{ 140, 6, "Dip: the angle, in degrees, between this\nsurface and the horizontal, measured\nperpendicular to strike." },
{ 140, 7, "Pitch: the angle, in degrees, between a\nlineation on this surface and its strike\nline, measured within the surface\nitself." },
{ 140, 8, "Dip direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the surface dips towards." },
{ 140, 9, "Pitch: the angle, in degrees, between a\nlineation on this surface and its strike\nline, measured within the surface\nitself." },
{ 140, 10, "Dip: the angle, in degrees, between this\nsurface and the horizontal, measured\nperpendicular to strike." },

/* ===== 141: ROTATION WINDOW -- Tilt event's "Scale" tab (the rotation
 * amount; the axis point and orientation live on the Position/
 * Orientation tabs, XYZ_WINDOW/PP_WINDOW below). ===== */
{ 141, 1, "Groups the rotation angle field." },
{ 141, 2, "The angle, in degrees, that everything\nbelow this event is rotated by around\nthe rotation axis defined on the\nPosition/Orientation tabs." },
{ 141, 3, "The angle, in degrees, that everything\nbelow this event is rotated by around\nthe rotation axis defined on the\nPosition/Orientation tabs." },
{ 141, 4, "The angle, in degrees, that everything\nbelow this event is rotated by around\nthe rotation axis defined on the\nPosition/Orientation tabs." },
{ 141, 5, "The angle, in degrees, that everything\nbelow this event is rotated by around\nthe rotation axis defined on the\nPosition/Orientation tabs." },

/* ===== 142: PP WINDOW -- "Plunge/Plunge-direction" orientation tab,
 * used by Tilt (orientation of the rotation axis) and Lineation (the
 * lineation's own orientation). ===== */
{ 142, 1, "Groups the plunge direction and plunge\nfields." },
{ 142, 2, "Plunge direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the line plunges towards." },
{ 142, 3, "Plunge: the angle, in degrees, that the\nline is inclined below the horizontal,\nmeasured in the vertical plane\ncontaining the plunge direction." },
{ 142, 4, "Plunge direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the line plunges towards." },
{ 142, 5, "Plunge: the angle, in degrees, that the\nline is inclined below the horizontal,\nmeasured in the vertical plane\ncontaining the plunge direction." },
{ 142, 6, "Plunge direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the line plunges towards." },
{ 142, 7, "Plunge: the angle, in degrees, that the\nline is inclined below the horizontal,\nmeasured in the vertical plane\ncontaining the plunge direction." },

/* ===== 143: GMA WINDOW -- Fault event's "Form" tab (profile shape and
 * displacement style, before Position/Orientation/Scale). ===== */
{ 143, 1, "The fault's cross-sectional (profile)\nshape -- e.g. planar, curved or listric\n(concave, flattening with depth) --\ncontrolling how the fault surface bends\nbetween the surface and depth." },
{ 143, 2, "The fault's cross-sectional (profile)\nshape -- e.g. planar, curved or listric\n(concave, flattening with depth) --\ncontrolling how the fault surface bends\nbetween the surface and depth." },
{ 143, 3, "Groups the fault's Form controls:\nprofile geometry, sense of movement, and\ndisplacement alteration." },
{ 143, 4, "The sense of displacement across the\nfault -- e.g. normal, reverse or\nstrike-slip -- determining which side\nmoves up, down or sideways relative to\nthe other." },
{ 143, 5, "The sense of displacement across the\nfault -- e.g. normal, reverse or\nstrike-slip -- determining which side\nmoves up, down or sideways relative to\nthe other." },
{ 143, 6, "How much the fault's displacement dies\naway from the fault's centre, tapering\nthe offset toward its edges instead of\napplying it uniformly along the whole\nsurface." },
{ 143, 7, "How much the fault's displacement dies\naway from the fault's centre, tapering\nthe offset toward its edges instead of\napplying it uniformly along the whole\nsurface." },
{ 143, 8, "How much the fault's displacement dies\naway from the fault's centre, tapering\nthe offset toward its edges instead of\napplying it uniformly along the whole\nsurface." },
{ 143, 9, "Opens a colour picker to set the colour\nthis fault is drawn with in diagrams and\nmaps." },
{ 143, 10, "Opens the profile editor to draw a\ncustom displacement/alteration profile\ninstead of using the default preset\nshape." },

/* ===== 144: ALT WINDOW -- Unconformity event's "Form" tab (erosion
 * relief profile; simpler than a Fault's since an unconformity has no
 * separate movement sense). ===== */
{ 144, 1, "How uneven the unconformity's erosion\nsurface is -- how much its relief varies\n-- rather than the surface being\nperfectly planar." },
{ 144, 2, "How uneven the unconformity's erosion\nsurface is -- how much its relief varies\n-- rather than the surface being\nperfectly planar." },
{ 144, 3, "How uneven the unconformity's erosion\nsurface is -- how much its relief varies\n-- rather than the surface being\nperfectly planar." },
{ 144, 4, "Groups the unconformity's Form control:\nthe erosion relief amount." },

/* ===== 145: DDD WINDOW -- shared "Orientation" tab (Dip/Dip-Direction
 * only, no pitch), used by Unconformity and Foliation -- planar
 * features with no separate lineation. ===== */
{ 145, 1, "Groups the dip and dip direction fields\ndescribing this surface's orientation." },
{ 145, 2, "Dip direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the surface dips towards -- the\ndirection water would run if poured onto\nit." },
{ 145, 3, "Dip: the angle, in degrees, between this\nsurface and the horizontal, measured\nperpendicular to strike -- 0 is\nflat-lying, 90 is vertical." },
{ 145, 4, "Dip direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the surface dips towards." },
{ 145, 5, "Dip: the angle, in degrees, between this\nsurface and the horizontal, measured\nperpendicular to strike." },
{ 145, 6, "Dip direction: the compass direction\n(0-360 degrees, clockwise from north)\nthat the surface dips towards." },
{ 145, 7, "Dip: the angle, in degrees, between this\nsurface and the horizontal, measured\nperpendicular to strike." },

/* ===== 146: MATRIX WINDOW -- Strain event's only tab: the 3x3
 * deformation tensor applied to every point below it (nodStruc.h's
 * STRAIN_OPTIONS::tensor[3][3]). Row i, column j is the coefficient of
 * input axis j's contribution to output axis i (row 1=X, 2=Y, 3=Z). ===== */
{ 146, 1, "Row 1, column 1 of the 3x3 deformation\nmatrix -- how much of a point's original\nX coordinate contributes to its new X\ncoordinate (X-direction stretch)." },
{ 146, 2, "Row 1, column 2 of the 3x3 deformation\nmatrix -- how much of a point's original\nY coordinate contributes to its new X\ncoordinate (a shear term)." },
{ 146, 3, "Row 1, column 3 of the 3x3 deformation\nmatrix -- how much of a point's original\nZ coordinate contributes to its new X\ncoordinate (a shear term)." },
{ 146, 4, "Row 2, column 1 of the 3x3 deformation\nmatrix -- how much of a point's original\nX coordinate contributes to its new Y\ncoordinate (a shear term)." },
{ 146, 5, "Row 2, column 2 of the 3x3 deformation\nmatrix -- how much of a point's original\nY coordinate contributes to its new Y\ncoordinate (Y-direction stretch)." },
{ 146, 6, "Row 2, column 3 of the 3x3 deformation\nmatrix -- how much of a point's original\nZ coordinate contributes to its new Y\ncoordinate (a shear term)." },
{ 146, 7, "Row 3, column 1 of the 3x3 deformation\nmatrix -- how much of a point's original\nX coordinate contributes to its new Z\ncoordinate (a shear term)." },
{ 146, 8, "Row 3, column 2 of the 3x3 deformation\nmatrix -- how much of a point's original\nY coordinate contributes to its new Z\ncoordinate (a shear term)." },
{ 146, 9, "Row 3, column 3 of the 3x3 deformation\nmatrix -- how much of a point's original\nZ coordinate contributes to its new Z\ncoordinate (Z-direction/vertical\nstretch)." },
{ 146, 10, "Groups the 3x3 deformation matrix that\nthis Strain event applies to every point\nbelow it in the history -- the identity\nmatrix (1 on the diagonal, 0 elsewhere)\nleaves geometry unchanged." },

/* ===== 148: ROCK WINDOW -- petrophysical property editor for a
 * stratigraphic unit or intrusion: density/susceptibility for gravity
 * and magnetic forward modelling, plus optional anisotropy and remanent
 * magnetization. ===== */
{ 148, 1, "The elevation of the top of this rock\nunit, in metres -- together with the\nunit above/below it, this sets the\nunit's thickness." },
{ 148, 2, "The elevation of the top of this rock\nunit, in metres -- together with the\nunit above/below it, this sets the\nunit's thickness." },
{ 148, 3, "The rock's density, used to calculate\nits contribution to the model's gravity\nresponse." },
{ 148, 4, "The rock's density, used to calculate\nits contribution to the model's gravity\nresponse." },
{ 148, 5, "The name of this rock unit, shown in the\nstratigraphy list and legends." },
{ 148, 6, "The rock's magnetic susceptibility\nmagnitude, used to calculate its\ncontribution to the model's induced\nmagnetic response." },
{ 148, 7, "Opens a colour picker to set the colour\nthis rock unit is drawn with in\ndiagrams, sections and maps." },
{ 148, 8, "Groups the rock's basic properties:\nname, density and magnetic\nsusceptibility." },
{ 148, 9, "The rock's magnetic susceptibility\nmagnitude, used to calculate its\ncontribution to the model's induced\nmagnetic response." },
{ 148, 10, "Groups the rock's magnetic anisotropy\nand remanent magnetization properties." },
{ 148, 11, "Enables anisotropic magnetic\nsusceptibility for this rock, so its\nmagnetic response depends on the\norientation set below rather than being\nthe same in every direction." },
{ 148, 12, "Enables remanent magnetization for this\nrock, adding a fixed magnetization\ndirection/intensity (from ancient field\nconditions) on top of the induced\nresponse from the present-day field." },
{ 148, 13, "Declination: the compass direction\n(0-360 degrees, clockwise from north) of\nthe rock's remanent magnetization." },
{ 148, 14, "Inclination: the angle, in degrees, that\nthe rock's remanent magnetization is\ntilted below (or above) the horizontal." },
{ 148, 15, "The X component of the remanent\nmagnetization vector, calculated from\nthe declination, inclination and\nintensity below." },
{ 148, 16, "The remanent magnetization vector's\nX/Y/Z components, calculated from the\ndeclination, inclination and intensity\nbelow." },
{ 148, 17, "The Y component of the remanent\nmagnetization vector, calculated from\nthe declination, inclination and\nintensity below." },
{ 148, 18, "The Z component of the remanent\nmagnetization vector, calculated from\nthe declination, inclination and\nintensity below." },
{ 148, 19, "The X component of the remanent\nmagnetization vector, calculated from\nthe declination, inclination and\nintensity below." },
{ 148, 20, "The Y component of the remanent\nmagnetization vector, calculated from\nthe declination, inclination and\nintensity below." },
{ 148, 21, "The Z component of the remanent\nmagnetization vector, calculated from\nthe declination, inclination and\nintensity below." },
{ 148, 22, "Declination: the compass direction\n(0-360 degrees, clockwise from north) of\nthe rock's remanent magnetization." },
{ 148, 23, "Inclination: the angle, in degrees, that\nthe rock's remanent magnetization is\ntilted below (or above) the horizontal." },
{ 148, 24, "Groups the remanent magnetization\ndirection and intensity fields." },
{ 148, 25, "Dip: the angle, in degrees, of the\nanisotropy ellipsoid's principal axis,\ncontrolling the direction in which\nmagnetic susceptibility differs from the\nother two axes." },
{ 148, 26, "Dip direction: the compass direction\n(0-360 degrees, clockwise from north) of\nthe anisotropy ellipsoid's principal\naxis." },
{ 148, 27, "Pitch: the angle, in degrees, of the\nanisotropy ellipsoid's secondary axis\nwithin the plane defined by the dip and\ndip direction above." },
{ 148, 28, "Dip: the angle, in degrees, of the\nanisotropy ellipsoid's principal axis,\ncontrolling the direction in which\nmagnetic susceptibility differs from the\nother two axes." },
{ 148, 29, "Dip direction: the compass direction\n(0-360 degrees, clockwise from north) of\nthe anisotropy ellipsoid's principal\naxis." },
{ 148, 30, "Pitch: the angle, in degrees, of the\nanisotropy ellipsoid's secondary axis\nwithin the plane defined by the dip and\ndip direction above." },
{ 148, 31, "The name of this rock unit, shown in the\nstratigraphy list and legends." },
{ 148, 32, "Groups the OK/Cancel/Help/Alter controls\nfor this rock property editor." },
{ 148, 33, "Applies the current property values to\nthis rock unit and closes the dialog." },
{ 148, 34, "Discards any changes made to this rock\nunit's properties and closes the dialog." },
{ 148, 35, "Opens help for rock properties (not\navailable in this build)." },
{ 148, 36, "Opens the alteration editor for this\nrock unit, letting its properties vary\nspatially instead of being a single\nfixed value." },
{ 148, 37, "The width of this rock unit, used\ntogether with its height/thickness where\nthe model needs a finite lateral or\nvertical extent for the unit." },
{ 148, 38, "The width of this rock unit, used\ntogether with its height/thickness where\nthe model needs a finite lateral or\nvertical extent for the unit." },

/* ===== 152: FFOLD WINDOW (fformwin.c) -- Fold event's "Form" tab: the
 * fold's cross-sectional profile shape. ===== */
{ 152, 1, "Opens the profile editor to draw a\ncustom fold cross-section instead of\nusing a preset Sine profile." },
{ 152, 2, "The fold's cross-sectional profile shape\n-- Sine Fold uses a simple sinusoidal\nwave; any other choice opens the profile\neditor to draw a custom shape." },
{ 152, 3, "The fold's cross-sectional profile shape\n-- Sine Fold uses a simple sinusoidal\nwave; any other choice opens the profile\neditor to draw a custom shape." },
{ 152, 4, "Groups the fold's Form controls: profile\nshape and single/periodic choice." },
{ 152, 5, "When checked, the fold occurs once (a\nsingle fold) instead of repeating\nperiodically along its wavelength for\nthe full extent of the model." },

/* ===== 153: FDSCL WINDOW (fdsclwin.c) -- Fold event's "Scale" tab. ===== */
{ 153, 1, "Groups the fold's Scale controls:\nwavelength, amplitude and cyclicity." },
{ 153, 2, "Wavelength: the distance, in metres,\nover which the fold's profile repeats\none full cycle." },
{ 153, 3, "Amplitude: half the vertical distance,\nin metres, between the fold's crest and\ntrough -- how tight or open the fold is." },
{ 153, 4, "Cyclicity: how many times the fold's\nprofile repeats; higher values produce a\ntrain of several fold cycles instead of\na single one." },
{ 153, 5, "Wavelength: the distance, in metres,\nover which the fold's profile repeats\none full cycle." },
{ 153, 6, "Amplitude: half the vertical distance,\nin metres, between the fold's crest and\ntrough -- how tight or open the fold is." },
{ 153, 7, "Cyclicity: how many times the fold's\nprofile repeats; higher values produce a\ntrain of several fold cycles instead of\na single one." },
{ 153, 8, "Wavelength: the distance, in metres,\nover which the fold's profile repeats\none full cycle." },
{ 153, 9, "Cyclicity: how many times the fold's\nprofile repeats; higher values produce a\ntrain of several fold cycles instead of\na single one." },
{ 153, 10, "Amplitude: half the vertical distance,\nin metres, between the fold's crest and\ntrough -- how tight or open the fold is." },

/* ===== 154: TGMA WINDOW (tgmawin.c) -- Shear Zone event's "Form" tab
 * (like Fault's GMA, plus a shear-style Type choice). ===== */
{ 154, 1, "The shear zone's cross-sectional\n(profile) shape -- e.g. planar, curved\nor listric -- controlling how the zone\nbends between the surface and depth." },
{ 154, 2, "The shear zone's cross-sectional\n(profile) shape -- e.g. planar, curved\nor listric -- controlling how the zone\nbends between the surface and depth." },
{ 154, 3, "Groups the shear zone's Form controls:\nprofile geometry, movement sense,\nalteration and shear style." },
{ 154, 4, "The sense of displacement across the\nshear zone -- e.g. normal, reverse or\nstrike-slip -- determining which side\nmoves relative to the other." },
{ 154, 5, "The sense of displacement across the\nshear zone -- e.g. normal, reverse or\nstrike-slip -- determining which side\nmoves relative to the other." },
{ 154, 6, "How much the shear zone's displacement\ndies away from its centre, tapering the\noffset toward its edges instead of\napplying it uniformly." },
{ 154, 7, "How much the shear zone's displacement\ndies away from its centre, tapering the\noffset toward its edges instead of\napplying it uniformly." },
{ 154, 8, "How much the shear zone's displacement\ndies away from its centre, tapering the\noffset toward its edges instead of\napplying it uniformly." },
{ 154, 9, "The shear zone's deformation style --\ne.g. simple shear or pure shear -- i.e.\nhow strain is distributed across the\nzone's width." },
{ 154, 10, "The shear zone's deformation style --\ne.g. simple shear or pure shear -- i.e.\nhow strain is distributed across the\nzone's width." },
{ 154, 11, "Opens the profile editor to draw a\ncustom displacement/alteration profile\ninstead of using the default preset\nshape." },

/* ===== 155: FSCL WINDOW (fsclwin.c) -- Fault event's "Scale" tab. ===== */
{ 155, 1, "Slip: the displacement, in metres,\nacross the fault -- how far one side has\nmoved relative to the other." },
{ 155, 2, "Slip: the displacement, in metres,\nacross the fault -- how far one side has\nmoved relative to the other." },
{ 155, 3, "Slip: the displacement, in metres,\nacross the fault -- how far one side has\nmoved relative to the other." },
{ 155, 4, "Groups the fault's Scale controls: slip,\nrotation, amplitude, radius,\ncylindricity and pitch." },
{ 155, 5, "Rotation: the angle, in degrees, that\nthe hanging-wall block is rotated by\naround the fault, for faults with a\nrotational (listric) component." },
{ 155, 6, "Rotation: the angle, in degrees, that\nthe hanging-wall block is rotated by\naround the fault, for faults with a\nrotational (listric) component." },
{ 155, 7, "Rotation: the angle, in degrees, that\nthe hanging-wall block is rotated by\naround the fault, for faults with a\nrotational (listric) component." },
{ 155, 8, "Amplitude: how strongly the fault\nsurface curves away from planar, for\ncurved or listric fault geometries." },
{ 155, 9, "Amplitude: how strongly the fault\nsurface curves away from planar, for\ncurved or listric fault geometries." },
{ 155, 10, "Amplitude: how strongly the fault\nsurface curves away from planar, for\ncurved or listric fault geometries." },
{ 155, 11, "Radius: the radius of curvature, in\nmetres, of the fault surface, for curved\nor listric fault geometries." },
{ 155, 12, "Radius: the radius of curvature, in\nmetres, of the fault surface, for curved\nor listric fault geometries." },
{ 155, 13, "Radius: the radius of curvature, in\nmetres, of the fault surface, for curved\nor listric fault geometries." },
{ 155, 14, "Cylindricity: how cylindrical (versus\nconical/spherical) the fault's curved\nsurface is along strike." },
{ 155, 15, "Cylindricity: how cylindrical (versus\nconical/spherical) the fault's curved\nsurface is along strike." },
{ 155, 16, "Cylindricity: how cylindrical (versus\nconical/spherical) the fault's curved\nsurface is along strike." },
{ 155, 17, "Pitch: the direction of slip within the\nfault plane, as an angle from the\nfault's strike line -- 90 is pure\ndip-slip, 0 is pure strike-slip." },
{ 155, 18, "Pitch: the direction of slip within the\nfault plane, as an angle from the\nfault's strike line -- 90 is pure\ndip-slip, 0 is pure strike-slip." },
{ 155, 19, "Pitch: the direction of slip within the\nfault plane, as an angle from the\nfault's strike line -- 90 is pure\ndip-slip, 0 is pure strike-slip." },

/* ===== 156: SSCL WINDOW (ssclwin.c) -- Shear Zone event's "Scale" tab
 * (same as Fault's FSCL, plus a finite zone Width since a shear zone's
 * displacement is distributed rather than on a single surface). ===== */
{ 156, 1, "Slip: the total displacement, in metres,\nacross the shear zone -- how far one\nside has moved relative to the other." },
{ 156, 2, "Slip: the total displacement, in metres,\nacross the shear zone -- how far one\nside has moved relative to the other." },
{ 156, 3, "Slip: the total displacement, in metres,\nacross the shear zone -- how far one\nside has moved relative to the other." },
{ 156, 4, "Groups the shear zone's Scale controls:\nslip, rotation, amplitude, radius,\ncylindricity, pitch and zone width." },
{ 156, 5, "Rotation: the angle, in degrees, that\nmaterial is rotated by across the shear\nzone, for zones with a rotational\ncomponent." },
{ 156, 6, "Rotation: the angle, in degrees, that\nmaterial is rotated by across the shear\nzone, for zones with a rotational\ncomponent." },
{ 156, 7, "Rotation: the angle, in degrees, that\nmaterial is rotated by across the shear\nzone, for zones with a rotational\ncomponent." },
{ 156, 8, "Amplitude: how strongly the shear zone's\nsurface curves away from planar, for\ncurved geometries." },
{ 156, 9, "Amplitude: how strongly the shear zone's\nsurface curves away from planar, for\ncurved geometries." },
{ 156, 10, "Amplitude: how strongly the shear zone's\nsurface curves away from planar, for\ncurved geometries." },
{ 156, 11, "Radius: the radius of curvature, in\nmetres, of the shear zone's surface, for\ncurved geometries." },
{ 156, 12, "Radius: the radius of curvature, in\nmetres, of the shear zone's surface, for\ncurved geometries." },
{ 156, 13, "Radius: the radius of curvature, in\nmetres, of the shear zone's surface, for\ncurved geometries." },
{ 156, 14, "Cylindricity: how cylindrical (versus\nconical/spherical) the shear zone's\ncurved surface is along strike." },
{ 156, 15, "Cylindricity: how cylindrical (versus\nconical/spherical) the shear zone's\ncurved surface is along strike." },
{ 156, 16, "Cylindricity: how cylindrical (versus\nconical/spherical) the shear zone's\ncurved surface is along strike." },
{ 156, 17, "Pitch: the direction of slip within the\nshear zone, as an angle from its strike\nline -- 90 is pure dip-slip, 0 is pure\nstrike-slip." },
{ 156, 18, "Pitch: the direction of slip within the\nshear zone, as an angle from its strike\nline -- 90 is pure dip-slip, 0 is pure\nstrike-slip." },
{ 156, 19, "Pitch: the direction of slip within the\nshear zone, as an angle from its strike\nline -- 90 is pure dip-slip, 0 is pure\nstrike-slip." },
{ 156, 20, "Width: the thickness, in metres, of the\nshear zone -- how far across the zone\nthe displacement is spread, rather than\nbeing concentrated on a single surface\nas in a Fault." },
{ 156, 21, "Width: the thickness, in metres, of the\nshear zone -- how far across the zone\nthe displacement is spread, rather than\nbeing concentrated on a single surface\nas in a Fault." },
{ 156, 22, "Width: the thickness, in metres, of the\nshear zone -- how far across the zone\nthe displacement is spread, rather than\nbeing concentrated on a single surface\nas in a Fault." },

/* ===== 157: FDYKE WINDOW (dformwin.c) -- Dyke event's "Form" tab. ===== */
{ 157, 1, "Opens the profile editor to draw how\nthis dyke's alteration/margin profile\nvaries instead of using a uniform value." },
{ 157, 2, "The dyke's cross-sectional shape/type." },
{ 157, 3, "The dyke's cross-sectional shape/type." },
{ 157, 4, "Groups the dyke's Form controls:\ncross-sectional type and alteration." },
{ 157, 5, "How much the dyke's contact alteration\nhalo varies -- e.g. thermal or chemical\nalteration of the host rock adjacent to\nthe dyke -- rather than being uniform\nalong its length." },
{ 157, 6, "How much the dyke's contact alteration\nhalo varies -- e.g. thermal or chemical\nalteration of the host rock adjacent to\nthe dyke -- rather than being uniform\nalong its length." },
{ 157, 7, "How much the dyke's contact alteration\nhalo varies -- e.g. thermal or chemical\nalteration of the host rock adjacent to\nthe dyke -- rather than being uniform\nalong its length." },
{ 157, 8, "Opens the rock-property editor for the\ndyke's own intrusive rock (density,\nsusceptibility, colour, etc.)." },

/* ===== 158: SW WINDOW (swwin.c) -- Dyke event's "Scale" tab. ===== */
{ 158, 1, "Groups the dyke's Scale controls:\nopening slip and width." },
{ 158, 2, "Slip: how far, in metres, the host rock\nis displaced apart (or the intrusion is\noffset) to make room for the dyke." },
{ 158, 3, "Width: the thickness, in metres, of the\ndyke." },
{ 158, 4, "Slip: how far, in metres, the host rock\nis displaced apart (or the intrusion is\noffset) to make room for the dyke." },
{ 158, 5, "Width: the thickness, in metres, of the\ndyke." },
{ 158, 6, "Slip: how far, in metres, the host rock\nis displaced apart (or the intrusion is\noffset) to make room for the dyke." },
{ 158, 7, "Width: the thickness, in metres, of the\ndyke." },

/* ===== 159: FPLUG WINDOW (pformwin.c) -- Plug event's "Form" tab. ===== */
{ 159, 1, "How this intrusive plug's geometry\nmerges with the surrounding rock -- e.g.\nwhether it truncates or blends with the\nhost stratigraphy." },
{ 159, 2, "The plug's overall geometry (e.g.\ncylindrical, conical, ellipsoidal) --\nits 3D shape is further refined by the\nradius, angle and axis values on the\nScale tab." },
{ 159, 3, "The plug's overall geometry (e.g.\ncylindrical, conical, ellipsoidal) --\nits 3D shape is further refined by the\nradius, angle and axis values on the\nScale tab." },
{ 159, 4, "Groups the plug's Form controls:\ngeometry type and alteration." },
{ 159, 5, "How much the plug's contact alteration\nhalo varies -- e.g. thermal or chemical\nalteration of the host rock adjacent to\nthe intrusion -- rather than being\nuniform." },
{ 159, 6, "How much the plug's contact alteration\nhalo varies -- e.g. thermal or chemical\nalteration of the host rock adjacent to\nthe intrusion -- rather than being\nuniform." },
{ 159, 7, "How much the plug's contact alteration\nhalo varies -- e.g. thermal or chemical\nalteration of the host rock adjacent to\nthe intrusion -- rather than being\nuniform." },
{ 159, 8, "Opens the rock-property editor for the\nplug's own intrusive rock (density,\nsusceptibility, colour, etc.)." },

/* ===== 160: PSCL WINDOW (psclwin.c) -- Plug event's "Scale" tab. ===== */
{ 160, 1, "Groups the plug's Scale controls:\nradius, apical angle and B-value shape\nexponent." },
{ 160, 2, "Radius: the plug's characteristic\nradius, in metres, at its reference\ndepth -- its overall size." },
{ 160, 3, "Apical angle: the angle, in degrees, the\nplug's sides flare outward (or inward)\nwith depth -- 0 gives a straight\ncylinder, positive values a widening\ncone." },
{ 160, 4, "Radius: the plug's characteristic\nradius, in metres, at its reference\ndepth -- its overall size." },
{ 160, 5, "Apical angle: the angle, in degrees, the\nplug's sides flare outward (or inward)\nwith depth -- 0 gives a straight\ncylinder, positive values a widening\ncone." },
{ 160, 6, "Radius: the plug's characteristic\nradius, in metres, at its reference\ndepth -- its overall size." },
{ 160, 7, "Apical angle: the angle, in degrees, the\nplug's sides flare outward (or inward)\nwith depth -- 0 gives a straight\ncylinder, positive values a widening\ncone." },
{ 160, 8, "B-value: a shape exponent controlling\nhow square versus rounded the plug's\ncross-section is -- higher values give a\nmore box-like (superelliptical) shape,\nlower values a more elliptical one." },
{ 160, 9, "B-value: a shape exponent controlling\nhow square versus rounded the plug's\ncross-section is -- higher values give a\nmore box-like (superelliptical) shape,\nlower values a more elliptical one." },
{ 160, 10, "B-value: a shape exponent controlling\nhow square versus rounded the plug's\ncross-section is -- higher values give a\nmore box-like (superelliptical) shape,\nlower values a more elliptical one." },

/* ===== 161: AXIS WINDOW -- Fault/Shear Zone/Plug's "Scale (Ellipsoid)"
 * tab: the three semi-axis lengths of the ellipsoidal zone their
 * displacement/alteration is distributed across (nodStruc.h's axisA/
 * axisB/axisC). ===== */
{ 161, 1, "Groups the ellipsoid's three semi-axis\nlengths, which set the 3D extent and\naspect ratio of this event's zone of\ninfluence." },
{ 161, 2, "The ellipsoid's semi-axis length, in\nmetres, along the model's X (East-West)\ndirection." },
{ 161, 3, "The ellipsoid's semi-axis length, in\nmetres, along the model's Y\n(North-South) direction." },
{ 161, 4, "The ellipsoid's semi-axis length, in\nmetres, along the model's Z (vertical)\ndirection." },
{ 161, 5, "The ellipsoid's semi-axis length, in\nmetres, along the model's X (East-West)\ndirection." },
{ 161, 6, "The ellipsoid's semi-axis length, in\nmetres, along the model's Y\n(North-South) direction." },
{ 161, 7, "The ellipsoid's semi-axis length, in\nmetres, along the model's Z (vertical)\ndirection." },
{ 161, 8, "The ellipsoid's semi-axis length, in\nmetres, along the model's X (East-West)\ndirection." },
{ 161, 9, "The ellipsoid's semi-axis length, in\nmetres, along the model's Z (vertical)\ndirection." },
{ 161, 10, "The ellipsoid's semi-axis length, in\nmetres, along the model's Y\n(North-South) direction." },

/* ===== 162: CORNER WINDOW -- Import event's "Form/Position" tab: the
 * imported block's bounding-box corners. ===== */
{ 162, 1, "Groups the bounding-box corner controls\nfor the imported block." },
{ 162, 2, "The centre point of the imported block's\nbounding box, in model coordinates." },
{ 162, 3, "The South-West/upper corner of the\nimported block's bounding box, defining\nits extent together with the centre\npoint." },
{ 162, 4, "Chooses the file to import as a block of\npre-computed geology (e.g. from another\nNoddy history or an external model)." },

/* ===== 163: IPROP WINDOW -- Import event's "Scale/Properties" tab. ===== */
{ 163, 1, "Groups the imported block's property\ncontrols." },
{ 163, 2, "Opens the rock-property editor to assign\ndensity, susceptibility and other\npetrophysical properties to the imported\nblock." },
{ 163, 3, "How transparent the imported block is\ndrawn in 3D views, letting geology on\neither side of it remain visible." },
{ 163, 4, "How transparent the imported block is\ndrawn in 3D views, letting geology on\neither side of it remain visible." },

/* ===== 164: ISCL WINDOW -- Import event's "Scale/Properties" tab: the
 * imported block's voxel size. ===== */
{ 164, 1, "Groups the imported block's size\ncontrol." },
{ 164, 2, "The block size (voxel edge length), in\nmetres, that the imported geology is\nresampled to when merged into the model." },
{ 164, 3, "The block size (voxel edge length), in\nmetres, that the imported geology is\nresampled to when merged into the model." },
{ 164, 4, "The block size (voxel edge length), in\nmetres, that the imported geology is\nresampled to when merged into the model." },

/* ===== 165: SURFACE WINDOW -- shared grid-surface tab used by Fault,
 * Shear Zone and Unconformity's "Form (Surface)" page (e.g. for a DXF/
 * grid-based surface instead of the default profile-based one). ===== */
{ 165, 1, "Selects which view (e.g. plan,\ncross-section) the surface preview below\nis shown in." },
{ 165, 2, "Selects the surface's source/type --\ne.g. a regular grid, or a surface\nimported from an external file." },
{ 165, 3, "Groups the surface's grid dimension\ncontrols." },
{ 165, 4, "The surface grid's extent, in metres,\nalong the model's X (East-West)\ndirection." },
{ 165, 5, "The surface grid's extent, in metres,\nalong the model's X (East-West)\ndirection." },
{ 165, 6, "The surface grid's extent, in metres,\nalong the model's X (East-West)\ndirection." },
{ 165, 7, "The surface grid's extent, in metres,\nalong the model's Y (North-South)\ndirection." },
{ 165, 8, "The surface grid's extent, in metres,\nalong the model's Y (North-South)\ndirection." },
{ 165, 9, "The surface grid's extent, in metres,\nalong the model's Y (North-South)\ndirection." },
{ 165, 10, "The surface grid's extent, in metres,\nalong the vertical (Z) direction." },
{ 165, 11, "The surface grid's extent, in metres,\nalong the vertical (Z) direction." },
{ 165, 12, "The surface grid's extent, in metres,\nalong the vertical (Z) direction." },

/* ===== 168: TRACE BUTTON WINDOW -- opens the trace editor used by
 * Fold, Fault and Shear Zone to draw the feature's trace directly on a
 * map instead of (or in addition to) entering position/orientation
 * numerically. ===== */
{ 168, 1, "Opens the trace editor, letting you draw\nthis event's trace directly on a map\ninstead of entering its position and\norientation numerically." },

/* ===== 169: STRATIGRAPHY WINDOW -- manages the ordered list of rock
 * units for a Stratigraphy event (the base of the model, or a Layers
 * page on an Unconformity). ===== */
{ 169, 1, "The ordered list of rock units in this\nstratigraphic sequence, from youngest\n(top) to oldest (bottom) -- double-click\n(or Edit) a unit to set its properties." },
{ 169, 2, "Adds a new rock unit to the\nstratigraphic sequence." },
{ 169, 3, "Removes the selected rock unit from the\nstratigraphic sequence." },
{ 169, 4, "Opens the rock-property editor for the\nselected unit (name, density,\nsusceptibility, colour, etc.)." },
{ 169, 5, "Moves the selected rock unit up\n(younger) in the sequence." },
{ 169, 6, "Moves the selected rock unit down\n(older) in the sequence." },
{ 169, 7, "The ordered list of rock units in this\nstratigraphic sequence, from youngest\n(top) to oldest (bottom)." },

/* ===== 170: GENERIC WINDOW -- a reusable 10-field "Options" tab for
 * event types without a bespoke dialog (generwin.c); which fields are
 * active and what they mean depends on which event opened it. ===== */
{ 170, 1, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 2, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 3, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 4, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 5, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 6, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 7, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 8, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 9, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 10, "A parameter field whose exact meaning\ndepends on the specific event type\ncurrently open -- see this event's own\ndocumentation/history notes for what it\ncontrols." },
{ 170, 11, "The value for the parameter field of the\nsame number to the left." },
{ 170, 12, "The value for the parameter field of the\nsame number to the left." },
{ 170, 13, "The value for the parameter field of the\nsame number to the left." },
{ 170, 14, "The value for the parameter field of the\nsame number to the left." },
{ 170, 15, "The value for the parameter field of the\nsame number to the left." },
{ 170, 16, "The value for the parameter field of the\nsame number to the left." },
{ 170, 17, "The value for the parameter field of the\nsame number to the left." },
{ 170, 18, "The value for the parameter field of the\nsame number to the left." },
{ 170, 19, "The value for the parameter field of the\nsame number to the left." },
{ 170, 20, "The value for the parameter field of the\nsame number to the left." },

/* ===== 116: GEOLOGY OPTIONS WINDOW (geolOp.c) -- project-wide geology
 * settings: model origin, topography draping, and grid-to-north
 * reference angle/declination/depth. ===== */
{ 116, 1, "Applies the current settings and closes\nthis dialog." },
{ 116, 2, "Discards any changes and closes this\ndialog." },
{ 116, 3, "Opens help for geology options (not\navailable in this build)." },
{ 116, 4, "Groups the model origin's X, Y and Z\ncoordinates." },
{ 116, 5, "The model's origin (reference point)\nposition along the X (East-West) axis;\nall event positions are measured\nrelative to this point." },
{ 116, 6, "The model's origin (reference point)\nposition along the Y (North-South) axis;\nall event positions are measured\nrelative to this point." },
{ 116, 7, "The model's origin (reference point)\nposition along the Z (vertical) axis;\nall event positions are measured\nrelative to this point." },
{ 116, 8, "The model's origin (reference point)\nposition along the X (East-West) axis." },
{ 116, 9, "The model's origin (reference point)\nposition along the Y (North-South) axis." },
{ 116, 10, "The model's origin (reference point)\nposition along the Z (vertical) axis." },
{ 116, 11, "The model's origin (reference point)\nposition along the X (East-West) axis." },
{ 116, 12, "The model's origin (reference point)\nposition along the Y (North-South) axis." },
{ 116, 13, "The model's origin (reference point)\nposition along the Z (vertical) axis." },
{ 116, 14, "Groups the grid orientation, magnetic\ndeclination and depth reference\nsettings." },
{ 116, 15, "The angle between the model's internal\ngrid and true north, used to correctly\norient dip/dip-direction and other\ncompass-referenced inputs against\nreal-world geography." },
{ 116, 16, "Magnetic declination: the angle, in\ndegrees, between true north and magnetic\nnorth at the model's location, used when\norienting magnetic-field-related inputs." },
{ 116, 17, "The reference depth used together with\nthe origin's Z coordinate for\ngeophysical calculations." },
{ 116, 18, "When checked, loads and drapes a real\ntopography surface over the model's top\ninstead of using a flat surface." },
{ 116, 19, "The angle between the model's internal\ngrid and true north, used to correctly\norient dip/dip-direction and other\ncompass-referenced inputs against\nreal-world geography." },
{ 116, 20, "Magnetic declination: the angle, in\ndegrees, between true north and magnetic\nnorth at the model's location, used when\norienting magnetic-field-related inputs." },
{ 116, 21, "The reference depth used together with\nthe origin's Z coordinate for\ngeophysical calculations." },
{ 116, 22, "The angle between the model's internal\ngrid and true north, used to correctly\norient dip/dip-direction and other\ncompass-referenced inputs against\nreal-world geography." },
{ 116, 23, "The reference depth used together with\nthe origin's Z coordinate for\ngeophysical calculations." },
{ 116, 24, "Magnetic declination: the angle, in\ndegrees, between true north and magnetic\nnorth at the model's location, used when\norienting magnetic-field-related inputs." },
{ 116, 25, "When checked, shows the model as a 3D\nchair diagram (a block with front, side\nand top faces cut away) instead of a\nplain block." },
{ 116, 26, "Opens the block view options,\ncontrolling how the 3D block/chair\ndiagram is displayed (faces shown,\nlighting, etc.)." },

/* ===== 135: ROCK DATABASE WIN (rockprop.c) -- a saved library of
 * reusable rock property presets, separate from the properties on any
 * one stratigraphic unit. ===== */
{ 135, 1, "Closes the rock property database." },
{ 135, 2, "Groups the rock database list and its\ncontrols." },
{ 135, 3, "The list of saved rock property presets\nin this database -- select one to view\nor edit its properties, or to apply it\nto a stratigraphic unit." },
{ 135, 4, "Imports rock property presets from a\ndatabase file on disk, adding them to\nthis list." },
{ 135, 5, "Exports the selected rock property\npreset(s) to a database file on disk,\nfor reuse in other projects." },
{ 135, 6, "Adds a new rock property preset to the\ndatabase." },
{ 135, 7, "Removes the selected rock property\npreset from the database." },
{ 135, 8, "The name of the selected rock property\npreset." },
{ 135, 9, "Opens the rock-property editor for the\nselected preset (density,\nsusceptibility, colour, etc.)." },
{ 135, 10, "Closes the rock property database." },

/* ===== 123: GEOPHYSICS OPTIONS WINDOW (geophOp.c) -- project-wide
 * settings for the gravity/magnetic forward calculation: survey type,
 * regional field, and which physical effects to include. ===== */
{ 123, 1, "Groups the survey type and regional\nfield settings." },
{ 123, 2, "Groups the survey type and regional\nfield settings." },
{ 123, 3, "Groups the physical effects to include\nin the calculation." },
{ 123, 4, "Surface survey: computes the\ngravity/magnetic response measured at\nground level, following the model's\ntopography." },
{ 123, 5, "Airborne survey: computes the\ngravity/magnetic response measured at a\nfixed flying altitude above the ground,\nset below." },
{ 123, 6, "The regional magnetic field's total\nintensity, in nanotesla (nT), used as\nthe inducing field for magnetic forward\nmodelling." },
{ 123, 7, "The regional magnetic field's\ninclination, in degrees, used as the\ninducing field for magnetic forward\nmodelling -- negative values are a\ndownward-pointing field (as in the\nSouthern Hemisphere)." },
{ 123, 8, "Groups the survey type controls\n(Surface/Airborne)." },
{ 123, 9, "Groups the regional magnetic field\ncontrols (intensity/inclination)." },
{ 123, 10, "Applies the current geophysics settings\nand closes this dialog." },
{ 123, 11, "Discards any changes and closes this\ndialog." },
{ 123, 12, "Opens help for geophysics options (not\navailable in this build)." },
{ 123, 13, "The survey altitude, in metres above\nground -- only used (and only editable)\nfor an Airborne survey." },
{ 123, 14, "The survey altitude, in metres above\nground -- only used (and only editable)\nfor an Airborne survey." },
{ 123, 15, "When checked, the airborne survey\nfollows (drapes over) the model's real\ntopography instead of flying at a\nconstant altitude above a flat plane." },
{ 123, 16, "When checked, rocks with remanent\nmagnetization contribute their fixed\nremanent field to the calculated\nmagnetic response, on top of the induced\nresponse from the regional field." },
{ 123, 17, "When checked, rocks with anisotropic\nmagnetic susceptibility use their full\ndirectional response in the calculation\ninstead of a single averaged value." },
{ 123, 18, "The survey altitude, in metres above\nground -- only used (and only editable)\nfor an Airborne survey." },
{ 123, 19, "Regional magnetic declination: the\nangle, in degrees, between true north\nand magnetic north, used to orient the\nregional field horizontally." },
{ 123, 20, "Chooses the calculation method: Spatial\ncomputes the response directly in real\nspace; Spectral uses an FFT-based\n(frequency-domain) method, which is\noften faster for large models." },

/* ===== 192: CALC TYPE WINDOW -- chooses which geophysical fields to
 * calculate and export for the project. ===== */
{ 192, 1, "Groups the calculation type controls." },
{ 192, 2, "The list of available calculation types\n(e.g. gravity, magnetics, or both) for\nthis project." },
{ 192, 3, "Runs the calculation and exports the\nselected geophysical field(s) for the\nwhole project." },

/* ===== 193: DEFORM FIELD WINDOW (defmwin.c) -- "Deform Fields" tab on
 * a deforming event: whether it also reorients rock properties it
 * carries along, rather than just moving material. ===== */
{ 193, 1, "When checked, this event also\nrotates/translates the remanent\nmagnetization direction of any rock it\ndeforms, keeping the magnetization\nconsistent with the rock's new\norientation." },
{ 193, 2, "When checked, this event also\nrotates/translates the magnetic\nanisotropy orientation of any rock it\ndeforms, keeping the anisotropy fabric\nconsistent with the rock's new\norientation." },
{ 193, 3, "Groups the property-deformation\ncontrols: whether this event reorients\nremanence and/or anisotropy along with\nthe rock it moves." },

/* ===== 194: PADDING WINDOW (padwin.c) -- background properties for the
 * model's padding/buffer zone, used to avoid edge-effect artefacts in
 * (especially FFT-based) geophysics calculations. ===== */
{ 194, 1, "Groups the padding zone's properties:\ntype, size, and background\nsusceptibility/density." },
{ 194, 2, "The padding style used to extend the\nmodel beyond its real extent -- e.g. a\nconstant background value or a\ntapered/reflected edge -- to reduce\nedge-effect artefacts in the\ncalculation." },
{ 194, 3, "Background susceptibility (X): the\nmagnetic susceptibility assigned to the\npadding zone along the X axis, used\ninstead of real geology beyond the\nmodel's edge." },
{ 194, 4, "Background susceptibility (X): the\nmagnetic susceptibility assigned to the\npadding zone along the X axis, used\ninstead of real geology beyond the\nmodel's edge." },
{ 194, 5, "The padding style used to extend the\nmodel beyond its real extent -- e.g. a\nconstant background value or a\ntapered/reflected edge -- to reduce\nedge-effect artefacts in the\ncalculation." },
{ 194, 6, "Background susceptibility (Y): the\nmagnetic susceptibility assigned to the\npadding zone along the Y axis, used\ninstead of real geology beyond the\nmodel's edge." },
{ 194, 7, "Background susceptibility (Y): the\nmagnetic susceptibility assigned to the\npadding zone along the Y axis, used\ninstead of real geology beyond the\nmodel's edge." },
{ 194, 8, "Background susceptibility (Z): the\nmagnetic susceptibility assigned to the\npadding zone along the Z axis, used\ninstead of real geology beyond the\nmodel's edge." },
{ 194, 9, "Background susceptibility (Z): the\nmagnetic susceptibility assigned to the\npadding zone along the Z axis, used\ninstead of real geology beyond the\nmodel's edge." },
{ 194, 10, "Background density assigned to the\npadding zone, used instead of real\ngeology beyond the model's edge." },
{ 194, 11, "Background density assigned to the\npadding zone, used instead of real\ngeology beyond the model's edge." },
{ 194, 12, "Fence width: how far, in blocks or\nmetres, the padding zone extends beyond\nthe model's real edge." },
{ 194, 13, "Fence width: how far, in blocks or\nmetres, the padding zone extends beyond\nthe model's real edge." },
{ 194, 14, "Padding percentage: the padding zone's\nsize expressed as a percentage of the\nmodel's own extent, as an alternative to\na fixed fence width." },
{ 194, 15, "Padding percentage: the padding zone's\nsize expressed as a percentage of the\nmodel's own extent, as an alternative to\na fixed fence width." },

/* ===== 195: VARIABLE CUBE WINDOW -- adaptive block-size settings for
 * the 3D geology cube, refining resolution only where needed. ===== */
{ 195, 1, "The ratio controlling how much finer the\nblock size becomes in areas that are\nrefined, relative to the model's base\nblock size." },
{ 195, 2, "The ratio controlling how much finer the\nblock size becomes in areas that are\nrefined, relative to the model's base\nblock size." },
{ 195, 3, "The cutoff threshold that decides where\nthe finer, refined block size is used --\ne.g. only near geology boundaries or\nproperty contrasts above this value." },
{ 195, 4, "Groups the variable (adaptive)\nblock-size controls: refinement ratio\nand cutoff threshold." },
{ 195, 5, "The cutoff threshold that decides where\nthe finer, refined block size is used." },

/* ===== 196: SUS UNITS WINDOW -- magnetic susceptibility unit system. ===== */
{ 196, 1, "SI units: susceptibility is\ndimensionless in the SI convention\n(values are 4*pi times larger than the\nequivalent CGS value)." },
{ 196, 2, "CGS units: susceptibility is expressed\nin the older CGS (emu) convention,\ncommonly used in older rock property\ndatabases and literature." },
{ 196, 3, "Groups the susceptibility unit system\nchoice (SI or CGS) applied when\nentering/displaying rock magnetic\nsusceptibility values." },

/* ===== 197: GEOPHYSICS SCALE WINDOW -- display scaling for a
 * gravity/magnetic image, analogous to PRINT SCALE for printed output. ===== */
{ 197, 1, "The scale factor applied when displaying\nthe calculated geophysical image (e.g.\nto stretch or compress its\ngreyscale/colour range)." },
{ 197, 2, "The scale factor applied when displaying\nthe calculated geophysical image." },
{ 197, 3, "The scale factor applied when displaying\nthe calculated geophysical image." },
{ 197, 4, "Groups the geophysics image display\nscale control." },

/* ===== 198: GEOPHYSICS IDISPLAY WINDOW -- opens the image display
 * options for each calculated geophysical field. ===== */
{ 198, 1, "Opens display options (colour scale,\ncontours, clipping) for the calculated\ngravity image." },
{ 198, 2, "Opens display options (colour scale,\ncontours, clipping) for the calculated\nmagnetics image." },

/* ===== 199: LOCAL REMOTE WINDOW -- where a calculation job runs. ===== */
{ 199, 1, "Groups the local/remote calculation\ncontrols." },
{ 199, 2, "Runs the calculation on a remote server\ninstead of this computer -- useful for\nlarge jobs, using the account/address\nset in the project options." },
{ 199, 3, "Runs the calculation on this computer." },

/* ===== 200: UNIT WINDOW -- model length unit. ===== */
{ 200, 1, "Uses metres as the length unit for all\nmodel coordinates and dimensions." },
{ 200, 2, "Uses feet as the length unit for all\nmodel coordinates and dimensions." },
{ 200, 3, "Groups the length unit choice (metres or\nfeet) applied throughout the model." },

/* ===== 105: PROFILE WINDOW -- the graphical curve editor opened by any
 * "Profile"/"Options" button (Fold Form, Fault/Shear Form, Dyke/Plug
 * alteration, ...) to draw a custom shape instead of using a preset. ===== */
{ 105, 1, "Applies the profile curve as drawn and\ncloses the editor." },
{ 105, 2, "Discards changes to the profile curve\nand closes the editor." },
{ 105, 3, "Opens help for the profile editor (not\navailable in this build)." },
{ 105, 4, "Groups the profile curve's point-joining\nstyle controls." },
{ 105, 5, "Joins the profile's control points with\nstraight line segments." },
{ 105, 6, "Joins the profile's control points with\na smooth curve." },
{ 105, 7, "Joins the profile's control points with\nsquare (step-like) segments." },
{ 105, 8, "Removes the currently selected control\npoint from the profile curve." },
{ 105, 9, "Removes every control point from the\nprofile curve, resetting it." },
{ 105, 10, "The profile curve itself -- click to add\nor drag control points, shaping the\ncustom cross-section this event will use\ninstead of a preset." },
{ 105, 11, "Shows the graph coordinates of the point\ncurrently under the cursor." },
{ 105, 12, "The profile curve itself -- click to add\nor drag control points, shaping the\ncustom cross-section this event will use\ninstead of a preset." },
{ 105, 13, "The X (horizontal) coordinate of the\npoint currently under the cursor, in the\nprofile's own graph units." },
{ 105, 14, "The Y (vertical) coordinate of the point\ncurrently under the cursor, in the\nprofile's own graph units." },
{ 105, 15, "Opens the profile's display/scale\noptions (axis range, snap settings)." },
{ 105, 16, "Scrolls the visible portion of the\nprofile graph horizontally." },

/* ===== 136: PROFILE OPTION WINDOW -- display/scale settings for the
 * profile curve editor above. ===== */
{ 136, 1, "Groups the profile display and\naxis-scale controls." },
{ 136, 2, "Toggles a display option for the profile\ngraph (e.g. showing a reference grid)." },
{ 136, 3, "Toggles a display option for the profile\ngraph (e.g. locking an axis while\ndragging points)." },
{ 136, 4, "Toggles a display option for the profile\ngraph (e.g. snapping points to the\ngrid)." },
{ 136, 5, "Zooms the profile graph's visible axis\nrange." },
{ 136, 6, "Zooms the profile graph's visible axis\nrange." },
{ 136, 7, "The profile graph's minimum axis value." },
{ 136, 8, "The profile graph's maximum axis value." },
{ 136, 9, "The value used to replace points below\nthe minimum scale." },
{ 136, 10, "The value used to replace points above\nthe maximum scale." },
{ 136, 11, "An alternate distribution option label\nfor the profile's control points." },
{ 136, 12, "The profile graph's minimum axis value\n-- points below this are clipped or\nreplaced." },
{ 136, 13, "The profile graph's maximum axis value\n-- points above this are clipped or\nreplaced." },
{ 136, 14, "The value used to replace points that\nfall below the minimum scale." },
{ 136, 15, "The value used to replace points that\nfall above the maximum scale." },
{ 136, 16, "An alternate distribution option label\nfor the profile's control points." },
{ 136, 17, "Redistributes the profile's control\npoints using an alternative\n(non-uniform) spacing along the curve." },

/* ===== 166: FOLD TRACE WINDOW (tracewin.c) -- draws the fold's hinge
 * trace on a map instead of entering position/orientation numerically;
 * Top/Bottom reference points anchor the trace to a real depth range. ===== */
{ 166, 1, "Adds a new point to the fold trace being\ndrawn on the map." },
{ 166, 2, "Removes the currently selected point\nfrom the fold trace." },
{ 166, 3, "Applies the drawn trace, converting it\ninto this fold's position and\norientation, and closes the editor." },
{ 166, 4, "Discards the drawn trace and closes the\neditor." },
{ 166, 5, "Opens help for the trace editor (not\navailable in this build)." },
{ 166, 6, "Groups the trace-drawing controls\n(add/delete point)." },
{ 166, 7, "Groups the trace's reference depth range\n(top and bottom points), which anchors\nthe drawn map trace to real 3D geometry." },
{ 166, 8, "The X coordinate of the trace's upper\nreference point." },
{ 166, 9, "The Y coordinate of the trace's upper\nreference point." },
{ 166, 10, "The Z (depth) coordinate of the trace's\nupper reference point." },
{ 166, 11, "The X coordinate of the trace's upper\nreference point." },
{ 166, 12, "The Z (depth) coordinate of the trace's\nupper reference point." },
{ 166, 13, "The Y coordinate of the trace's upper\nreference point." },
{ 166, 14, "The X coordinate of the trace's upper\nreference point." },
{ 166, 15, "The Y coordinate of the trace's upper\nreference point." },
{ 166, 16, "The Z (depth) coordinate of the trace's\nupper reference point." },
{ 166, 17, "The X coordinate of the trace's lower\nreference point." },
{ 166, 18, "The Y coordinate of the trace's lower\nreference point." },
{ 166, 19, "The Z (depth) coordinate of the trace's\nlower reference point." },
{ 166, 20, "The X coordinate of the trace's lower\nreference point." },

/* ===== 167: FAULT TRACE WINDOW (tracewin.c) -- same idea as Fold Trace,
 * for a fault's map trace. ===== */
{ 167, 1, "Applies the drawn trace, converting it\ninto this fault's position and\norientation, and closes the editor." },
{ 167, 2, "Discards the drawn trace and closes the\neditor." },
{ 167, 3, "Opens help for the trace editor (not\navailable in this build)." },
{ 167, 4, "Groups the trace's reference depth range\n(top and bottom points), which anchors\nthe drawn map trace to real 3D geometry." },
{ 167, 5, "The X coordinate of the trace's upper\nreference point." },
{ 167, 6, "The Y coordinate of the trace's upper\nreference point." },
{ 167, 7, "The Z (depth) coordinate of the trace's\nupper reference point." },
{ 167, 8, "The X coordinate of the trace's upper\nreference point." },
{ 167, 9, "The Z (depth) coordinate of the trace's\nupper reference point." },
{ 167, 10, "The Y coordinate of the trace's upper\nreference point." },
{ 167, 11, "The X coordinate of the trace's upper\nreference point." },
{ 167, 12, "The Y coordinate of the trace's upper\nreference point." },
{ 167, 13, "The Z (depth) coordinate of the trace's\nupper reference point." },
{ 167, 14, "The X coordinate of the trace's lower\nreference point." },
{ 167, 15, "The Y coordinate of the trace's lower\nreference point." },
{ 167, 16, "The Z (depth) coordinate of the trace's\nlower reference point." },
{ 167, 17, "The X coordinate of the trace's lower\nreference point." },
{ 167, 18, "The Z (depth) coordinate of the trace's\nlower reference point." },
{ 167, 19, "The Y coordinate of the trace's lower\nreference point." },

/* ===== 117: VIEW OPTIONS WINDOW -- 3D view direction/scale and which
 * stratigraphic surfaces are drawn. ===== */
{ 117, 1, "Declination: the compass direction the\n3D view is looking from, in degrees." },
{ 117, 2, "Elevation: the angle, in degrees, the 3D\nview is tilted up or down from\nhorizontal." },
{ 117, 3, "Scale: the zoom level of the 3D view." },
{ 117, 4, "Declination: the compass direction the\n3D view is looking from, in degrees." },
{ 117, 5, "Elevation: the angle, in degrees, the 3D\nview is tilted up or down from\nhorizontal." },
{ 117, 6, "Scale: the zoom level of the 3D view." },
{ 117, 7, "Declination: the compass direction the\n3D view is looking from, in degrees." },
{ 117, 8, "Elevation: the angle, in degrees, the 3D\nview is tilted up or down from\nhorizontal." },
{ 117, 9, "Scale: the zoom level of the 3D view." },
{ 117, 10, "Groups the 3D view's viewing angle and\nzoom controls." },
{ 117, 11, "Applies the current view settings and\ncloses this dialog." },
{ 117, 12, "Discards any changes and closes this\ndialog." },
{ 117, 13, "Opens help for view options (not\navailable in this build)." },
{ 117, 14, "Shows a live preview of the 3D view with\nthe current settings applied." },
{ 117, 15, "Shows which stratigraphic surfaces are\ncurrently selected to be drawn." },
{ 117, 16, "When checked, every stratigraphic\nsurface is drawn, instead of only the\nones selected in the list below." },
{ 117, 17, "Groups the surface-selection controls." },
{ 117, 18, "Selects which surfaces (by type) are\navailable to choose from below." },
{ 117, 19, "Selects which surfaces (by type) are\navailable to choose from below." },
{ 117, 20, "Selects which surfaces (by type) are\navailable to choose from below." },

/* ===== 176: VIEW WINDOW -- the visible view volume's dimensions and
 * origin (a sub-region of the full model to display/export). ===== */
{ 176, 1, "Shows a live preview of the view volume\nwith the current dimension/origin\nsettings applied." },
{ 176, 2, "Groups the view volume's size (X/Y/Z\ndimension) controls." },
{ 176, 3, "The view volume's extent along the X\n(East-West) axis." },
{ 176, 4, "The view volume's extent along the Y\n(North-South) axis." },
{ 176, 5, "The view volume's extent along the Z\n(vertical) axis." },
{ 176, 6, "The view volume's extent along the X\n(East-West) axis." },
{ 176, 7, "The view volume's extent along the Y\n(North-South) axis." },
{ 176, 8, "The view volume's extent along the Z\n(vertical) axis." },
{ 176, 9, "The view volume's extent along the X\n(East-West) axis." },
{ 176, 10, "The view volume's extent along the Z\n(vertical) axis." },
{ 176, 11, "The view volume's extent along the Y\n(North-South) axis." },
{ 176, 12, "Groups the view volume's origin (X/Y/Z\nposition) controls." },
{ 176, 13, "The view volume's origin position along\nthe X (East-West) axis." },
{ 176, 14, "The view volume's origin position along\nthe Y (North-South) axis." },
{ 176, 15, "The view volume's origin position along\nthe Z (vertical) axis." },
{ 176, 16, "The view volume's origin position along\nthe X (East-West) axis." },
{ 176, 17, "The view volume's origin position along\nthe Y (North-South) axis." },
{ 176, 18, "The view volume's origin position along\nthe Z (vertical) axis." },
{ 176, 19, "The view volume's origin position along\nthe X (East-West) axis." },
{ 176, 20, "The view volume's origin position along\nthe Z (vertical) axis." },

/* ===== 184: CHAIR WINDOW -- position of the cut planes for the 3D
 * "chair diagram" (a block with front/side/top faces cut away to reveal
 * internal geology), toggled from Geology Options. ===== */
{ 184, 1, "Groups the chair diagram's cut-plane\nposition controls." },
{ 184, 2, "The X position of the chair diagram's\ncut plane, in model coordinates." },
{ 184, 3, "The Y position of the chair diagram's\ncut plane, in model coordinates." },
{ 184, 4, "The Z (depth) position of the chair\ndiagram's cut plane, in model\ncoordinates." },
{ 184, 5, "The X position of the chair diagram's\ncut plane, in model coordinates." },
{ 184, 6, "The Y position of the chair diagram's\ncut plane, in model coordinates." },
{ 184, 7, "The Z (depth) position of the chair\ndiagram's cut plane, in model\ncoordinates." },
{ 184, 8, "The X position of the chair diagram's\ncut plane, in model coordinates." },
{ 184, 9, "The Z (depth) position of the chair\ndiagram's cut plane, in model\ncoordinates." },
{ 184, 10, "The Y position of the chair diagram's\ncut plane, in model coordinates." },
{ 184, 11, "When checked, the 3D block is displayed\nas a chair diagram (cut away on the near\nfaces to reveal internal geology)\ninstead of a plain solid block." },

/* ===== 185: THREED VIEW WINDOW -- the 3D perspective's viewing angle
 * and zoom (same concept as View Options' declination/elevation/scale,
 * exposed as its own standalone dialog). ===== */
{ 185, 1, "Declination: the compass direction the\n3D view is looking from, in degrees." },
{ 185, 2, "Azimuth: the compass direction the 3D\nview is looking from, in degrees." },
{ 185, 3, "Scale: the zoom level of the 3D view, as\na percentage." },
{ 185, 4, "Declination: the compass direction the\n3D view is looking from, in degrees." },
{ 185, 5, "Azimuth: the compass direction the 3D\nview is looking from, in degrees." },
{ 185, 6, "Scale: the zoom level of the 3D view, as\na percentage." },
{ 185, 7, "Declination: the compass direction the\n3D view is looking from, in degrees." },
{ 185, 8, "Azimuth: the compass direction the 3D\nview is looking from, in degrees." },
{ 185, 9, "Scale: the zoom level of the 3D view, as\na percentage." },
{ 185, 10, "Groups the 3D view's viewing angle and\nzoom controls." },
{ 185, 11, "Scale: the zoom level of the 3D view, as\na percentage." },

/* ===== 132: BLOCK CONVERT WINDOW -- mirrors the calculated block model
 * along one or more axes, e.g. to match a real-world survey's handedness
 * or orientation convention. ===== */
{ 132, 1, "Groups the axis-flip controls for\nconverting the block model." },
{ 132, 2, "Mirrors the block model along the X\n(East-West) axis." },
{ 132, 3, "Mirrors the block model along the Y\n(North-South) axis." },
{ 132, 4, "Mirrors the block model along the Z\n(vertical) axis." },
{ 132, 5, "Applies the selected axis flips and\ncloses the dialog." },
{ 132, 6, "Shows a preview of the block model with\nthe selected axis flips applied." },
{ 132, 7, "Shows a preview of the block model with\nthe selected axis flips applied." },

/* ===== 211: SECTION WINDOW -- a vertical cross-section's line and
 * depth extent through the model. ===== */
{ 211, 1, "Groups the section line's direction,\nposition and length controls." },
{ 211, 2, "Declination: the compass direction, in\ndegrees, the section line runs -- the\ncross-section is drawn as if looking\nalong this line." },
{ 211, 3, "Length: how far, in metres, the section\nline extends." },
{ 211, 4, "Declination: the compass direction, in\ndegrees, the section line runs." },
{ 211, 5, "Length: how far, in metres, the section\nline extends." },
{ 211, 6, "Length: how far, in metres, the section\nline extends." },
{ 211, 7, "Declination: the compass direction, in\ndegrees, the section line runs." },
{ 211, 8, "The section line's starting position\nalong the X (East-West) axis." },
{ 211, 9, "The section line's starting position\nalong the Y (North-South) axis." },
{ 211, 10, "The section line's starting position\nalong the Z (vertical) axis." },
{ 211, 11, "The section line's starting position\nalong the X (East-West) axis." },
{ 211, 12, "The section line's starting position\nalong the Y (North-South) axis." },
{ 211, 13, "The section line's starting position\nalong the Z (vertical) axis." },
{ 211, 14, "The section line's starting position\nalong the X (East-West) axis." },
{ 211, 15, "The section line's starting position\nalong the Z (vertical) axis." },
{ 211, 16, "The section line's starting position\nalong the Y (North-South) axis." },
{ 211, 17, "Height: how far, in metres, the\ncross-section extends vertically." },
{ 211, 18, "Height: how far, in metres, the\ncross-section extends vertically." },
{ 211, 19, "Height: how far, in metres, the\ncross-section extends vertically." },

/* ===== 216: VOLUME WINDOW -- displays a calculated volume. ===== */
{ 216, 1, "The calculated volume of the selected\nrock unit or region, in cubic metres." },
{ 216, 2, "Groups the calculated volume display." },

/* ===== 217: HELP OPTION WINDOW -- which external viewer opens help
 * content. ===== */
{ 217, 1, "Groups the help viewer choice." },
{ 217, 2, "The application used to display help\ncontent." },
{ 217, 3, "The application used to display help\ncontent (e.g. a web browser or PDF\nviewer)." },
{ 217, 4, "Information about the selected help\nviewer." },

/* ===== 203: VIEW NAME WINDOW -- manages saved 3D view presets. ===== */
{ 203, 1, "The name to save the current 3D view\nunder." },
{ 203, 2, "Deletes the selected saved view." },
{ 203, 3, "Groups the saved-view controls." },
{ 203, 4, "The list of saved 3D view presets --\nselect one to restore that viewing angle\nand zoom." },

/* ===== 204: CUBE SIZE WINDOW -- block resolution, set separately for
 * the geology model and the (often coarser, for performance) geophysics
 * calculation grid. ===== */
{ 204, 1, "Groups the block size controls for\ngeology and geophysics." },
{ 204, 2, "The block size (voxel edge length), in\nmetres, used for the geology model --\nsmaller values give finer detail at the\ncost of more memory/computation." },
{ 204, 3, "The block size (voxel edge length), in\nmetres, used for the geophysics\ncalculation grid -- often coarser than\nthe geology grid since gravity/magnetic\nresponses vary more smoothly." },
{ 204, 4, "The block size (voxel edge length), in\nmetres, used for the geology model." },
{ 204, 5, "The block size (voxel edge length), in\nmetres, used for the geophysics\ncalculation grid." },

/* ===== 205: ORIGIN WINDOW -- the model's origin (reference point). ===== */
{ 205, 1, "Groups the model origin's X, Y and Z\ncoordinates." },
{ 205, 2, "The model's origin (reference point)\nposition along the X (East-West) axis;\nall event positions are measured\nrelative to this point." },
{ 205, 3, "The model's origin (reference point)\nposition along the Y (North-South) axis;\nall event positions are measured\nrelative to this point." },
{ 205, 4, "The model's origin (reference point)\nposition along the Z (vertical) axis;\nall event positions are measured\nrelative to this point." },
{ 205, 5, "The model's origin (reference point)\nposition along the X (East-West) axis." },
{ 205, 6, "The model's origin (reference point)\nposition along the Y (North-South) axis." },
{ 205, 7, "The model's origin (reference point)\nposition along the Z (vertical) axis." },
{ 205, 8, "The model's origin (reference point)\nposition along the X (East-West) axis." },
{ 205, 9, "The model's origin (reference point)\nposition along the Z (vertical) axis." },
{ 205, 10, "The model's origin (reference point)\nposition along the Y (North-South) axis." },

/* ===== 206: DIMENSION WINDOW -- the model's overall block-model size. ===== */
{ 206, 1, "Groups the model's overall X, Y and Z\ndimensions." },
{ 206, 2, "The model's overall extent along the X\n(East-West) axis, in metres." },
{ 206, 3, "The model's overall extent along the Y\n(North-South) axis, in metres." },
{ 206, 4, "The model's overall extent along the Z\n(vertical) axis, in metres." },
{ 206, 5, "The model's overall extent along the X\n(East-West) axis, in metres." },
{ 206, 6, "The model's overall extent along the Y\n(North-South) axis, in metres." },
{ 206, 7, "The model's overall extent along the Z\n(vertical) axis, in metres." },
{ 206, 8, "The model's overall extent along the X\n(East-West) axis, in metres." },
{ 206, 9, "The model's overall extent along the Z\n(vertical) axis, in metres." },
{ 206, 10, "The model's overall extent along the Y\n(North-South) axis, in metres." },

/* ===== 207: USE TOPOGRAPHY WINDOW ===== */
{ 207, 1, "When checked, a loaded topography\nsurface is draped over the model's top\ninstead of using a flat surface." },

/* ===== 208: FALSE ORIGIN WINDOW -- a surveying-style false easting/
 * northing offset for georeferencing the model. ===== */
{ 208, 1, "Groups the false origin's easting and\nnorthing offsets." },
{ 208, 2, "False easting: an offset added to the\nmodel's X coordinate when reporting or\nexporting real-world (e.g. UTM)\ncoordinates." },
{ 208, 3, "False northing: an offset added to the\nmodel's Y coordinate when reporting or\nexporting real-world (e.g. UTM)\ncoordinates." },
{ 208, 4, "False easting: an offset added to the\nmodel's X coordinate when reporting or\nexporting real-world (e.g. UTM)\ncoordinates." },
{ 208, 5, "False northing: an offset added to the\nmodel's Y coordinate when reporting or\nexporting real-world (e.g. UTM)\ncoordinates." },

/* ===== 125: TOPO OPTIONS WINDOW -- display settings for a loaded
 * topography surface (topoOp.c). ===== */
{ 125, 1, "Label for the topography display scale\nfield." },
{ 125, 2, "Label for the topography display offset\nfield." },
{ 125, 3, "Label for the contour interval field." },
{ 125, 4, "Label for the topography display range\nfields." },
{ 125, 5, "The vertical scale factor applied when\ndisplaying the topography surface." },
{ 125, 6, "A vertical offset added to the\ntopography surface before display." },
{ 125, 7, "When checked, elevation contour lines\nare drawn on the topography surface." },
{ 125, 8, "The elevation interval, in metres,\nbetween successive contour lines." },
{ 125, 9, "Applies the current topography display\nsettings and closes this dialog." },
{ 125, 10, "Discards any changes and closes this\ndialog." },
{ 125, 11, "Opens help for topography options (not\navailable in this build)." },
{ 125, 12, "Uses the topography data's full\nelevation range when choosing display\ncolours/shading." },
{ 125, 13, "Groups the elevation range controls used\nfor display colours/shading." },
{ 125, 14, "Uses a manually adjusted elevation range\n(instead of the full data range) when\nchoosing display colours/shading." },

/* ===== 133: XYZ GENERATION WINDOW (xyzgen.c) -- generates a regular
 * grid/profile of sample points (e.g. for synthetic drillholes or
 * geochemical samples) across the model. ===== */
{ 133, 1, "When checked, sample points are\ngenerated along regularly spaced lines\ninstead of a single point/profile." },
{ 133, 2, "Point spacing: the distance, in metres,\nbetween generated sample points along\neach line." },
{ 133, 3, "Declination: the compass direction, in\ndegrees, the generation lines run." },
{ 133, 4, "Point spacing: the distance, in metres,\nbetween generated sample points along\neach line." },
{ 133, 5, "Declination: the compass direction, in\ndegrees, the generation lines run." },
{ 133, 6, "Point spacing: the distance, in metres,\nbetween generated sample points along\neach line." },
{ 133, 7, "Declination: the compass direction, in\ndegrees, the generation lines run." },
{ 133, 8, "Generates the sample points with the\ncurrent settings and closes this dialog." },
{ 133, 9, "Discards any changes and closes this\ndialog." },
{ 133, 10, "Opens help for XYZ generation (not\navailable in this build)." },
{ 133, 11, "An additional generation option (e.g.\nwhether to include a header row) for the\nsample point output." },
{ 133, 12, "Label for the reference position's X\ncoordinate field." },
{ 133, 13, "Label for the reference position's Y\ncoordinate field." },
{ 133, 14, "Label for the reference position's Z\ncoordinate field." },
{ 133, 15, "The reference position's X coordinate,\nfrom which sample point generation\nbegins." },
{ 133, 16, "The reference position's Z coordinate,\nfrom which sample point generation\nbegins." },
{ 133, 17, "The reference position's Y coordinate,\nfrom which sample point generation\nbegins." },
{ 133, 18, "Scrolls/adjusts the reference position's\nX coordinate." },

/* ===== 134: XYZ CONVERT WINDOW (xyzConvt.c) -- maps which columns in
 * an imported/exported text file hold X/Y/Z, plus coordinate offsets. ===== */
{ 134, 1, "Shows a preview of the converted XYZ\ndata with the current column mapping and\noffsets applied." },
{ 134, 2, "Groups the column mapping and coordinate\noffset controls." },
{ 134, 3, "Converts the file using the current\ncolumn mapping and offsets, and closes\nthis dialog." },
{ 134, 4, "Discards any changes and closes this\ndialog." },
{ 134, 5, "Opens help for XYZ conversion (not\navailable in this build)." },
{ 134, 6, "Label for the X column field." },
{ 134, 7, "Label for the Y column field." },
{ 134, 8, "Label for the Z column field." },
{ 134, 9, "Label for the X coordinate offset field." },
{ 134, 10, "Label for the Y coordinate offset field." },
{ 134, 11, "Label for the Z coordinate offset field." },
{ 134, 12, "Which column in the file holds the X\ncoordinate." },
{ 134, 13, "Which column in the file holds the Y\ncoordinate." },
{ 134, 14, "Which column in the file holds the Z\ncoordinate." },
{ 134, 15, "An offset added to every X value on\nconversion, e.g. to shift into model\ncoordinates." },
{ 134, 16, "An offset added to every Y value on\nconversion, e.g. to shift into model\ncoordinates." },
{ 134, 17, "An offset added to every Z value on\nconversion, e.g. to shift into model\ncoordinates." },

/* ===== 179: ASCII IMAGE OPTIONS -- import settings for an ASCII grid
 * raster (e.g. a DEM or an externally computed gravity/magnetic grid). ===== */
{ 179, 1, "Label for the number-of-rows field." },
{ 179, 2, "Label for the number-of-columns field." },
{ 179, 3, "Label for the grid cell size field." },
{ 179, 4, "The number of rows in the ASCII grid." },
{ 179, 5, "The number of columns in the ASCII grid." },
{ 179, 6, "The grid's cell size, in metres -- the\nreal-world distance each row/column step\nrepresents." },
{ 179, 7, "Groups the grid dimension controls\n(rows, columns, cell size)." },
{ 179, 8, "Discards this import and closes the\ndialog." },
{ 179, 9, "Imports the ASCII grid with the current\nsettings and closes the dialog." },
{ 179, 10, "Opens help for ASCII image import (not\navailable in this build)." },
{ 179, 11, "Groups the grid's real-world\nregistration (georeferencing) controls." },
{ 179, 12, "Easting: the real-world X (East-West)\ncoordinate of the grid's registration\ncorner." },
{ 179, 13, "Northing: the real-world Y (North-South)\ncoordinate of the grid's registration\ncorner." },
{ 179, 14, "Label for the easting registration\nfield." },
{ 179, 15, "Label for the northing registration\nfield." },
{ 179, 16, "Label for the grid registration\ncontrols." },
{ 179, 17, "Groups the regional field parameters\nused if this grid represents a\ngeophysical (gravity/magnetic) image." },
{ 179, 18, "Regional magnetic field inclination, in\ndegrees, used if this grid is a magnetic\nimage." },
{ 179, 19, "Regional magnetic field declination, in\ndegrees, used if this grid is a magnetic\nimage." },
{ 179, 20, "Regional magnetic field total intensity,\nin nanotesla, used if this grid is a\nmagnetic image." },
{ 179, 21, "The survey altitude, in metres, this\ngrid was measured/computed at." },
{ 179, 22, "Groups the regional field parameters." },
{ 179, 23, "Label for the regional field declination\nfield." },
{ 179, 24, "Label for the regional field intensity\nfield." },
{ 179, 25, "Label for the survey altitude field." },
{ 179, 26, "Groups the grid's data type choice:\nGravity or Magnetics." },
{ 179, 27, "This grid represents a gravity\ndataset/image." },
{ 179, 28, "This grid represents a magnetic\ndataset/image." },
{ 179, 29, "When checked, the grid's rows and\ncolumns are transposed on import, e.g.\nto correct a mismatched file\norientation." },

/* ===== 180: DXF IMPORT WINDOW -- imports a DXF file's geometry into
 * the model within a specified bounding box. ===== */
{ 180, 1, "Label for the DXF data's minimum/maximum\nX extent fields." },
{ 180, 2, "Label for the DXF data's minimum/maximum\nY extent fields." },
{ 180, 3, "Label for the DXF data's minimum/maximum\nZ extent fields." },
{ 180, 4, "The DXF data's minimum X coordinate, as\nread from the file." },
{ 180, 5, "The DXF data's minimum Y coordinate, as\nread from the file." },
{ 180, 6, "The DXF data's minimum Z coordinate, as\nread from the file." },
{ 180, 7, "The DXF data's maximum X coordinate, as\nread from the file." },
{ 180, 8, "The DXF data's maximum Y coordinate, as\nread from the file." },
{ 180, 9, "The DXF data's maximum Z coordinate, as\nread from the file." },
{ 180, 10, "Groups the DXF data's bounding box and\ntarget-position controls." },
{ 180, 11, "The model X coordinate the DXF data's\norigin is placed at." },
{ 180, 12, "The model Y coordinate the DXF data's\norigin is placed at." },
{ 180, 13, "The model Z coordinate the DXF data's\norigin is placed at." },
{ 180, 14, "Label for the block size field." },
{ 180, 15, "The block size (voxel edge length), in\nmetres, the imported DXF geometry is\nresampled to." },
{ 180, 16, "The resulting number of blocks (voxels)\nthe imported DXF geometry will occupy,\ngiven the block size above." },
{ 180, 17, "Applies the current settings and imports\nthe DXF file into the model." },
{ 180, 18, "Discards this import and closes the\ndialog." },
{ 180, 19, "Opens help for DXF import (not available\nin this build)." },

/* ===== 181: ASSIGN PROP WINDOW -- assigns a rock property to imported/
 * selected geology. ===== */
{ 181, 1, "The list of available rock properties to\nassign." },
{ 181, 2, "The value to assign for the selected\nproperty." },
{ 181, 3, "Applies the property assignment and\ncloses this dialog." },
{ 181, 4, "Discards any changes and closes this\ndialog." },
{ 181, 5, "Opens help for property assignment (not\navailable in this build)." },

/* ===== 182: FILE FORMAT WINDOW -- column order and axis-direction
 * conventions used when importing/exporting XYZ-style text files. ===== */
{ 182, 1, "Groups the file's column order controls\n-- which column position holds X, Y and\nZ." },
{ 182, 2, "Label for the X column-position choice." },
{ 182, 3, "Label for the Y column-position choice." },
{ 182, 4, "Label for the Z column-position choice." },
{ 182, 5, "X is read from/written to the file's 1st\ncolumn." },
{ 182, 6, "X is read from/written to the file's 2nd\ncolumn." },
{ 182, 7, "X is read from/written to the file's 3rd\ncolumn." },
{ 182, 8, "Y is read from/written to the file's 1st\ncolumn." },
{ 182, 9, "Y is read from/written to the file's 2nd\ncolumn." },
{ 182, 10, "Y is read from/written to the file's 3rd\ncolumn." },
{ 182, 11, "Z is read from/written to the file's 1st\ncolumn." },
{ 182, 12, "Z is read from/written to the file's 2nd\ncolumn." },
{ 182, 13, "Z is read from/written to the file's 3rd\ncolumn." },
{ 182, 14, "Groups the file's axis-direction\nconvention controls -- which way each\naxis increases." },
{ 182, 15, "Label for the X axis direction choice." },
{ 182, 16, "Label for the Y axis direction choice." },
{ 182, 17, "Label for the Z axis direction choice." },
{ 182, 18, "X values increase from East to West in\nthe file." },
{ 182, 19, "X values increase from West to East in\nthe file." },
{ 182, 20, "Y values increase from North to South in\nthe file." },
{ 182, 21, "Y values increase from South to North in\nthe file." },
{ 182, 22, "Z values increase from Up to Down\n(depth) in the file." },
{ 182, 23, "Z values increase from Down to Up\n(elevation) in the file." },
{ 182, 24, "Applies the current file format settings\nand closes this dialog." },
{ 182, 25, "Discards any changes and closes this\ndialog." },
{ 182, 26, "Opens help for file format options (not\navailable in this build)." },
{ 182, 27, "Which rock property the file's extra\ndata column represents." },
{ 182, 28, "Groups the property-column controls." },
{ 182, 29, "When checked, a single constant value is\nused for every point instead of reading\na per-point value from the file." },

/* ===== 188: FIELD WINDOW -- an alternative regional-field
 * specification supporting a spatially variable field, not just a
 * single fixed one. ===== */
{ 188, 1, "Label for the regional field inclination\nfield." },
{ 188, 2, "Label for the regional field intensity\nfield." },
{ 188, 3, "Regional magnetic field total intensity,\nin nanotesla." },
{ 188, 4, "Regional magnetic field inclination, in\ndegrees." },
{ 188, 5, "Label for the regional field declination\nfield." },
{ 188, 6, "Regional magnetic field declination, in\ndegrees -- the angle between true north\nand magnetic north." },
{ 188, 7, "Groups the regional field type and value\ncontrols." },
{ 188, 8, "Uses a single fixed regional field (the\nsame declination/inclination/intensity\neverywhere in the model)." },
{ 188, 9, "Uses a spatially variable regional\nfield, read from a reference\nposition/grid instead of a single fixed\nvalue." },
{ 188, 10, "Label for the variable field's reference\nX position." },
{ 188, 11, "Label for the variable field's reference\nY position." },
{ 188, 12, "Label for the variable field's reference\nZ position." },
{ 188, 13, "The variable field's reference position\nalong the X axis." },
{ 188, 14, "The variable field's reference position\nalong the Y axis." },
{ 188, 15, "The variable field's reference position\nalong the Z axis." },
{ 188, 16, "The variable field's reference position\n(X, Y, Z)." },
{ 188, 17, "Regional magnetic field declination, in\ndegrees, at the reference position." },
{ 188, 18, "Regional magnetic field inclination, in\ndegrees, at the reference position." },

/* ===== 189: ALTITUDE WINDOW -- survey type/height, reused wherever a
 * standalone altitude setting is needed (e.g. an anomaly image). ===== */
{ 189, 1, "Surface survey: the response is computed\nat ground level, following topography." },
{ 189, 2, "Airborne survey: the response is\ncomputed at a fixed flying altitude\nabove the ground, set below." },
{ 189, 3, "Groups the survey type and altitude\ncontrols." },
{ 189, 4, "Label for the altitude unit." },
{ 189, 5, "The survey altitude, in metres above\nground -- only used for an Airborne\nsurvey." },

/* ===== 190: DRAPED SURVEY WINDOW -- follow-topography toggle. ===== */
{ 190, 1, "When checked, the survey follows (drapes\nover) the model's real topography\ninstead of a flat plane at a constant\naltitude." },

/* ===== 191: RANGE WINDOW -- how far beyond the model's real geology
 * the calculation considers/pads before switching to background
 * properties (see also PADDING WINDOW/194). ===== */
{ 191, 1, "Label for the calculation range field." },
{ 191, 2, "The distance beyond the model's edge, in\nreal units, that the calculation still\nconsiders real geology before switching\nto background/padding properties." },
{ 191, 3, "Label for the calculation range's unit\n(e.g. metres)." },
{ 191, 4, "Label showing the calculation range\nexpressed in whole blocks/cubes instead\nof real units." },
{ 191, 5, "Groups the calculation range controls." },
{ 191, 6, "Label for the analytic (far-field) range\nfield." },
{ 191, 7, "The distance beyond which the\ncalculation switches to a faster,\nsimplified (analytic) approximation\ninstead of the full model." },
{ 191, 8, "Label showing the analytic range\nexpressed in whole blocks/cubes instead\nof real units." },
{ 191, 9, "Label for the analytic range's unit\n(e.g. metres)." },
{ 191, 10, "When checked, the calculation range is\nmatched exactly rather than rounded to a\nwhole number of blocks." },
{ 191, 11, "When checked, the padding zone beyond\nthe model's edge uses a copy of the real\nnearby geology instead of a uniform\nbackground value." },

/* ===== 210: BOREHOLE WINDOW -- a borehole/drillhole's collar position,
 * azimuth, dip and length. ===== */
{ 210, 1, "Groups the borehole's azimuth, length,\ndip and position controls." },
{ 210, 2, "Label for the borehole's azimuth\n(declination) field." },
{ 210, 3, "Label for the borehole's length field." },
{ 210, 4, "Azimuth: the compass direction, in\ndegrees, the borehole is drilled\ntowards." },
{ 210, 5, "Length: the total drilled length of the\nborehole, in metres." },
{ 210, 6, "Dip: the angle, in degrees, the borehole\nis inclined below the horizontal -- 90\nis a vertical hole straight down." },
{ 210, 7, "Length: the total drilled length of the\nborehole, in metres." },
{ 210, 8, "Azimuth: the compass direction, in\ndegrees, the borehole is drilled\ntowards." },
{ 210, 9, "Label for the borehole's dip field." },
{ 210, 10, "Dip: the angle, in degrees, the borehole\nis inclined below the horizontal -- 90\nis a vertical hole straight down." },
{ 210, 11, "The borehole collar's position along the\nX (East-West) axis." },
{ 210, 12, "The borehole collar's position along the\nY (North-South) axis." },
{ 210, 13, "The borehole collar's position along the\nZ (vertical) axis -- its surface\nelevation." },
{ 210, 14, "The borehole collar's position along the\nX (East-West) axis." },
{ 210, 15, "The borehole collar's position along the\nY (North-South) axis." },
{ 210, 16, "The borehole collar's position along the\nZ (vertical) axis -- its surface\nelevation." },
{ 210, 17, "The borehole collar's position along the\nX (East-West) axis." },
{ 210, 18, "The borehole collar's position along the\nZ (vertical) axis -- its surface\nelevation." },

/* ===== 104: DEFINE COLOUR WINDOW -- a colour picker (HSV sliders plus
 * a saved-colour list), opened from any "Colour..." button (rock
 * properties, fault/dyke colour, etc). ===== */
{ 104, 1, "Hue: the base colour (red, green, blue,\n...), as an angle around the colour\nwheel." },
{ 104, 2, "Saturation: how vivid versus\nwashed-out/grey the colour is." },
{ 104, 3, "Value/brightness: how light or dark the\ncolour is." },
{ 104, 4, "Hue: the base colour (red, green, blue,\n...), as an angle around the colour\nwheel." },
{ 104, 5, "Saturation: how vivid versus\nwashed-out/grey the colour is." },
{ 104, 6, "Value/brightness: how light or dark the\ncolour is." },
{ 104, 7, "Groups the hue/saturation/brightness\nsliders." },
{ 104, 8, "A preview swatch of the colour being\nedited." },
{ 104, 9, "A preview swatch of the colour being\nedited." },
{ 104, 10, "A preview swatch of the colour being\nedited." },
{ 104, 11, "A list of previously used or saved\ncolours -- select one to reuse it\ninstead of mixing a new one." },
{ 104, 12, "Groups the saved colour list." },
{ 104, 13, "Label for the saved colour list." },
{ 104, 14, "Label for the colour preview swatch." },

/* ===== 138: PREVIEW WINDOW -- the small live 3D preview shown inside
 * an event dialog (see EVENT WINDOW/137's Preview button). ===== */
{ 138, 1, "Chooses what the preview shows (e.g.\nfull model, current event only) and its\ndisplay options." },
{ 138, 2, "Chooses what the preview shows (e.g.\nfull model, current event only)." },
{ 138, 3, "Redraws the preview to reflect the\ndialog's current parameter values." },

/* ===== 172: PREVIEW MENU WINDOW -- chooses which of several saved
 * viewing angles the preview above uses. ===== */
{ 172, 1, "Shows the preview from the first saved\nviewing angle." },
{ 172, 2, "Shows the preview from the second saved\nviewing angle." },
{ 172, 3, "Shows the preview from the third saved\nviewing angle." },

/* ===== 174: IMAGE OPTION WINDOW -- display and export/registration
 * settings for a calculated or imported geophysics/other image. ===== */
{ 174, 1, "Label for the display clipping maximum." },
{ 174, 2, "Label for the display clipping minimum." },
{ 174, 3, "The maximum value shown in the image's\ncolour/greyscale range -- values above\nthis are clipped to the top colour." },
{ 174, 4, "The minimum value shown in the image's\ncolour/greyscale range -- values below\nthis are clipped to the bottom colour." },
{ 174, 5, "The maximum value shown in the image's\ncolour/greyscale range -- values above\nthis are clipped to the top colour." },
{ 174, 6, "The minimum value shown in the image's\ncolour/greyscale range -- values below\nthis are clipped to the bottom colour." },
{ 174, 7, "Groups the image's display clipping\nrange controls." },
{ 174, 8, "Unit label for the maximum clip value." },
{ 174, 9, "Unit label for the minimum clip value." },
{ 174, 10, "Label describing the image's data range." },
{ 174, 11, "Groups the image's colour lookup table\n(LUT) controls." },
{ 174, 12, "Chooses the colour lookup table style\n(grey, pseudocolour, or contour\nbanding)." },
{ 174, 13, "Chooses the colour lookup table style\n(grey, pseudocolour, or contour\nbanding)." },
{ 174, 14, "Chooses the colour lookup table style\n(grey, pseudocolour, or contour\nbanding)." },
{ 174, 15, "Displays the image in greyscale." },
{ 174, 16, "Displays the image in a pseudocolour\n(rainbow-style) palette." },
{ 174, 17, "Displays the image as banded colour\ncontours instead of a continuous\ngradient." },
{ 174, 18, "The number of contour bands used when\nthe contour LUT style is selected." },
{ 174, 19, "The number of contour bands used when\nthe contour LUT style is selected." },
{ 174, 20, "Chooses how out-of-range values are\nclipped (e.g. hard clip versus clamped\nto the nearest in-range colour)." },
{ 174, 21, "When checked, a coordinate grid is\noverlaid on the image." },
{ 174, 22, "Groups the image's coordinate grid\ncontrols." },
{ 174, 23, "Label for the grid's starting X\ncoordinate." },
{ 174, 24, "Label for the grid's starting Y\ncoordinate." },
{ 174, 25, "Label for the grid's X spacing." },
{ 174, 26, "Label for the grid's Y spacing." },
{ 174, 27, "The grid's starting X coordinate." },
{ 174, 28, "The grid's starting Y coordinate." },
{ 174, 29, "The spacing between grid lines along X." },
{ 174, 30, "The spacing between grid lines along Y." },
{ 174, 31, "Automatically chooses grid start/spacing\nvalues from the image's own extent." },
{ 174, 32, "Uses the survey sensor spacing as the\ngrid spacing instead of a manually\nchosen value." },

/* ===== 177: LAYER DISPLAY WINDOW -- per-layer colour/display settings
 * for the 3D block/chair view. ===== */
{ 177, 1, "The list of stratigraphic layers/rock\nunits available to configure display\nfor." },
{ 177, 2, "Chooses what property the block view is\ncoloured by (e.g. rock type, density,\ncalculated field)." },
{ 177, 3, "Resets all layer colours back to their\ndefault values." },
{ 177, 4, "Edits the colour lookup table's colours\ndirectly." },
{ 177, 5, "Recalculates the displayed values (e.g.\nafter changing rock properties)." },
{ 177, 6, "Label for the colour lookup table value\nfield." },
{ 177, 7, "Groups the layer selection controls." },
{ 177, 8, "When checked, every layer is shown,\ninstead of only the ones selected in the\nlist." },
{ 177, 9, "Groups the display type controls." },
{ 177, 10, "Groups the colour lookup table (LUT)\nrange controls." },
{ 177, 11, "Label for the colour lookup table's\nstarting value." },
{ 177, 12, "Label for the colour lookup table's\nending value." },
{ 177, 13, "The value mapped to the start of the\ncolour lookup table." },
{ 177, 14, "The value mapped to the end of the\ncolour lookup table." },
{ 177, 15, "Applies the current display settings and\ncloses this dialog." },
{ 177, 16, "Opens help for layer display options\n(not available in this build)." },
{ 177, 17, "Discards any changes and closes this\ndialog." },
{ 177, 18, "The value at the current\ncursor/selection position on the colour\nlookup table." },
{ 177, 19, "Chooses how the colour lookup table is\nstretched across the data's value range\n(e.g. linear, histogram-equalized)." },
{ 177, 20, "When checked, the topography surface is\nshown overlaid on the block/chair view." },

/* ===== 186: NEWWIN WINDOW -- a one-off startup preference. ===== */
{ 186, 1, "When checked, this dialog is shown again\neach time a new window of this type is\ncreated, instead of only the first time." },

/* ===== 187: GENERATE WINDOW -- which geophysical fields to compute. ===== */
{ 187, 1, "Groups the field-generation choices." },
{ 187, 2, "Calculates the gravity response for the\nmodel." },
{ 187, 3, "Calculates the magnetic response for the\nmodel." },
{ 187, 4, "Calculates the individual X/Y/Z (or\nvertical/horizontal) components of the\nfield, in addition to the total field." },

/* ===== 209: MOVIE WINDOW -- movie generation/playback settings. ===== */
{ 209, 1, "Label for the frames-per-event field." },
{ 209, 2, "How many movie frames are generated for\neach event's transition -- more frames\ngive smoother but\nlarger/slower-to-render animations." },
{ 209, 3, "Label for the replay speed field." },
{ 209, 4, "Adjusts the movie's replay speed, from\nslow to fast." },
{ 209, 5, "Label for the fast end of the replay\nspeed slider." },
{ 209, 6, "Label for the slow end of the replay\nspeed slider." },
{ 209, 7, "The movie's replay speed." },
{ 209, 8, "Groups the movie generation controls\n(frame count and type)." },
{ 209, 9, "Chooses what the movie shows as it plays\nthrough the history (e.g. the block\nmodel, a section, or geophysics)." },
{ 209, 10, "Label for the movie content type choice." },
{ 209, 11, "Groups the movie replay controls." },

/* ===== 214: BACKGROUND WINDOW -- opens display background settings. ===== */
{ 214, 1, "Opens the background colour/appearance\nsettings for this view." },

/* ===== 215: SURFACE SELECTION WINDOW -- chooses which stratigraphic
 * surfaces are drawn (used by, e.g., the Line Map view). ===== */
{ 215, 1, "The list of stratigraphic surfaces\navailable to show or hide." },
{ 215, 2, "When checked, every surface is shown,\ninstead of only the ones selected in the\nlist." },
{ 215, 3, "Selects which surfaces (by type) are\navailable to choose from in the list." },
{ 215, 4, "Label for the surface type filter." },
{ 215, 5, "Groups the surface selection controls." },

/* ===== 218: NOISE WINDOW (noisewin.c, todo.txt #44) -- adds random
 * Gaussian noise to a calculated geophysics field, e.g. to simulate
 * realistic survey measurement noise. ===== */
{ 218, 1, "Groups the Gaussian noise controls." },
{ 218, 2, "When checked, random Gaussian noise is\nadded to the calculated field,\nsimulating realistic survey measurement\nnoise." },
{ 218, 3, "Label for the noise standard deviation\nfield." },
{ 218, 4, "Sigma: the standard deviation of the\nadded Gaussian noise, as a percentage of\nthe field's value -- larger values add\nmore noise." },
{ 218, 5, "Label for the random seed field." },
{ 218, 6, "The random number seed used to generate\nthe noise -- 0 picks a different random\nseed each time, while any other value\nreproduces the exact same noise pattern\non every run." },

/* ===== 121: PROJECT OPTIONS WINDOW (projOp.c) -- the project-wide
 * settings container: units, calculation targets, and buttons opening
 * several of the other dialogs above as sub-panels. ===== */
{ 121, 1, "Groups the project-wide unit and\ncalculation settings." },
{ 121, 2, "Applies the current project settings and\ncloses this dialog." },
{ 121, 3, "Discards any changes and closes this\ndialog." },
{ 121, 4, "Opens help for project options (not\navailable in this build)." },
{ 121, 5, "Groups the length unit choice\n(Metres/Feet)." },
{ 121, 6, "Uses metres as the length unit for all\nmodel coordinates and dimensions." },
{ 121, 7, "Uses feet as the length unit for all\nmodel coordinates and dimensions." },
{ 121, 8, "Groups the susceptibility unit choice\n(SI/CGS)." },
{ 121, 9, "SI units: susceptibility is\ndimensionless in the SI convention." },
{ 121, 10, "CGS units: susceptibility is expressed\nin the older CGS (emu) convention." },
{ 121, 11, "Groups the local/remote calculation\nchoice." },
{ 121, 12, "Runs calculations on a remote server\ninstead of this computer." },
{ 121, 13, "Runs calculations on this computer." },
{ 121, 14, "Label for the print scale field." },
{ 121, 15, "Label for the image scale field." },
{ 121, 16, "The scale factor used when printing\ndiagrams and images." },
{ 121, 17, "When checked, a new window is opened\neach time a diagram of this type is\ncreated, instead of reusing an existing\none." },
{ 121, 18, "When checked, the gravity response is\nincluded in geophysics calculations for\nthis project." },
{ 121, 19, "When checked, the magnetic response is\nincluded in geophysics calculations for\nthis project." },
{ 121, 20, "The scale factor applied when displaying\ncalculated geophysics images." },

/* ===== 124: JOB STATUS WINDOW (jobStat.c) -- shows progress for a
 * long-running calculation. ===== */
{ 124, 1, "Shows what stage of the calculation is\ncurrently running." },
{ 124, 2, "Shows what stage of the calculation is\ncurrently running." },
{ 124, 3, "The current calculation's completion\npercentage." },
{ 124, 4, "A status message describing the current\ncalculation step." },
{ 124, 5, "Stops the currently running calculation\nbefore it finishes." },

/* ===== 126: DLG 126 -- an About/info box (title/version/copyright
 * text; layout inferred from context, no unique control names were
 * recoverable). ===== */
{ 126, 1, "Closes this dialog." },
{ 126, 2, "Program name and version information." },
{ 126, 3, "Program name and version information." },
{ 126, 4, "Program name and version information." },
{ 126, 5, "Program name and version information." },
{ 126, 6, "Program name and version information." },
{ 126, 7, "Program name and version information." },
{ 126, 8, "Program name and version information." },
{ 126, 9, "Program name and version information." },

/* ===== 127: PARALLEL OPTIONS WINDOW (parOp.c) -- settings for running
 * a calculation on a remote/parallel computing resource. ===== */
{ 127, 1, "The network address of the remote\ncomputer or cluster to run the\ncalculation on." },
{ 127, 2, "The login/username used to connect to\nthe remote computer or cluster." },
{ 127, 3, "Label for the remote address field." },
{ 127, 4, "Label for the login field." },
{ 127, 5, "A remote-connection option (e.g.\nconnection protocol or authentication\nmethod)." },
{ 127, 6, "A remote-connection option (e.g.\nconnection protocol or authentication\nmethod)." },
{ 127, 7, "A remote-connection option (e.g.\nconnection protocol or authentication\nmethod)." },
{ 127, 8, "A remote-connection option (e.g.\nconnection protocol or authentication\nmethod)." },
{ 127, 9, "The remote file path where the\ncalculation's working files are stored." },
{ 127, 10, "The remote file path where the\ncalculation's working files are stored." },

/* ===== 128: MOVIE OPTIONS -- movie generation settings (Project
 * Options' own copy of MOVIE WINDOW/209's concept). ===== */
{ 128, 1, "Label for the frames-per-event field." },
{ 128, 2, "How many movie frames are generated for\neach event's transition -- more frames\ngive smoother but\nlarger/slower-to-render animations." },
{ 128, 3, "Label for the replay speed field." },
{ 128, 4, "Adjusts the movie's replay speed, from\nslow to fast." },
{ 128, 5, "Label for the fast end of the replay\nspeed slider." },
{ 128, 6, "Label for the slow end of the replay\nspeed slider." },
{ 128, 7, "The movie's replay speed." },
{ 128, 8, "Applies the current movie settings and\ncloses this dialog." },
{ 128, 9, "Discards any changes and closes this\ndialog." },
{ 128, 10, "Opens help for movie options (not\navailable in this build)." },
{ 128, 11, "Groups the movie generation controls." },
{ 128, 12, "Chooses what the movie shows as it plays\nthrough the history (e.g. the block\nmodel, a section, or geophysics)." },
{ 128, 13, "Label for the movie content type choice." },

/* ===== 171: TIME WINDOW (timewin.c, "Event Timing") -- how long a
 * deformation event takes, for movies/animation timing. ===== */
{ 171, 1, "Groups the event duration control." },
{ 171, 2, "Label for the event duration field." },
{ 171, 3, "The relative duration of this event,\nused to pace movies/animations that play\nthrough the history over time." },
{ 171, 4, "The relative duration of this event,\nused to pace movies/animations that play\nthrough the history over time." },

/* ===== 173: LICENCE WINDOW (licwin.c) -- software license
 * registration. ===== */
{ 173, 1, "A warning message about the current\nlicense status (e.g. expiring or\ninvalid)." },
{ 173, 2, "Instructions for entering or requesting\na license." },
{ 173, 3, "Label for the company name field." },
{ 173, 4, "Label for the license code field." },
{ 173, 5, "Groups the license registration fields." },
{ 173, 6, "The company name this copy of the\nsoftware is licensed to." },
{ 173, 7, "The license code/key for this copy of\nthe software." },
{ 173, 8, "Applies the entered license code." },
{ 173, 9, "Closes this dialog without changing the\nlicense." },
{ 173, 10, "Opens help for licensing (not available\nin this build)." },
{ 173, 11, "Opens a form to request a new license." },
{ 173, 12, "Imports a license code from a file\ninstead of typing it in." },

/* ===== 175: CONTACT WINDOW (infowin.c, "Contact Information") -- a
 * multi-step contact/registration details form used when requesting a
 * software license (see LICENCE WINDOW/173's "Request" button). ===== */
{ 175, 1, "Moves to the next step of the contact\ninformation form." },
{ 175, 2, "Label for a contact information field on\nthis step of the form." },
{ 175, 3, "Label for a contact information field on\nthis step of the form." },
{ 175, 4, "Label for a contact information field on\nthis step of the form." },
{ 175, 5, "Label for a contact information field on\nthis step of the form." },
{ 175, 6, "Label for a contact information field on\nthis step of the form." },
{ 175, 7, "Label for a contact information field on\nthis step of the form." },
{ 175, 8, "Label for a contact information field on\nthis step of the form." },
{ 175, 9, "Label for a contact information field on\nthis step of the form." },
{ 175, 10, "Label for a contact information field on\nthis step of the form." },
{ 175, 11, "Moves to the next step of the contact\ninformation form." },
{ 175, 12, "Moves back to the previous step of the\ncontact information form." },
{ 175, 13, "Submits the completed contact\ninformation form." },
{ 175, 14, "Moves back to the previous step of the\ncontact information form." },
{ 175, 15, "Moves to a specific step of the contact\ninformation form." },
{ 175, 16, "Moves to a specific step of the contact\ninformation form." },
{ 175, 17, "Label for a contact information field on\nthis step of the form." },
{ 175, 18, "Label for a contact information field on\nthis step of the form." },
{ 175, 19, "Submits the completed contact\ninformation form." },
{ 175, 20, "Label for a contact information field on\nthis step of the form." },

/* ===== 178: WINPOS WINDOW / 202: WINPOSITION WINDOW -- saved window
 * layout presets (position and size of the app's various sub-windows). ===== */
{ 178, 1, "Groups the saved window layout list." },
{ 178, 2, "The list of the app's windows -- select\none to view or edit its saved position\nand size." },
{ 178, 3, "The selected window's saved X position." },
{ 178, 4, "The selected window's saved Y position." },
{ 178, 5, "The selected window's saved width." },
{ 178, 6, "The selected window's saved height." },
{ 178, 7, "The selected window's saved X position." },
{ 178, 8, "The selected window's saved Y position." },
{ 178, 9, "The selected window's saved width." },
{ 178, 10, "The selected window's saved height." },
{ 178, 11, "Label for the saved X position field." },
{ 178, 12, "Label for the saved Y position field." },
{ 178, 13, "Label for the saved width field." },
{ 178, 14, "Label for the saved height field." },
{ 178, 15, "Groups the selected window's saved\nposition and size fields." },
{ 178, 16, "Applies the current window layout\nsettings and closes this dialog." },
{ 178, 17, "Discards any changes and closes this\ndialog." },
{ 178, 18, "Opens help for window position options\n(not available in this build)." },

{ 202, 1, "Groups the saved window layout list." },
{ 202, 2, "The list of the app's windows -- select\none to view or edit its saved position\nand size." },
{ 202, 3, "The selected window's saved X position." },
{ 202, 4, "The selected window's saved Y position." },
{ 202, 5, "The selected window's saved width." },
{ 202, 6, "The selected window's saved height." },
{ 202, 7, "The selected window's saved X position." },
{ 202, 8, "The selected window's saved Y position." },
{ 202, 9, "The selected window's saved width." },
{ 202, 10, "The selected window's saved height." },
{ 202, 11, "Label for the saved X position field." },
{ 202, 12, "Label for the saved Y position field." },
{ 202, 13, "Label for the saved width field." },
{ 202, 14, "Label for the saved height field." },
{ 202, 15, "Groups the selected window's saved\nposition and size fields." },

/* ===== 183: OPTIONS WINDOW -- a general options container with a
 * category list and live preview, hosting several of the settings
 * dialogs above as selectable pages. ===== */
{ 183, 1, "Shows a live preview reflecting the\ncurrently selected option category's\nsettings." },
{ 183, 2, "The list of option categories available\nto configure." },
{ 183, 3, "Groups the option category list." },
{ 183, 4, "Applies the current settings and closes\nthis dialog." },
{ 183, 5, "Discards any changes and closes this\ndialog." },
{ 183, 6, "Opens help for these options (not\navailable in this build)." },

/* ===== 201: PRINT SCALE WINDOW -- scale factor for printed output
 * (Project Options' own copy of this concept). ===== */
{ 201, 1, "Label for the print scale field." },
{ 201, 2, "Label for the print scale field." },
{ 201, 3, "The scale factor applied when printing\ndiagrams and images." },
{ 201, 4, "Groups the print scale control." },

/* ===== Completing partial-coverage dialogs above: controls past the
 * point earlier registry excerpts were truncated at. ===== */
{ 117, 21, "Opens the block view display options\n(faces shown, lighting, etc.)." },

{ 121, 21, "Groups the geophysics image scale and\ndisplay-options buttons." },
{ 121, 22, "Label for the image scale field." },
{ 121, 23, "Label for the image scale field." },
{ 121, 24, "The scale factor applied when displaying\ncalculated geophysics images." },
{ 121, 25, "Opens the movie generation and replay\noptions." },
{ 121, 26, "Opens the gravity image's display\noptions (colour scale, contours,\nclipping)." },
{ 121, 27, "Opens the background colour/appearance\nsettings for 3D and diagram views." },
{ 121, 28, "Opens the magnetic image's display\noptions (colour scale, contours,\nclipping)." },
{ 121, 29, "Groups the false origin\n(easting/northing) controls." },
{ 121, 30, "False easting: an offset added to the\nmodel's X coordinate when reporting or\nexporting real-world (e.g. UTM)\ncoordinates." },
{ 121, 31, "False northing: an offset added to the\nmodel's Y coordinate when reporting or\nexporting real-world (e.g. UTM)\ncoordinates." },
{ 121, 32, "Label for the false easting field." },
{ 121, 33, "Label for the false northing field." },

{ 123, 21, "Chooses the calculation method: Spatial\ncomputes the response directly in real\nspace; Spectral uses an FFT-based\n(frequency-domain) method, which is\noften faster for large models." },
{ 123, 22, "When checked, the model is internally\npadded to a power-of-two size for a\nfaster FFT-based (spectral) calculation." },
{ 123, 23, "The survey altitude, in metres above\nground -- only used for an Airborne\nsurvey." },
{ 123, 24, "When checked, a constant padding size is\nused instead of automatically rounding\nup to the next efficient FFT size." },
{ 123, 25, "An additional geophysics calculation\noption." },
{ 123, 26, "An additional geophysics calculation\noption." },
{ 123, 27, "The distance beyond the model's edge, in\nreal units, that the calculation still\nconsiders real geology before switching\nto background/padding properties." },
{ 123, 28, "Groups the regional field parameters." },
{ 123, 29, "Groups the property-deformation controls\n(whether events reorient\nremanence/anisotropy)." },
{ 123, 30, "When checked, the individual X/Y/Z (or\nvertical/horizontal) magnetic field\ncomponents are calculated in addition to\nthe total field." },
{ 123, 31, "Opens the block view display options\n(faces shown, lighting, etc.)." },
{ 123, 32, "Unit label for the calculation range\nfield." },
{ 123, 33, "Groups the spectral (FFT-based)\ncalculation settings." },
{ 123, 34, "The padding style used to extend the\nmodel beyond its real extent for the\ncalculation -- e.g. a constant\nbackground value or a tapered/reflected\nedge." },
{ 123, 35, "Label for the padding parameter field." },
{ 123, 36, "A parameter controlling the padding\nstyle selected above (e.g. taper width)." },
{ 123, 37, "Label for the padding type choice." },
{ 123, 38, "The calculation range expressed in whole\nblocks/cubes instead of real units." },

{ 133, 19, "Point spacing along the Y-direction\ngeneration lines." },
{ 133, 20, "Point spacing along the Z-direction\ngeneration lines." },
{ 133, 21, "An additional line-generation option\n(e.g. whether lines run along X or Y)." },
{ 133, 22, "Line length: how far each generated\nsample line extends." },
{ 133, 23, "Label for the line length field." },
{ 133, 24, "Line length: how far each generated\nsample line extends." },
{ 133, 25, "Line spacing: the distance between\nadjacent generated sample lines." },
{ 133, 26, "Label for the line spacing field." },
{ 133, 27, "Line spacing: the distance between\nadjacent generated sample lines." },
{ 133, 28, "Line height: the vertical (or offset)\nspacing used when generating sample\nlines." },
{ 133, 29, "Label for the line height field." },
{ 133, 30, "Line height: the vertical (or offset)\nspacing used when generating sample\nlines." },
{ 133, 31, "An additional generation option (e.g.\nwhether to generate along lines instead\nof a single grid)." },
{ 133, 32, "An additional generation option." },
{ 133, 33, "An additional generation option." },
{ 133, 34, "An additional generation option." },

{ 166, 21, "The Z (depth) coordinate of the trace's\nlower reference point." },
{ 166, 22, "The Y coordinate of the trace's lower\nreference point." },
{ 166, 23, "The X coordinate of the trace's lower\nreference point." },
{ 166, 24, "The Y coordinate of the trace's lower\nreference point." },
{ 166, 25, "The Z (depth) coordinate of the trace's\nlower reference point." },
{ 166, 26, "Label for the trace's upper reference\npoint." },
{ 166, 27, "Label for the trace's lower reference\npoint." },

{ 167, 20, "The X coordinate of the trace's lower\nreference point." },
{ 167, 21, "The Y coordinate of the trace's lower\nreference point." },
{ 167, 22, "The Z (depth) coordinate of the trace's\nlower reference point." },
{ 167, 23, "Label for the trace's upper reference\npoint." },
{ 167, 24, "Label for the trace's lower reference\npoint." },

{ 175, 21, "Label for a contact information field on\nthis step of the form." },
{ 175, 22, "Label for a contact information field on\nthis step of the form." },
{ 175, 23, "Label for a contact information field on\nthis step of the form." },
{ 175, 24, "Label for a contact information field on\nthis step of the form." },
{ 175, 25, "Label for a contact information field on\nthis step of the form." },
{ 175, 26, "Moves to a specific step of the contact\ninformation form." },
{ 175, 27, "Label for a contact information field on\nthis step of the form." },
{ 175, 28, "Label for a contact information field on\nthis step of the form." },

{ 176, 21, "The view volume's origin position along\nthe Y (North-South) axis." },
{ 176, 22, "Applies the current view volume settings\nand closes this dialog." },
{ 176, 23, "Discards any changes and closes this\ndialog." },
{ 176, 24, "Opens help for view settings (not\navailable in this build)." },
{ 176, 25, "Saves the current view volume settings\nas a new named preset." },
{ 176, 26, "Deletes the selected saved view volume\npreset." },
{ 176, 27, "Groups the saved view volume preset\nlist." },
{ 176, 28, "The list of saved view volume presets --\nselect one to restore its dimensions and\norigin." },
{ 176, 29, "The list of saved view volume presets --\nselect one to restore its dimensions and\norigin." },
{ 176, 30, "Groups the block size controls for this\nview." },
{ 176, 31, "The block size (voxel edge length) used\nfor the geology model within this view\nvolume." },
{ 176, 32, "The block size (voxel edge length) used\nfor the geophysics calculation grid\nwithin this view volume." },
{ 176, 33, "Label for the geology block size field." },
{ 176, 34, "Label for the geophysics block size\nfield." },
{ 176, 35, "Unit label for the block size fields." },
{ 176, 36, "Unit label for the block size fields." },
{ 176, 37, "Opens additional 3D display options for\nthis view." },

{ 188, 19, "Regional magnetic field total intensity,\nin nanotesla, at the reference position." },

{ 210, 19, "The borehole collar's position along the\nY (North-South) axis." },

{ 217, 5, "Selects the last option in the help\nviewer choice list." },

};
extern const int g_tooltipsCount = (int)(sizeof(g_tooltips) / sizeof(g_tooltips[0]));

/* ============================================================================
 * Per-icon tooltips (user-requested), keyed by the *_ICON resource id
 * (builder.h) rather than a dialog/ctlId pair -- these icons are drawn
 * directly onto a window's backing pixmap (not as separate Qt widgets)
 * by builder.c's updateFloatingMenu() (the toolbar's 14 event-type
 * icons, hit-tested/created via menuEventHandler's E_MOUSE_DOWN) and
 * eventlib.c's updateEventOptions() (an event dialog's Form/Position/
 * Orientation/Scale tab icons down the right side, hit-tested by
 * eventOptionsMouseUp()) -- both call the same xvt_dwin_draw_icon(),
 * which is what actually records the hit-test region these are looked
 * up against (see xvt_compat.cpp's XvtWindow::iconTipRegions/event()).
 * ============================================================================
 */
struct IconTooltipEntry { int iconId; const char *text; };

extern const IconTooltipEntry g_iconTooltips[] = {
    /* Toolbar: the 14 event-type icons (builder.c's menuItems[]/tools[]) --
     * click one to set that as the tool used when a new event icon is
     * dropped into the History, or double-click to add it immediately. */
    { 10000, "Pointer tool: click an existing\nevent's icon in the History to\nselect and edit it, instead of\ncreating a new one." },
    { 10001, "Stratigraphy: an ordered sequence\nof flat-lying rock layers, forming\nthe base of every model." },
    { 10002, "Fold: bends the layers below it\ninto a wave-like fold, with a\nchosen wavelength, amplitude and\norientation." },
    { 10003, "Fault: displaces rock on either\nside of a surface by a chosen\nslip, dip and dip direction." },
    { 10004, "Unconformity: an erosion surface,\nwith a new sequence of layers\ndeposited on top of it." },
    { 10005, "Shear Zone: spreads fault-like\ndisplacement across a finite width\ninstead of a single surface." },
    { 10006, "Dyke: a sheet-like intrusion of\nrock that cuts across the existing\nlayers." },
    { 10007, "Plug: a roughly cylindrical or\nconical intrusion of rock, such as\na volcanic plug." },
    { 10008, "Strain: applies a uniform 3x3\ndeformation (stretch, shear,\nrotation) to everything below it." },
    { 10009, "Tilt: rotates everything below it\nby a chosen angle around a chosen\naxis." },
    { 10010, "Foliation: records a planar fabric\norientation (dip and dip\ndirection) without deforming\nanything." },
    { 10011, "Lineation: records a linear fabric\norientation (plunge and plunge\ndirection) without deforming\nanything." },
    { 10012, "Stop: marks the end of the\nhistory -- any events after this\npoint are ignored by calculations." },
    { 10013, "Import: brings in a block of\npre-computed geology from another\nfile." },
    { 10014, "Generic: a placeholder event type\nwith freely labelled parameters,\nfor cases the other event types\ndon't cover." },

    /* Event dialog: the Form/Position/Orientation/Scale tab icons down
     * the right side of an event's options window (eventlib.c's tabs[]). */
    { 10200, "Form: this event's shape/profile\nparameters (e.g. wavelength,\nalteration, geometry type)." },
    { 10201, "Position: this event's X, Y, Z\nlocation in the model." },
    { 10202, "Orientation: this event's dip,\ndip direction and/or pitch." },
    { 10203, "Scale: this event's size\nparameters (e.g. wavelength,\namplitude, slip, radius)." },
    { 10204, "Time: this event's relative\ntiming/duration, used to pace\nmovies/animations." },
    { 10205, "Form (Surface): this event's\nshape expressed as a 3D grid\ninstead of a simple profile." },
    { 10206, "Scale (Ellipsoid): the 3D\nellipsoidal extent of this event's\nzone of influence." },

    /* Drawn directly on Section/Map/Block diagrams (nodwork2.c) to mark
     * an event's location -- not part of the toolbar or tab strip, but
     * drawn via the same xvt_dwin_draw_icon() primitive. */
    { 10100, "Marks where this event is located\non the diagram." },
};
extern const int g_iconTooltipsCount = (int)(sizeof(g_iconTooltips) / sizeof(g_iconTooltips[0]));
