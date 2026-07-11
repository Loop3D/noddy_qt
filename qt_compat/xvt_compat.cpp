/* ============================================================================
 * XVT -> Qt COMPATIBILITY LAYER  (flagged file: see qt_compat/README.md)
 *
 * Qt5-backed implementation of the xvt_xxx() functions declared in xvt.h.
 * This is the ONLY place in the whole port where "XVT calls" actually turn
 * into Qt calls -- every one of the ~200 original application .c files is
 * untouched and still just calls xvt_win_get_ctl(), xvt_dm_post_error(),
 * etc. exactly as it always did.
 *
 * Central design:
 *  - Real XVT's WINDOW is `long` (an opaque handle, not a pointer) -- kept
 *    that way here too. `g_objs` maps each handle to an XvtObj that owns
 *    either a live QWidget (on-screen windows/controls) or nothing (pure
 *    off-screen pixmaps), plus a retained QImage "backing store".
 *  - All xvt_dwin_draw_* calls paint into that backing store with a
 *    QPainter, matching XVT's "draw anytime, not just in paintEvent"
 *    semantics; on-screen windows just blit their backing store in
 *    paintEvent. This is the same mechanism real XVT effectively gives you.
 *  - XvtWindow (a plain QWidget subclass, deliberately NOT a QObject with
 *    Q_OBJECT/signals -- see below) turns native Qt events into synthesized
 *    EVENT structs and calls the application's WIN_EVENT_HANDLER, so every
 *    existing `..._eh(WINDOW, EVENT*)` function in the app keeps working
 *    unchanged. Button/control callbacks are wired with lambda-based
 *    QObject::connect (legal without Q_OBJECT/moc on the *receiver* side),
 *    which is why this whole compat layer builds with plain g++ and no moc
 *    step in the Makefile.
 *
 * Functions tagged [STUB] in xvt.h are implemented at the bottom of each
 * section: they log via qWarning() once and return a harmless default, so
 * the full ~200-file application links today; each is a marked TODO for a
 * later porting pass.
 * ============================================================================
 */
#include "xvt.h"
#include "statbar.h"

#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QScreen>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QScrollBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QVariant>
#include <QDebug>
#include <QHash>
#include <QVector>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPolygonF>
#include <QStatusBar>
#include <QFrame>
#include <QPrinter>
#include <QPrintDialog>
#include <QPageSetupDialog>

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ==========================================================================
 * Handle table
 * ========================================================================== */
struct XvtObj {
    QWidget *widget = nullptr;   /* null for pure off-screen pixmaps */
    QImage backing;              /* retained drawing surface (all drawable objs) */
    WIN_EVENT_HANDLER handler = nullptr;
    long userData = 0;
    WIN_TYPE type = W_DOC;
    WINDOW parent = NULL_WIN;
    int ctlId = -1;
    COLOR foreColor = COLOR_BLACK;
    COLOR backColor = COLOR_WHITE;    /* current DRAWING BRUSH colour (xvt_dwin_set_back_color/
                                        * set_std_cbrush/set_draw_ctools's tools->brush.color) --
                                        * a graphics-state value that changes constantly mid-draw,
                                        * NOT the window's own background. */
    COLOR winBackColor = COLOR_WHITE; /* the window's ATTR_BACK_COLOR (xvt_vobj_get/set_attr) --
                                        * distinct from backColor above. These two were the same
                                        * field until this split: drawBlockImageColorScale (and
                                        * any other code looping xvt_dwin_set_draw_ctools with a
                                        * per-shape brush colour, e.g. per-layer swatches) left
                                        * backColor holding whatever colour was drawn LAST: the
                                        * window's own E_UPDATE handler then read that same field
                                        * back via ATTR_BACK_COLOR to decide what colour to
                                        * xvt_dwin_clear() with, so every subsequent repaint
                                        * cleared the ENTIRE window to the last-drawn shape's
                                        * colour instead of the intended background (matches a
                                        * user report/screenshot: the Display Type popup's
                                        * background turned solid green, exactly the last-drawn
                                        * layer swatch's colour). */
    int drawMode = 0 /* M_COPY */;  /* see xvt_dwin_set_draw_ctools/set_draw_mode */
    short penPat = PAT_SOLID;    /* see xvt_dwin_set_draw_ctools; PAT_HOLLOW/PAT_NONE means
                                   * "no visible outline" for draw_rect/polygon/oval/roundrect */
    short brushPat = PAT_SOLID;  /* same, for fill -- PAT_HOLLOW/PAT_NONE means no fill */
    PNT drawPos{0, 0};
    QFont font;
    bool createSent = false;
    long resId = 0;              /* dialog_registry.cpp lookup key, for windows created via the _def or _res creation functions */
    QFormLayout *formLayout = nullptr; /* lazily created; holds auto-vivified controls, see xvt_win_get_ctl */
    MENU_ITEM *menuTree = nullptr; /* lazily allocated, see xvt_menu_get_tree/xvt_menu_set_tree */
    DATA_PTR palette = nullptr;    /* see xvt_vobj_get_palet/xvt_vobj_set_palet */
    QRect clipRect;                 /* see xvt_dwin_set_clip; invalid/null = no clip */
    QRegion updateRegion;            /* see xvt_dwin_is_update_needed; the region passed by
                                       * the current in-progress paintEvent's E_UPDATE dispatch,
                                       * empty/null when not currently handling E_UPDATE */
    QPainter *printPainter = nullptr; /* see xvt_print_open_page -- set only for the WINDOW
                                        * handle returned by xvt_print_create_win, which has no
                                        * widget/backing of its own; drawing calls made against
                                        * it (nodLib1.c's printPicture) go straight to this
                                        * painter, targeting a real QPrinter device. */
    bool isModalDialog = false;   /* see pushModalDialog/popModalDialog -- true for windows
                                    * created via the xvt_win_create_def/xvt_dlg_create_def/_res
                                    * "real dialog" paths (as opposed to xvt_win_create's
                                    * "document window" path used for History/Toolbar/diagrams),
                                    * so xvt_vobj_destroy knows to pop it off the modal stack. */
    bool isCustomCtl = false;     /* see xvt_custom_ctl_create -- true for windows backing an
                                    * xvtcm_create() custom control (e.g. the Profile window's
                                    * graph canvas). xvt_win_get_ctl's auto-vivify skips these:
                                    * real callers (e.g. proflib.c's updateProfileLocation) query
                                    * the CANVAS first for a sibling control ctlId, expecting
                                    * NULL_WIN back so they can retry against the canvas's PARENT
                                    * (the actual dialog those controls really belong to) --
                                    * auto-vivify "succeeding" on the canvas itself instead
                                    * planted a stray duplicate placeholder widget directly on
                                    * top of the canvas's own drawing, and the real, correctly-
                                    * positioned control in the parent dialog was never queried
                                    * (matches a user report/screenshot: the Profile window's X/Y
                                    * cursor readout appearing as overlapping boxes on top of the
                                    * curve instead of beside it). */
};

/* XVT_PALETTE is an opaque XVT_PALETTE_REC* handle (xvt.h): modern
 * displays are true-color, so there's no hardware palette to actually
 * install (xvt_vobj_set_palet is correspondingly a no-op as far as
 * rendering goes) -- but the app still expects to build up a real list
 * of colors and read it back (e.g. custom-colour dialogs, and
 * nodGraph.c's setStratPalet round-tripping a window's palette onto
 * another), so give the handle real backing storage rather than
 * stubbing it out to nullptr. The XVT_PALETTE_REC struct itself stays an
 * opaque `{int placeholder;}` in xvt.h (matching real XVT's opacity) --
 * this pointer is never dereferenced as that type, only ever cast back
 * to XvtPaletteImpl*, mirroring the WINDOW-handle pattern used
 * throughout this file. Defined this early (well before its actual
 * xvt_palet_* functions further down) so both xvt_vobj_destroy and
 * xvt_vobj_set_palet can see the complete type. */
struct XvtPaletteImpl { QVector<COLOR> colors; long tolerance = 0; };

/* XVT_IMAGE: an in-memory bitmap (distinct from XVT_PIXMAP, an
 * on-screen-capable drawing surface -- see xvt_pmap_create further down).
 * Defined this early, same reasoning as XvtPaletteImpl above, so
 * xvt_dwin_draw_image (needed earlier in the file, in the drawing
 * primitives section) can see the complete type. */
struct XvtImageImpl { QImage img; };

static QHash<WINDOW, XvtObj *> g_objs;
static WINDOW g_nextHandle = 1;
WINDOW TASK_WIN = NULL_WIN;
WINDOW SCREEN_WIN = NULL_WIN;
/* xvt_config itself is DEFINED by the application (noddy.c has
 * `XVT_CONFIG xvt_config = {...};`), matching real XVT exactly -- only
 * declared `extern` in xvt.h. Do not define it here too (that would be a
 * duplicate-definition link error). */

static void pushModalDialog(WINDOW h);
static void popModalDialog(WINDOW h);

static WINDOW allocHandle(XvtObj *obj)
{
    WINDOW h = g_nextHandle++;
    g_objs.insert(h, obj);
    if (obj->widget) {
        obj->widget->setProperty("xvtHandle", (qlonglong)h);
        /* Qt can delete this widget as a side effect of a PARENT widget's
         * destruction (cascading child delete -- e.g. an event dialog's
         * nested preview sub-window, destroyed automatically when the
         * outer dialog is closed, without xvt_vobj_destroy ever being
         * called for it specifically). Left unguarded, g_objs keeps a
         * dangling XvtObj::widget pointer forever, which later crashes
         * anything that walks g_objs and dereferences ->widget (e.g.
         * xvt_scr_list_wins calling windowTitle() on freed memory).
         * QObject::destroyed fires from every QWidget's base destructor
         * regardless of Q_OBJECT/moc on the derived class, so this catches
         * ALL deletion paths generically, including our own explicit
         * xvt_vobj_destroy()'s deleteLater() -- deliberately does NOT
         * touch/delete the XvtObj itself here, since xvt_vobj_destroy may
         * already have deleted it synchronously before this deferred
         * signal fires; only g_objs.remove() is safe to repeat. */
        QObject::connect(obj->widget, &QObject::destroyed, [h]() { g_objs.remove(h); popModalDialog(h); });
    }
    return h;
}

/* Simulated modality for "real dialog" windows (see makeWindow's
 * isDialogPath handling): a top-level window disables every OTHER
 * top-level window while it's open and restores them when it closes,
 * matching the exact technique real (auto-generated, pre-Qt-port) app
 * code already uses for LAYER_DISPLAY_WINDOW popups (blklayop.c's
 * E_CREATE/E_DESTROY, which walks xvt_scr_list_wins() and toggles
 * xvt_vobj_set_enabled) -- generalized here to event/options dialogs so
 * the user can't interact with History/Toolbar/other diagrams while one
 * is open (matches user request: dialogs should "stay in front and
 * block actions until closed"). A stack (not a single flag) so nested
 * dialogs (one dialog opening another) don't re-enable everything when
 * the INNER one closes while the OUTER one is still up -- only the
 * transition to/from a fully-empty stack actually disables/restores. */
static QList<WINDOW> g_modalDialogStack;
static QList<WINDOW> g_modalDisabledWindows;

static void pushModalDialog(WINDOW h)
{
    if (g_modalDialogStack.isEmpty()) {
        g_modalDisabledWindows.clear();
        for (auto it = g_objs.begin(); it != g_objs.end(); ++it) {
            XvtObj *other = it.value();
            if (!other->widget || other->ctlId != -1 || it.key() == TASK_WIN || it.key() == h)
                continue;
            /* g_objs' ctlId==-1 convention marks EVERY window-level object
             * this port hands out, not just genuinely independent
             * top-level windows -- createPositionedWindow's per-event
             * Form/Scale/Position/Layers sub-panels (STRATIGRAPHY_WINDOW,
             * FFOLD_WINDOW, XYZ_WINDOW, ...) are flat W_NO_BORDER children
             * reparented directly into their owning EVENT_WINDOW's widget
             * (see makeWindow's flatChild path), yet still get their own
             * ctlId==-1 XvtObj. Without this check, opening any event
             * dialog disabled its OWN just-created sub-panels (their
             * E_CREATE -- and thus createEventOptions building them --
             * already ran synchronously inside the show() call above,
             * before this code even runs), leaving every button inside
             * the freshly-opened dialog itself greyed out (matches user
             * report/screenshot: "nearly all event dialog widgets are
             * currently disabled"). A genuine independent window is
             * either truly top-level (no Qt parent at all -- e.g.
             * xvt_dlg_create_def dialogs) or nested as a real QMdiSubWindow
             * (e.g. EVENT_WINDOW, History, Block Diagram); a reparented
             * sub-panel's parent is a plain sibling widget, neither. */
            QWidget *parent = other->widget->parentWidget();
            bool isRealTopLevel = !parent || qobject_cast<QMdiSubWindow *>(parent);
            if (!isRealTopLevel) continue;
            if (other->widget->isEnabled()) {
                other->widget->setEnabled(false);
                g_modalDisabledWindows.append(it.key());
            }
        }
    }
    g_modalDialogStack.append(h);
}

/* Safe to call even if h was never pushed (removeOne no-ops) -- also
 * called unconditionally from allocHandle's QObject::destroyed handler
 * as a safety net in case a modal dialog's widget is ever deleted via
 * cascading parent destruction rather than xvt_vobj_destroy, so the
 * stack can never get stuck non-empty (which would permanently disable
 * the rest of the app). Only ever touches WINDOW handles/g_objs lookups,
 * never the (possibly already-freed) XvtObj that owned h. */
static void popModalDialog(WINDOW h)
{
    if (!g_modalDialogStack.removeOne(h)) return;
    if (g_modalDialogStack.isEmpty()) {
        for (WINDOW w : g_modalDisabledWindows) {
            XvtObj *other = g_objs.value(w, nullptr);
            if (other && other->widget) other->widget->setEnabled(true);
        }
        g_modalDisabledWindows.clear();
    }
}

static XvtObj *objFor(WINDOW win)
{
    return g_objs.value(win, nullptr);
}

static QWidget *widgetFor(WINDOW win)
{
    XvtObj *o = objFor(win);
    return o ? o->widget : nullptr;
}

static QColor toQColor(COLOR c)
{
    return QColor(XVT_GET_RED(c), XVT_GET_GREEN(c), XVT_GET_BLUE(c));
}

static QRect toQRect(const RCT *r)
{
    return QRect(r->left, r->top, r->right - r->left, r->bottom - r->top);
}

static void fromQRect(RCT *r, const QRect &qr)
{
    r->left = (short)qr.left();
    r->top = (short)qr.top();
    r->right = (short)qr.right();
    r->bottom = (short)qr.bottom();
}

/* Every xvt_dwin_* draw primitive below ends by requesting a repaint so
 * draw calls made OUTSIDE of a paintEvent (the normal case -- app code
 * drawing in response to a button click, a timer, etc) actually show up
 * on screen. But when a draw call happens INSIDE this window's own
 * paintEvent -- i.e. from its E_UPDATE handler, which is how almost every
 * window redraws itself, typically starting with xvt_dwin_clear() as the
 * very first call -- requesting ANOTHER repaint is not just redundant,
 * it's actively harmful: Qt's update() schedules a new paint event, which
 * fires again as soon as the event loop is next idle (near-immediately),
 * re-entering the same E_UPDATE handler, which calls xvt_dwin_clear()
 * again, which requests another repaint again -- an infinite, CPU-pegging
 * repaint loop for as long as the window exists. Confirmed via gdb:
 * interrupting the idle app repeatedly caught it mid-paintEvent every
 * time (never blocked waiting for X11 events, as an idle Qt app should
 * be), and matches user reports of a permanently busy/flickering cursor
 * and pegged CPU the whole time any window (not just dialogs) is open.
 * o->updateRegion is already tracked for exactly this "are we currently
 * inside this object's own E_UPDATE dispatch" question (see
 * XvtWindow::paintEvent/xvt_dwin_is_update_needed) -- when it's non-null,
 * skip the repaint request; the backing image change will show up when
 * the CURRENT paintEvent blits it momentarily, no new one needed. */
static void requestRepaint(XvtObj *o)
{
    if (!o || !o->widget) return;
    if (!o->updateRegion.isEmpty()) return;
    o->widget->update();
}

/* Ensure a backing image exists and is at least (w,h) in size, preserving
 * existing content -- mirrors XVT windows retaining their drawn content
 * across resizes without the application having to redraw everything. */
static void ensureBacking(XvtObj *o, int w, int h)
{
    if (o->backing.width() >= w && o->backing.height() >= h)
        return;
    QImage bigger(qMax(w, o->backing.width()), qMax(h, o->backing.height()), QImage::Format_RGB32);
    bigger.fill(toQColor(o->winBackColor));
    if (!o->backing.isNull()) {
        QPainter p(&bigger);
        p.drawImage(0, 0, o->backing);
    }
    o->backing = bigger;
}

/* Real XVT does NOT implicitly capture the mouse to whichever window a
 * button-press started on -- E_MOUSE_MOVE/E_MOUSE_UP normally go to
 * whatever window is actually under the cursor, and an app must call
 * xvt_win_trap_pointer() explicitly to get exclusive delivery (e.g.
 * builder.c's drag-to-create-object logic). Qt's QWidget, by contrast,
 * implicitly grabs the mouse to the pressed widget for the whole
 * press-drag-release gesture, even across sibling child widgets in the
 * same top-level window. Left alone, that mismatch silently breaks any
 * drag gesture that starts on one window (e.g. the Toolbar) and is meant
 * to be tracked by a different one (e.g. History) -- since
 * menuEventHandler's own E_MOUSE_MOVE/_UP are no-ops, all such drags
 * would vanish with no visible effect. g_trappedWindow explicitly tracks
 * XVT-model capture; when not trapped, sendMouse() below redirects
 * MOVE/UP events to whatever XvtWindow is actually under the cursor
 * instead of the Qt-grabbed origin widget. */
static WINDOW g_trappedWindow = NULL_WIN;

class XvtWindow;
static XvtWindow *enclosingXvtWindow(QWidget *w);

/* ==========================================================================
 * XvtWindow: turns native Qt events into EVENT structs for the app's
 * WIN_EVENT_HANDLER. Deliberately not Q_OBJECT -- no moc required.
 * ========================================================================== */
class XvtWindow : public QWidget {
public:
    WINDOW handle = NULL_WIN;
    QMenuBar *menuBar = nullptr; /* see buildMenuBar(); nullptr if this window has no menu */
    QMdiArea *mdiArea = nullptr; /* see ensureMdiArea(); lazily created the first time a
                                   * child window needs to nest inside this one */
    QStatusBar *statBar = nullptr; /* see statbar_create(); nullptr if this window has no status bar */

    explicit XvtWindow(QWidget *parent) : QWidget(parent) {}

    void dispatch(EVENT &ev)
    {
        XvtObj *o = objFor(handle);
        if (o && o->handler)
            o->handler(handle, &ev);
    }

    /* Real XVT/Windows MDI child windows (Toolbar, History, and per-event
     * dialogs like EVENT_WINDOW) each get their own real title bar and
     * are freely draggable/resizable WITHIN the main frame -- see
     * tectonique.net/noddy/pics/demo2.gif. A plain reparented child
     * QWidget (the original nesting fix) has no window chrome at all, so
     * nested dialogs looked squashed with no visible border. QMdiArea is
     * Qt's real equivalent of this exact behavior. */
    QMdiArea *ensureMdiArea()
    {
        if (!mdiArea) {
            mdiArea = new QMdiArea(this);
            mdiArea->setActivationOrder(QMdiArea::CreationOrder);
            int top = menuBar ? menuBar->sizeHint().height() : 0;
            int bottom = statBar ? statBar->sizeHint().height() : 0;
            mdiArea->setGeometry(0, top, width(), height() - top - bottom);
            mdiArea->show();
            /* MDI children's activation is a QMdiSubWindow-level concept --
             * Qt does not deliver ActivationChange to the plain content
             * widget inside a subwindow when only ITS subwindow (de)
             * activates within the QMdiArea. Real XVT sends E_FOCUS for
             * this (see changeEvent's comment for why it matters -- e.g.
             * Legend window population). Track the previously-active
             * subwindow ourselves so both the newly- and previously-active
             * windows get their active/inactive E_FOCUS. */
            /* Keyed by the owning QMdiArea so multiple independent MDI
             * hosts (if this app ever creates more than one) each track
             * their own previously-active subwindow rather than sharing
             * one global. */
            static QHash<QMdiArea *, QPointer<QMdiSubWindow>> lastActiveByArea;
            QMdiArea *areaPtr = mdiArea;
            QObject::connect(mdiArea, &QMdiArea::subWindowActivated,
                              [areaPtr](QMdiSubWindow *activated) {
                QPointer<QMdiSubWindow> &lastActive = lastActiveByArea[areaPtr];
                if (lastActive && lastActive != activated) {
                    if (auto *xw = dynamic_cast<XvtWindow *>(lastActive->widget())) {
                        EVENT e{}; e.type = E_FOCUS; e.v.active = FALSE;
                        xw->dispatch(e);
                    }
                }
                if (activated) {
                    if (auto *xw = dynamic_cast<XvtWindow *>(activated->widget())) {
                        EVENT e{}; e.type = E_FOCUS; e.v.active = TRUE;
                        xw->dispatch(e);
                    }
                }
                lastActive = activated;
            });
        }
        return mdiArea;
    }

protected:
    void showEvent(QShowEvent *) override
    {
        XvtObj *o = objFor(handle);
        if (o && !o->createSent) {
            o->createSent = true;
            EVENT ev{}; ev.type = E_CREATE;
            dispatch(ev);
        }
    }
    void closeEvent(QCloseEvent *ev) override
    {
        EVENT e{}; e.type = E_CLOSE;
        dispatch(e);
        ev->ignore(); /* app calls xvt_vobj_destroy explicitly, as on real XVT */
    }
    void changeEvent(QEvent *ev) override
    {
        /* E_FOCUS was never dispatched anywhere in this port -- real XVT
         * sends it when a window gains/loses activation. Several call
         * sites depend on it for more than cosmetics: e.g. nodwork2.c's
         * block-diagram E_FOCUS handler is the ONLY place that calls
         * updateLegendData(win, BLOCK_DIAGRAM_LEGENDS), so the Legend
         * window stayed permanently blank without this (matches user
         * report). Covers top-level and flatChild windows; MDI-nested
         * children get the same treatment via QMdiArea::subWindowActivated
         * in ensureMdiArea(), since activation of an MDI child is a
         * QMdiSubWindow-level concept Qt doesn't route through a plain
         * child widget's own ActivationChange. */
        if (ev->type() == QEvent::ActivationChange) {
            EVENT e{}; e.type = E_FOCUS;
            e.v.active = isActiveWindow() ? TRUE : FALSE;
            dispatch(e);
        }
        QWidget::changeEvent(ev);
    }
    /* When nested as a QMdiSubWindow (see makeWindow), Qt delivers a
     * close request (clicking the subwindow's own [X]) to the
     * QMdiSubWindow itself, not to this widget's closeEvent -- installed
     * as an event filter on the subwindow so that still routes through
     * the same E_CLOSE dispatch / "app calls xvt_vobj_destroy explicitly"
     * model as every other window. Overriding eventFilter is a plain
     * virtual from QObject, doesn't need Q_OBJECT/moc. */
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::Close) {
            EVENT e{}; e.type = E_CLOSE;
            dispatch(e);
            event->ignore();
            return true;
        }
        return QWidget::eventFilter(watched, event);
    }
    void resizeEvent(QResizeEvent *) override
    {
        XvtObj *o = objFor(handle);
        if (o) ensureBacking(o, width(), height());
        int top = 0;
        if (menuBar) { menuBar->setGeometry(0, 0, width(), menuBar->sizeHint().height()); top = menuBar->height(); }
        int bottom = 0;
        if (statBar) {
            int h = statBar->sizeHint().height();
            statBar->setGeometry(0, height() - h, width(), h);
            bottom = h;
        }
        if (mdiArea) mdiArea->setGeometry(0, top, width(), height() - top - bottom);
        EVENT ev{}; ev.type = E_SIZE;
        dispatch(ev);
    }
    void paintEvent(QPaintEvent *pe) override
    {
        XvtObj *o = objFor(handle);
        if (o) o->updateRegion = pe->region();
        EVENT ev{}; ev.type = E_UPDATE;
        dispatch(ev);
        if (o) o->updateRegion = QRegion();
        if (o && !o->backing.isNull()) {
            QPainter p(this);
            p.drawImage(0, 0, o->backing);
        }
    }
    void mousePressEvent(QMouseEvent *me) override { sendMouse(me, me->button() == Qt::RightButton ? E_MOUSE_DOWN : E_MOUSE_DOWN); }
    void mouseReleaseEvent(QMouseEvent *me) override { sendMouse(me, E_MOUSE_UP); }
    void mouseMoveEvent(QMouseEvent *me) override { sendMouse(me, E_MOUSE_MOVE); }
    void mouseDoubleClickEvent(QMouseEvent *me) override { sendMouse(me, E_MOUSE_DBL); }
    void keyPressEvent(QKeyEvent *ke) override
    {
        EVENT ev{}; ev.type = E_CHAR;
        QByteArray txt = ke->text().toLatin1();
        ev.v.chr.ch = txt.isEmpty() ? 0 : txt.at(0);
        ev.v.chr.virtual_key = ke->key();
        ev.v.chr.shift = (ke->modifiers() & Qt::ShiftModifier) ? TRUE : FALSE;
        ev.v.chr.control = (ke->modifiers() & Qt::ControlModifier) ? TRUE : FALSE;
        dispatch(ev);
    }

private:
    void sendMouse(QMouseEvent *me, EVENT_TYPE type)
    {
        /* E_MOUSE_DOWN always goes to whichever window Qt's hit-test
         * actually delivered the press to -- that part matches real XVT.
         * For MOVE/UP, replicate XVT's "no implicit capture" model: if
         * the app explicitly trapped the pointer (xvt_win_trap_pointer),
         * route there; otherwise route to whatever XvtWindow is actually
         * under the cursor right now, NOT wherever Qt's own implicit
         * per-gesture grab happens to still be delivering to. See
         * g_trappedWindow's comment above for why this matters (toolbar
         * icon drag-to-canvas would otherwise silently do nothing). */
        XvtWindow *target = this;
        QPoint localPos = me->pos();
        if (type == E_MOUSE_MOVE || type == E_MOUSE_UP) {
            if (g_trappedWindow != NULL_WIN) {
                if (auto *tw = dynamic_cast<XvtWindow *>(widgetFor(g_trappedWindow))) {
                    target = tw;
                    localPos = tw->mapFromGlobal(me->globalPos());
                }
            } else if (QWidget *under = QApplication::widgetAt(me->globalPos())) {
                if (auto *tw = enclosingXvtWindow(under)) {
                    target = tw;
                    localPos = tw->mapFromGlobal(me->globalPos());
                }
            }
        }
        EVENT ev{}; ev.type = type;
        ev.v.mouse.where.h = (short)localPos.x();
        ev.v.mouse.where.v = (short)localPos.y();
        /* Real XVT's MOUSE_EVENT.button is just "button number" with no
         * documented physical mapping (checked the real SDK header --
         * xvt_type.h s_mouse -- no constants). App code (nodwork2.c) relies
         * on a genuine 3-way distinction for the block diagram (0=primary/
         * rotate-drag, 1=popup options menu, 2=zoom-drag) but XVT's real
         * Windows build's actual physical-button assignment isn't
         * recoverable from source alone. On Windows specifically, middle-
         * click is rare/awkward while right-click is the OS-wide "secondary
         * action" convention users expect a popup from -- mapping physical
         * Right to logical button 1 (not 2) matches that convention and a
         * user report of right-click doing nothing useful in the block
         * diagram (it was landing on button 2 == zoom-drag, not the popup
         * at button 1). Swapped from the previous Right=2/Middle=1. */
        ev.v.mouse.button = (me->button() == Qt::RightButton) ? 1 : (me->button() == Qt::MiddleButton ? 2 : 0);
        ev.v.mouse.shift = (me->modifiers() & Qt::ShiftModifier) ? TRUE : FALSE;
        ev.v.mouse.control = (me->modifiers() & Qt::ControlModifier) ? TRUE : FALSE;
        target->dispatch(ev);
    }
};

static XvtWindow *enclosingXvtWindow(QWidget *w)
{
    while (w) {
        if (auto *xw = dynamic_cast<XvtWindow *>(w)) return xw;
        w = w->parentWidget();
    }
    return nullptr;
}

/* ==========================================================================
 * [STUB] helper -- log once per call site so we can see what's missing
 * without flooding stdout on every event loop tick.
 * ========================================================================== */
#define XVT_STUB() do { \
        static bool warned = false; \
        if (!warned) { qWarning("XVT-Qt: %s not yet ported (stub)", __func__); warned = true; } \
    } while (0)

/* ==========================================================================
 * Application lifecycle
 * ========================================================================== */
static QApplication *g_app = nullptr;

/* Batch/CLI runs (batchNod.c) never call xvt_app_create -- the original
 * XVT toolkit apparently didn't need a running app object for headless
 * window/dialog creation calls made along the calculation path. Qt's
 * QWidget does need a QApplication to exist first, so lazily construct
 * one here if nothing has done so yet. */
static void ensureQApp()
{
    if (g_app) return;
    static int fakeArgc = 1;
    static char fakeProgName[] = "noddy";
    static char *fakeArgv[] = { fakeProgName, nullptr };
    g_app = new QApplication(fakeArgc, fakeArgv);
}

int xvt_app_create(int argc, char **argv, long /*flags*/, WIN_EVENT_HANDLER task_eh, XVT_CONFIG *cfg)
{
    if (cfg) xvt_config = *cfg;
    static int fakeArgc = 1;
    static char fakeProgName[] = "noddy";
    static char *fakeArgv[] = { fakeProgName, nullptr };
    if (argc <= 0 || !argv) { argc = fakeArgc; argv = fakeArgv; }
    ensureQApp();

    XvtObj *taskObj = new XvtObj();
    taskObj->type = W_DOC;
    taskObj->handler = task_eh;
    auto *taskWidget = new XvtWindow(nullptr);
    taskObj->widget = taskWidget;
    TASK_WIN = allocHandle(taskObj);
    taskWidget->handle = TASK_WIN;
    taskWidget->setWindowTitle(xvt_config.taskwin_title ? xvt_config.taskwin_title : "Noddy");
    /* Real XVT's task window is the app's main OS frame, normally given a
     * generous default size by the window manager. Qt widgets default to
     * a small size otherwise -- and now that W_DOC windows created with
     * TASK_WIN as parent nest INSIDE it (see makeWindow's nestAsChild),
     * it needs to actually be big enough to contain them. */
    taskWidget->resize(1000, 750);

    XvtObj *screenObj = new XvtObj();
    screenObj->type = W_DOC;
    SCREEN_WIN = allocHandle(screenObj);

    taskWidget->show(); /* triggers E_CREATE via showEvent */
    EVENT sizeEv{}; sizeEv.type = E_SIZE;
    taskObj->handler(TASK_WIN, &sizeEv);

    int rc = g_app->exec();

    EVENT destroyEv{}; destroyEv.type = E_DESTROY;
    if (taskObj->handler) taskObj->handler(TASK_WIN, &destroyEv);
    return rc;
}

void xvt_app_destroy(void)
{
    if (g_app) g_app->quit();
}

BOOLEAN xvt_app_allow_quit(void)
{
    if (g_app) g_app->quit();
    return TRUE;
}

void xvt_app_process_pending_events(void)
{
    if (g_app) g_app->processEvents();
}

void xvt_app_get_default_ctools(DRAW_CTOOLS *tools)
{
    if (!tools) return;
    tools->pen.color = COLOR_BLACK;
    tools->pen.width = 1;
    tools->pen.pat = PAT_SOLID;
    tools->brush.color = COLOR_WHITE;
    tools->brush.pat = PAT_SOLID;
    tools->mode = M_COPY;
}

/* ==========================================================================
 * vobj: generic object attributes / data
 * ========================================================================== */
long xvt_vobj_get_attr(WINDOW win, XVT_ATTR attr)
{
    switch (attr) {
    case ATTR_SCREEN_WIDTH:  return QGuiApplication::primaryScreen()->geometry().width();
    case ATTR_SCREEN_HEIGHT: return QGuiApplication::primaryScreen()->geometry().height();
    case ATTR_TITLE_HEIGHT:  return 24;
    case ATTR_DOCFRAME_WIDTH:
    case ATTR_FRAME_WIDTH:   return 4;
    case ATTR_DOCFRAME_HEIGHT:
    case ATTR_FRAME_HEIGHT:  return 4;
    case ATTR_BACK_COLOR: {
        XvtObj *o = objFor(win);
        return o ? o->winBackColor : COLOR_WHITE;
    }
    case ATTR_NATIVE_WINDOW: {
        QWidget *w = widgetFor(win);
        return w ? (long)w->winId() : 0;
    }
    /* nodLib1.c's cascadeWindows() offsets each successive window by
     * exactly these two attrs (corner.h = startX + winNum*cascadeWidth,
     * same for v) -- falling through to the generic 0 default meant every
     * window landed at the identical position, stacked exactly on top of
     * each other with no visible fan-out at all -- user-reported as
     * "Cascade does nothing". Typical desktop cascade offset values. */
    case ATTR_DOC_STAGGER_HORZ: return 24;
    case ATTR_DOC_STAGGER_VERT: return 24;
    default:
        return 0;
    }
}

void xvt_vobj_set_attr(WINDOW win, XVT_ATTR attr, long value)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    switch (attr) {
    case ATTR_BACK_COLOR:
        o->winBackColor = (COLOR)value;
        break;
    default:
        break; /* platform-specific attrs (ATTR_MAC_*, ATTR_WIN_PM_*, ATTR_X_*, ...): no-op under Qt */
    }
}

long xvt_vobj_get_data(WINDOW win)
{
    XvtObj *o = objFor(win);
    return o ? o->userData : 0;
}

void xvt_vobj_set_data(WINDOW win, long data)
{
    XvtObj *o = objFor(win);
    if (o) o->userData = data;
}

void xvt_vobj_destroy(WINDOW win)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    if (o->handler) {
        EVENT ev{}; ev.type = E_DESTROY;
        o->handler(win, &ev);
    }
    if (o->isModalDialog) popModalDialog(win);
    g_objs.remove(win);
    if (o->widget) {
        /* If nested as a real QMdiSubWindow (see makeWindow), deleting
         * just the content widget would leave an empty subwindow frame
         * behind -- delete the wrapping subwindow instead so the whole
         * thing disappears. */
        if (auto *sub = qobject_cast<QMdiSubWindow *>(o->widget->parentWidget()))
            sub->deleteLater();
        else
            o->widget->deleteLater();
    }
    delete reinterpret_cast<XvtPaletteImpl *>(o->palette);
    delete o;
}

void xvt_vobj_set_enabled(WINDOW win, BOOLEAN enabled)
{
    QWidget *w = widgetFor(win);
    if (w) w->setEnabled(enabled != FALSE);
}

BOOLEAN xvt_vobj_get_enabled(WINDOW win)
{
    QWidget *w = widgetFor(win);
    return (w && w->isEnabled()) ? TRUE : FALSE;
}

void xvt_vobj_set_visible(WINDOW win, BOOLEAN visible)
{
    QWidget *w = widgetFor(win);
    if (!w) return;
    /* If nested as a real QMdiSubWindow (see makeWindow), hiding just the
     * content widget leaves the wrapping subwindow's own frame/title bar
     * visible but blank (matches a user report: a "Calculation Status"
     * progress dialog that goes blank but never disappears -- finishLongJob
     * (nodLib2.c) hides it via exactly this call on job completion, same
     * pattern already applied to xvt_vobj_destroy/set_title for the same
     * class of bug). Hide the subwindow instead when present. */
    if (auto *sub = qobject_cast<QMdiSubWindow *>(w->parentWidget())) {
        sub->setVisible(visible != FALSE);
        if (visible) {
            /* Section/Map (and other WSF_INVISIBLE-created-then-shown-
             * later) windows get their geometry set via makeWindow's
             * atomic setGeometry() call while still hidden -- some Qt
             * styles skip laying out a QMdiSubWindow's title-bar
             * decoration for a geometry change that happened before the
             * subwindow was ever visible, and don't redo it on the
             * subsequent show(), only on a genuine post-show geometry
             * change (matches user report: "AGAIN" -- no header bar until
             * manually enlarged). Resize-and-restore to the same size
             * forces Qt to treat this as a real geometry change now that
             * the subwindow is actually shown, without altering the final
             * size. */
            QSize sz = sub->size();
            sub->resize(sz.width() + 1, sz.height());
            sub->resize(sz);
        }
    } else {
        w->setVisible(visible != FALSE);
    }
}

BOOLEAN xvt_vobj_get_visible(WINDOW win)
{
    QWidget *w = widgetFor(win);
    if (!w) return FALSE;
    if (auto *sub = qobject_cast<QMdiSubWindow *>(w->parentWidget()))
        return sub->isVisible() ? TRUE : FALSE;
    return w->isVisible() ? TRUE : FALSE;
}

void xvt_vobj_set_title(WINDOW win, const char *title)
{
    QWidget *w = widgetFor(win);
    if (!w || !title) return;
    if (auto *btn = qobject_cast<QAbstractButton *>(w)) btn->setText(QString::fromLocal8Bit(title));
    else if (auto *lbl = qobject_cast<QLabel *>(w)) lbl->setText(QString::fromLocal8Bit(title));
    else if (auto *edit = qobject_cast<QLineEdit *>(w)) edit->setText(QString::fromLocal8Bit(title));
    else if (auto *box = qobject_cast<QGroupBox *>(w)) box->setTitle(QString::fromLocal8Bit(title));
    /* Missing WC_LISTBUTTON (QComboBox) branch meant this fell through to
     * the generic setWindowTitle() below, which has no visible effect on
     * a plain child combo box -- e.g. nodLib1.c's updateFloatTextField
     * writing the Cube Size Geology/Geophysics dropdowns' current value
     * silently did nothing, so they always showed whatever their first
     * item happened to be rather than the real current setting. setEditText
     * updates the visible text directly (works whether or not the combo is
     * editable -- for a non-editable one it's equivalent to what the user
     * would see typed there); setCurrentText would only succeed if the
     * text exactly matches an existing item, which callers writing a
     * freshly-formatted numeric string can't rely on. */
    else if (auto *combo = qobject_cast<QComboBox *>(w)) combo->setEditText(QString::fromLocal8Bit(title));
    else {
        w->setWindowTitle(QString::fromLocal8Bit(title));
        /* If nested as a real QMdiSubWindow (see makeWindow), its own
         * title bar has a separate title string that doesn't auto-track
         * the content widget's -- keep it in sync (e.g. the event dialog
         * frame gets renamed to the object's name, "Strat"/"Fold"/etc,
         * after creation). */
        if (auto *sub = qobject_cast<QMdiSubWindow *>(w->parentWidget()))
            sub->setWindowTitle(QString::fromLocal8Bit(title));
    }
}

BOOLEAN xvt_vobj_get_title(WINDOW win, char *buf, int maxlen)
{
    if (!buf || maxlen <= 0) return FALSE;
    QWidget *w = widgetFor(win);
    QString s;
    if (auto *btn = qobject_cast<QAbstractButton *>(w)) s = btn->text();
    else if (auto *lbl = qobject_cast<QLabel *>(w)) s = lbl->text();
    else if (auto *edit = qobject_cast<QLineEdit *>(w)) s = edit->text();
    else if (auto *box = qobject_cast<QGroupBox *>(w)) s = box->title();
    /* Missing WC_LISTBUTTON (QComboBox) branch meant reading a dropdown's
     * current value via xvt_vobj_get_title fell through to windowTitle(),
     * which is always empty for a plain child widget -- e.g. csizewin.c's
     * Cube Size Geology/Geophysics dropdowns are read back with
     * getFloatTextFieldValue() -> xvt_vobj_get_title() -> strtod(), so
     * whatever the user picked was silently discarded in favour of 0.0,
     * matching a user report that the Cube Size Geology setting "does not
     * seem to be affecting actual calculation". */
    else if (auto *combo = qobject_cast<QComboBox *>(w)) s = combo->currentText();
    else if (w) s = w->windowTitle();
    QByteArray b = s.toLocal8Bit();
    int n = qMin(maxlen - 1, b.size());
    memcpy(buf, b.constData(), n);
    buf[n] = '\0';
    return w ? TRUE : FALSE;
}

void xvt_vobj_move(WINDOW win, RCT *rct)
{
    /* No menuBar-aware adjustment here (see git blame/history for why
     * that was tried and reverted -- double-counted the menu-bar height
     * for callers that already build their rect from
     * xvt_vobj_get_client_rect(), which is menu-bar-aware itself).
     *
     * Separately, THIS window's content widget may be embedded in a real
     * QMdiSubWindow (see makeWindow's nestAsChild path) -- `rct` is always
     * expressed in CONTENT terms (matches makeWindow's own creation-time
     * convention, and every real caller: cascadeWindows/tileWindows build
     * it from xvt_vobj_get_client_rect(), restoreWindowSize from a
     * content pixmap's size). Just calling setGeometry on the content
     * widget directly only resizes/repositions IT, leaving the subwindow's
     * own outer frame (title bar + border) wherever it was -- since the
     * content widget's local (0,0) is relative to the SUBWINDOW's client
     * area, not to its outer frame, the effect ranged from no visible
     * change to the content visibly starting above/over the subwindow's
     * own title bar once the two geometries disagreed (matches a user
     * screenshot of Section/Line Map windows drawing over their title
     * bar). Mirror makeWindow's own title-bar/frame-overhead calculation
     * so every move/resize grows the real outer subwindow consistently,
     * the same way creation already does. */
    QWidget *w = widgetFor(win);
    if (!w || !rct) return;
    QRect r = toQRect(rct);
    if (auto *subWin = qobject_cast<QMdiSubWindow *>(w->parentWidget())) {
        int titleBar = subWin->style()->pixelMetric(QStyle::PM_TitleBarHeight, nullptr, subWin);
        int frame = subWin->style()->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth, nullptr, subWin);
        QRect newGeom(r.left(), r.top(), r.width() + 2 * frame, r.height() + titleBar + 2 * frame);
        subWin->setGeometry(newGeom);
        return;
    }
    w->setGeometry(r);
}

void xvt_vobj_get_client_rect(WINDOW win, RCT *rct)
{
    if (!rct) return;
    QWidget *w = widgetFor(win);
    if (w) {
        QRect r = w->rect();
        rct->left = 0; rct->top = 0;
        rct->right = (short)r.width();
        rct->bottom = (short)r.height();
        /* If this window has its own menu bar (xvt_win_create's
         * TASK_MENUBAR path, or xvt_menu_set_tree -- e.g. Section/Map
         * windows via createLineMapMenubar), report the client area as
         * starting BELOW it, same as any real windowing system's
         * client-vs-frame distinction. Without this, app code that sizes
         * a pixmap/diagram to "the whole client rect" (e.g. nodwork1.c's
         * Section/Map drawing) painted right through the menu bar's row --
         * the bar is a real child widget so it still rendered on top, but
         * the diagram's own top slice was pointlessly hidden behind it
         * and its scale/size math didn't match what was actually visible
         * below the bar. */
        if (auto *xw = dynamic_cast<XvtWindow *>(w)) {
            if (xw->menuBar) {
                int barHeight = xw->menuBar->height();
                rct->top = (short)barHeight;
            }
        }
        return;
    }
    /* Pixmaps (xvt_pmap_create) have no QWidget -- only an o->backing
     * QImage -- so this always fell through to a degenerate {0,0,0,0}.
     * Real call sites treat a pixmap's client rect as its actual pixel
     * size (e.g. nodwork2.c's block-diagram E_SIZE handler divides by
     * xvt_rect_get_width() of the OLD pixmap's "client rect" to compute a
     * rescale factor when the window resizes -- dividing by the resulting
     * 0 silently produced +inf, corrupting the 3D camera's scale to
     * infinity on every resize, including the initial one Qt sends right
     * after window creation -- matches a user report of the block diagram
     * never appearing centered/correctly scaled in its window). */
    XvtObj *o = objFor(win);
    if (o && !o->backing.isNull()) {
        rct->left = 0; rct->top = 0;
        rct->right = (short)o->backing.width();
        rct->bottom = (short)o->backing.height();
    } else {
        rct->left = rct->top = rct->right = rct->bottom = 0;
    }
}

void xvt_vobj_get_outer_rect(WINDOW win, RCT *rct)
{
    if (!rct) return;
    QWidget *w = widgetFor(win);
    if (w) fromQRect(rct, w->frameGeometry());
    else { rct->left = rct->top = rct->right = rct->bottom = 0; }
}

WINDOW xvt_vobj_get_parent(WINDOW win)
{
    XvtObj *o = objFor(win);
    return o ? o->parent : NULL_WIN;
}

WIN_TYPE xvt_vobj_get_type(WINDOW win)
{
    XvtObj *o = objFor(win);
    return o ? o->type : W_DOC;
}

void xvt_vobj_translate_points(WINDOW from, WINDOW to, PNT *pts, int n)
{
    QWidget *fw = widgetFor(from);
    QWidget *tw = widgetFor(to);
    if (!fw || !tw || !pts) return;
    for (int i = 0; i < n; i++) {
        QPoint p = fw->mapTo(tw->isAncestorOf(fw) ? tw : tw->window(), QPoint(pts[i].h, pts[i].v));
        pts[i].h = (short)p.x();
        pts[i].v = (short)p.y();
    }
}

void xvt_vobj_raise(WINDOW win)
{
    QWidget *w = widgetFor(win);
    if (!w) return;
    w->raise();
    /* raise() alone only reorders Z-order -- it doesn't make Qt consider
     * this the ACTIVE window, so its widgets kept rendering with Qt's
     * inactive/greyed palette (every style does this for a non-active
     * top-level window) even though nothing was really disabled --
     * matches a user report of an entire popup dialog (Layer Display,
     * Define Colour, ...) looking "not activated". Every popup dialog in
     * this codebase calls bringWindowToFront (-> here) right after
     * xvt_vobj_set_visible, expecting the new dialog to be immediately
     * interactive, same as any real modal/popup on a real OS. */
    if (auto *sub = qobject_cast<QMdiSubWindow *>(w->parentWidget())) {
        /* MDI-nested (e.g. createCenteredWindow's W_DOC windows, DEFINE_
         * COLOUR_WINDOW among them) -- QMdiArea tracks "active subwindow"
         * as a separate concept from raw widget stacking order. */
        if (auto *area = qobject_cast<QMdiArea *>(sub->parentWidget()))
            area->setActiveSubWindow(sub);
    } else {
        w->activateWindow();
    }
}

void xvt_vobj_set_palet(WINDOW win, DATA_PTR palet)
{
    /* Must deep-copy, not just store the pointer: noddy.c's task_eh does
     * `xvt_vobj_set_palet(TASK_WIN, winPalette); xvt_palet_destroy
     * (winPalette);` immediately afterward -- storing the raw pointer
     * would leave TASK_WIN's palette dangling the instant that runs,
     * and setStratPalet (nodGraph.c) later reads TASK_WIN's palette and
     * copies it onto other windows, so a dangling source would propagate
     * a use-after-free. The real XVT SDK must do the same internally for
     * this exact app code pattern to be safe. */
    XvtObj *o = objFor(win);
    if (!o) return;
    delete reinterpret_cast<XvtPaletteImpl *>(o->palette);
    o->palette = nullptr;
    if (palet) {
        auto *src = reinterpret_cast<XvtPaletteImpl *>(palet);
        o->palette = reinterpret_cast<DATA_PTR>(new XvtPaletteImpl(*src));
    }
}
DATA_PTR xvt_vobj_get_palet(WINDOW win)
{
    XvtObj *o = objFor(win);
    return o ? o->palette : nullptr;
}

/* ==========================================================================
 * Windows / dialogs
 *
 * Dialog control reconstruction: this app's ~150 dialogs were originally
 * laid out in a compiled XVT resource file that isn't in this repo (see
 * qt_compat/README.md) -- but the *widget type* of nearly every control in
 * every dialog is recoverable from nodInc.h's auto-generated constant names
 * (e.g. WIN_116_CHECKBOX_68), which qt_compat/dialog_registry.cpp captures
 * as a static table (generated by qt_compat/gen_dialog_registry.py). Real
 * pixel positions/sizes from the original resource file are NOT recovered
 * (that data simply isn't preserved anywhere) -- controls are auto-flowed
 * into a QFormLayout instead, so dialogs are functionally complete (every
 * control is the right widget type and reachable via xvt_win_get_ctl) but
 * not pixel-faithful to the original screenshots.
 * ========================================================================== */
struct DialogCtlEntry { int ctlId; WIN_TYPE type; const char *label; };
struct DialogEntry { long resId; const char *title; const DialogCtlEntry *ctls; int numCtls; };
extern const DialogEntry g_dialogRegistry[];
extern const int g_dialogRegistryCount;

/* ==========================================================================
 * Dialog POSITION overrides -- reconstructed from the original help manual's
 * screenshots (qt_compat/gen_dialog_positions.py reads the manual's .htm/
 * .GIF pages, see scratchpad notes), layered on top of the widget-TYPE
 * registry above. Kept in a separate table (qt_compat/dialog_positions.cpp)
 * so the auto-generated, regeneratable type registry never needs hand
 * edits. A dialog with no override here just keeps the QFormLayout
 * auto-flow fallback -- not every one of the ~150 dialogs has a manual
 * screenshot to reconstruct from.
 * ========================================================================== */
/* typeOverride: W_NONE means "no override, use dialog_registry.cpp's
 * type as-is"; anything else replaces it. Needed for controls the
 * nodInc.h-name-based registry classifier got wrong -- e.g. EVENT_WINDOW
 * (137)'s OK/Cancel/Help/Previous/Next buttons are all named without a
 * recognizable "PUSHBUTTON" substring (just "EVENT_OK" etc.), so the
 * registry defaults them to W_NONE -> WC_EDIT (a text box) instead of a
 * real button. Kept here rather than hand-editing the auto-generated
 * dialog_registry.cpp, matching the existing "layered override" approach
 * used for positions. */
/* labelOverride: nullptr means "use dialog_registry.cpp's auto-generated
 * label as-is" (a spaced-out version of the raw nodInc.h constant name,
 * e.g. "EVENT NAME GB" for EVENT_NAME_GB -- fine for edit/button ctlIds
 * that don't display their registry label as visible text, but wrong for
 * anything that DOES, like a QGroupBox's title). Set to the real text
 * confirmed from an actual screenshot when the raw name would otherwise
 * be user-visible and wrong (e.g. "Event Name", not "EVENT NAME GB"). */
/* initiallyDisabled: control starts disabled (Qt setEnabled(false)) right
 * after creation -- e.g. OPTIONS_WINDOW's Help button, whose real action
 * shells out to a hardcoded Windows browser command this port has no
 * equivalent for yet. */
/* hidden: control is a real, functional XVT object (xvt_win_get_ctl,
 * xvt_list_add, xvt_vobj_set/get_title etc. all still work on it) but is
 * never meant to be drawn -- e.g. DEFINE_COLOUR_WINDOW_LISTEDIT_41, which
 * real XVT uses purely as a backing store (selected-colour-name title,
 * SLIST of predefined/custom names for E_CONTROL lookups) while the
 * visible/clickable "Predefined Colours" swatch grid is separately painted
 * by nodGraph.c's drawDefinedColorGrid straight onto the parent window
 * (see its E_UPDATE handler) at the same on-screen rect. Giving this
 * control a normal opaque QListWidget there just draws a scrollable text
 * list on top of -- and hides -- the real swatch grid underneath (user
 * screenshot: a sliver of the colour grid peeking out above an opaque
 * "Color 1"/"Color 2"/... list box). Hiding it lets the grid paint fully
 * and lets clicks fall through to the window's own E_MOUSE_UP handler
 * (interactivelySelectColor), which is how real XVT's swatch-click
 * selection actually works. */
/* editable: this WC_LISTBUTTON (QComboBox) should accept free-typed text
 * as well as a predefined-item dropdown -- e.g. csizewin.c's Cube Size
 * Geology/Geophysics controls, whose E_CONTROL handler reads
 * xdEvent->v.ctl.ci.v.edit.focus_change/.active (the EDIT sub-event
 * fields, not lbox's), confirming real XVT's control here is an
 * editable combo, not a plain closed-choice dropdown -- user-requested:
 * "should be both drop down menu with predefined values but also allow
 * arbitrary values to be typed in". */
/* multiSelect: this WC_LBOX/WC_LISTEDIT is iterated with xvt_list_is_sel()
 * over every row by the owning app code (not just a single xvt_list_get_
 * sel_index() lookup) -- e.g. nodLib2.c's updateBlockImageOptions()
 * (LAYER_DISPLAY_LIST, "Layers") and optnlib.c's Volume/Surface options
 * (SURFACE_SELECTION_LIST) both loop `xvt_list_is_sel(list, i)` per row to
 * build a per-layer on/off array, i.e. real XVT lets you tick an arbitrary
 * subset of rows. QListWidget defaults to single-selection, which silently
 * capped these to "pick exactly one layer" -- user-reported as "only one
 * strat layer can be selected" / "multiple arbitrary layers cannot be
 * selected from list". */
/* radioGroup: QRadioButton is exclusive-by-parent-widget in Qt when not
 * explicitly grouped -- but every control in a reconstructed dialog is a
 * flat sibling of the same parent (see the `pos` branch below), so a
 * dialog with more than one logically-independent set of radio buttons
 * (e.g. File Format's X/Y/Z "Order" 1st/2nd/3rd rows, 3 independent
 * 3-way choices, plus 3 more independent 2-way "Direction" choices) would
 * otherwise have ALL of them wired into one giant mutually-exclusive
 * group, letting only a single radio button be selected across the whole
 * window. Nonzero values partition radios in the same dialog into
 * separate QButtonGroups; 0 (default) preserves the old implicit
 * same-parent grouping, which is harmless for the common case of a
 * dialog with only one radio group. */
struct DialogCtlPosition { int ctlId; short left, top, right, bottom; WIN_TYPE typeOverride = W_NONE; const char *labelOverride = nullptr; bool initiallyDisabled = false; bool hidden = false; bool multiSelect = false; bool editable = false; int radioGroup = 0; bool skipAutoCreate = false; };
struct DialogPositionEntry {
    long resId;
    short width, height;          /* overall window client size */
    const DialogCtlPosition *ctls;
    int numCtls;
    /* nullptr = use dialog_registry.cpp's auto-generated title as-is (the
     * raw nodInc.h resource name, e.g. "DEFINE COLOUR WINDOW") -- set to
     * the real title confirmed from a screenshot when that raw name would
     * otherwise show in the window's title bar (the app itself never
     * calls xvt_vobj_set_title for most dialogs, so the registry default
     * is genuinely what renders). */
    const char *titleOverride = nullptr;
};
extern const DialogPositionEntry g_dialogPositions[];
extern const int g_dialogPositionsCount;

static const DialogEntry *findDialogEntry(long resId)
{
    if (!resId) return nullptr;
    for (int i = 0; i < g_dialogRegistryCount; i++)
        if (g_dialogRegistry[i].resId == resId) return &g_dialogRegistry[i];
    return nullptr;
}

static const DialogCtlEntry *findCtlEntry(const DialogEntry *dlg, int ctlId)
{
    if (!dlg) return nullptr;
    for (int i = 0; i < dlg->numCtls; i++)
        if (dlg->ctls[i].ctlId == ctlId) return &dlg->ctls[i];
    return nullptr;
}

static const DialogPositionEntry *findPositionEntry(long resId)
{
    if (!resId) return nullptr;
    for (int i = 0; i < g_dialogPositionsCount; i++)
        if (g_dialogPositions[i].resId == resId) return &g_dialogPositions[i];
    return nullptr;
}

static const DialogCtlPosition *findCtlPosition(const DialogPositionEntry *pos, int ctlId)
{
    if (!pos) return nullptr;
    for (int i = 0; i < pos->numCtls; i++)
        if (pos->ctls[i].ctlId == ctlId) return &pos->ctls[i];
    return nullptr;
}

static void sendControlEvent(WINDOW parentWin, int ctlId, WINDOW ctlWin, WIN_TYPE ctlType)
{
    XvtObj *o = objFor(parentWin);
    if (!o || !o->handler) return;
    EVENT ev{};
    ev.type = E_CONTROL;
    ev.v.ctl.id = (short)ctlId;
    ev.v.ctl.ci.type = ctlType;
    ev.v.ctl.ci.win = ctlWin;
    o->handler(parentWin, &ev);
}

/* Creates the Qt widget for a control of the given type, wires its natural
 * Qt signal to synthesize the matching E_CONTROL event back to the owning
 * window's handler (unchanged application code), and registers it in the
 * handle table tagged with ctlId so xvt_win_get_ctl finds it again. */
static WINDOW createControlWidget(WINDOW parentWin, QWidget *parentWidget, QFormLayout *layout,
                                   int ctlId, WIN_TYPE type, const QString &label,
                                   const DialogCtlPosition *pos = nullptr, int fallbackIndex = -1)
{
    QWidget *w = nullptr;
    switch (type) {
    case WC_PUSHBUTTON: w = new QPushButton(label, parentWidget); break;
    case WC_CHECKBOX:   w = new QCheckBox(label, parentWidget); break;
    case WC_RADIOBUTTON:w = new QRadioButton(label, parentWidget); break;
    case WC_GROUPBOX:   w = new QGroupBox(label, parentWidget); break;
    case WC_TEXT:       w = new QLabel(label, parentWidget); break;
    case WC_LBOX: case WC_LISTEDIT:
                        w = new QListWidget(parentWidget); break;
    case WC_LISTBUTTON: {
        /* Real XVT: "button with list" (xvt_type.h) -- a compact
         * click-to-open dropdown, NOT an always-expanded list box. Was
         * previously lumped in with WC_LBOX as a QListWidget, which is
         * fine for a tall always-visible list but unusable at the
         * ~14-23px height these controls are actually given (e.g.
         * EVENT_OPTION_SELECTION, the Form/Scale-style tab switcher at
         * the top of every event dialog, has no room to grow taller --
         * the nested content tab starts right below it at a hardcoded
         * y-offset) -- user-reported as "too small to read the text
         * vertically and cannot be selected in any case". QComboBox is
         * compact when closed and fully usable via its popup regardless
         * of the control's own height. */
        auto *combo = new QComboBox(parentWidget);
        w = combo;
        break;
    }
    case WC_HSCROLL:    w = new QScrollBar(Qt::Horizontal, parentWidget); break;
    case WC_VSCROLL:    w = new QScrollBar(Qt::Vertical, parentWidget); break;
    case WC_TREEVIEW:   w = new QTreeWidget(parentWidget); break;
    case WC_ICON:       w = new QLabel(parentWidget); break;
    case WC_EDIT:
    default:            w = new QLineEdit(parentWidget); break;
    }

    XvtObj *o = new XvtObj();
    o->widget = w;
    o->parent = parentWin;
    o->type = type;
    o->ctlId = ctlId;
    WINDOW h = allocHandle(o);
    w->setProperty("xvtCtlId", ctlId);

    /* Disable every "Help..." button app-wide (per user request: the real
     * XVT displayHelp() shells out to a hardcoded Windows browser path with
     * no Qt-port equivalent, so a live Help button either does nothing or
     * errors -- better to show it disabled than a dead control). Matches
     * on the resolved label text so it covers both dialog_positions.cpp's
     * manually reconstructed "Help..." overrides and any dialog_registry.cpp
     * auto-generated label ending in " HELP" (e.g. "EVENT HELP", "ROCK
     * HELP") that happens to already resolve to a real pushbutton. */
    if (type == WC_PUSHBUTTON && label.contains("help", Qt::CaseInsensitive)) {
        w->setEnabled(false);
    }

    if (type == WC_GROUPBOX || type == WC_TEXT || type == WC_ICON) {
        /* These are purely decorative/container controls in real XVT --
         * a groupbox's rect is meant to visually frame the controls
         * positioned inside it, not literally parent them in Qt's widget
         * tree (this port places every control as a flat sibling via
         * setGeometry, see `pos` branch below). Controls are created in
         * dialog_registry.cpp's ctlId-ascending order, which frequently
         * puts a groupbox/label AFTER the interactive controls it visually
         * contains (e.g. EVENT_WINDOW's ctlId 2 "EVENT NAME" edit field is
         * created before ctlId 3 "EVENT NAME GB" groupbox, whose rect
         * fully contains it) -- Qt stacks later siblings on top by
         * default, so the decorative widget silently ate every click meant
         * for the control underneath (visible, since QGroupBox only paints
         * a border, but unusable -- user-reported as "Event name is not
         * editable" and, for the same reason, "Options selection dropdown
         * readable but cannot be selected"). Making these transparent to
         * mouse input is correct regardless of creation order or which
         * control ends up on top. */
        w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }

    if (pos) {
        /* Reconstructed pixel position from a manual screenshot: place
         * directly, no layout manager involved for this dialog. */
        int h = pos->bottom - pos->top;
        if (type == WC_LISTBUTTON) {
            /* Real XVT's WC_LISTBUTTON is a fixed-height "button with a
             * popup list" (xvt_type.h) -- some call sites give it a much
             * taller rect than that (e.g. builder.c's createPreviewWindow
             * sizes PREVIEW_TYPE's rect 100px tall), which real XVT
             * apparently treats as reserved space for the DROPPED-DOWN
             * popup extent, not the closed control's own height. Taking
             * that literally as a QComboBox's widget height stretched it
             * into an oddly tall, near-invisible-looking control (a
             * combo box isn't designed to render that tall) -- matches a
             * user report of the preview's "Preview Type"/"Preview Type
             * Options" dropdowns and "On" checkbox not appearing. Cap to
             * the combo's natural closed height, anchored at pos->top. */
            int natural = w->sizeHint().height();
            if (h > natural + 4) h = natural;
        }
        w->setGeometry(pos->left, pos->top, pos->right - pos->left, h);
    } else if (!layout) {
        /* This dialog has a position-override table, but this particular
         * control isn't in it (incomplete manual reconstruction) -- stack
         * it at a simple fallback position rather than crash for lack of
         * a layout manager. */
        int idx = fallbackIndex >= 0 ? fallbackIndex : 0;
        w->setGeometry(10, 10 + 24 * idx, 260, 22);
    } else if (type == WC_TEXT || type == WC_GROUPBOX) {
        layout->addRow(w);
    } else {
        layout->addRow(label, w);
    }

    if (auto *btn = qobject_cast<QAbstractButton *>(w)) {
        QObject::connect(btn, &QAbstractButton::clicked, [btn, parentWin, ctlId, h, type](bool nowChecked) {
            /* Real XVT checkboxes/radio buttons do NOT auto-toggle their
             * own checked state on click -- the app's E_CONTROL handler
             * is expected to read the OLD state and explicitly negate +
             * set it itself (e.g. rockwin.c's ROCK_ANISOTROPY case:
             * `state = !xvt_ctl_is_checked(ctl); xvt_ctl_set_checked(ctl,
             * state);`, and xvt_ctl_check_radio_button() for radio
             * groups). Qt's QCheckBox/QRadioButton, unlike real XVT,
             * already auto-toggles natively as part of a plain click,
             * BEFORE this signal fires -- so every such app handler was
             * reading the ALREADY-toggled state, negating it, and setting
             * it right back to what it was before the click, undoing
             * every single click. Matches a user report that checkboxes
             * "check/uncheck but don't stick". Revert Qt's native toggle
             * here (silently, before the app ever sees it) so
             * xvt_ctl_is_checked() reports the pre-click state, matching
             * real XVT's contract -- the app's own handler is responsible
             * for actually committing the new state. */
            if (btn->isCheckable()) {
                btn->blockSignals(true);
                btn->setChecked(!nowChecked);
                btn->blockSignals(false);
            }
            sendControlEvent(parentWin, ctlId, h, type);
        });
    } else if (auto *sb = qobject_cast<QScrollBar *>(w)) {
        QObject::connect(sb, &QScrollBar::valueChanged, [parentWin, ctlId, h, type](int) {
            sendControlEvent(parentWin, ctlId, h, type);
        });
    } else if (auto *lw = qobject_cast<QListWidget *>(w)) {
        QObject::connect(lw, &QListWidget::itemSelectionChanged, [parentWin, ctlId, h, type]() {
            sendControlEvent(parentWin, ctlId, h, type);
        });
    } else if (auto *combo = qobject_cast<QComboBox *>(w)) {
        QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [parentWin, ctlId, h, type](int) {
            sendControlEvent(parentWin, ctlId, h, type);
        });
    } else if (auto *le = qobject_cast<QLineEdit *>(w)) {
        QObject::connect(le, &QLineEdit::editingFinished, [parentWin, ctlId, h, type]() {
            sendControlEvent(parentWin, ctlId, h, type);
        });
    }
    if (pos && pos->initiallyDisabled) w->setEnabled(false);
    if (pos && pos->hidden) w->setVisible(false);
    if (pos && pos->multiSelect) {
        if (auto *lw = qobject_cast<QListWidget *>(w))
            lw->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    if (pos && pos->editable) {
        if (auto *combo = qobject_cast<QComboBox *>(w))
            combo->setEditable(true);
    }
    if (pos && pos->radioGroup != 0) {
        if (auto *rb = qobject_cast<QRadioButton *>(w)) {
            static QHash<quint64, QButtonGroup *> groups;
            quint64 key = (quint64(quint32(parentWin)) << 32) | quint32(pos->radioGroup);
            QButtonGroup *&grp = groups[key];
            if (!grp) grp = new QButtonGroup(parentWidget);
            grp->addButton(rb);
        }
    }
    return h;
}

static WINDOW makeWindow(WIN_TYPE type, RCT *rct, const char *title, WINDOW parent,
                          WIN_EVENT_HANDLER eh, long user_data, long resId = 0,
                          bool isDialogPath = false)
{
    ensureQApp();
    /* Several app-level window-position tables (nodLib3.c's
     * winPositionOptions -- Line Map, Section, Movie, Image, Contour
     * windows among others) use height == -1 (and some, width == -1) as
     * a deliberate "no fixed size, auto-size to content" sentinel.
     * getWindowStartPosition() (nodLib3.c) computes the caller's
     * position.bottom as `y + height`, so a -1 height produces
     * bottom == top-1 -- a degenerate/inverted rect passed straight into
     * xvt_win_create with no fixup in between. Treating that literally
     * (resizing to a negative/zero size) makes the window collapse to
     * nothing, which reads as "this diagram never got its own window" --
     * matches a user report of Map/Section diagrams appearing to draw
     * into the History window instead of their own. Sanitize: a
     * non-positive width or height means "no explicit size", not "zero
     * size" -- keep the given position (left/top) but substitute a
     * reasonable default for the degenerate dimension so the window is
     * still real and visible; the app can still resize it later via its
     * own logic if it wants to auto-fit content. */
    RCT sanitizedRct{};
    if (rct) {
        sanitizedRct = *rct;
        if (sanitizedRct.right <= sanitizedRct.left) sanitizedRct.right = sanitizedRct.left + 500;
        if (sanitizedRct.bottom <= sanitizedRct.top) sanitizedRct.bottom = sanitizedRct.top + 400;
        rct = &sanitizedRct;
    }
    XvtObj *o = new XvtObj();
    o->type = type;
    o->parent = parent;
    o->handler = eh;
    o->userData = user_data;
    o->resId = resId;
    QWidget *parentWidget = widgetFor(parent);
    /* Real XVT on Windows nests windows created with a real parent as MDI
     * children clipped inside that parent's frame (see the reference
     * screenshot at tectonique.net/noddy/pics/demo2.gif -- the "Toolbar"
     * and "History" windows both live inside the outer "Noddy" frame, not
     * as separate top-level OS windows; likewise builder.c's
     * createPreviewWindow creates its W_PLAIN preview canvas parented to
     * the event-options window and expects it embedded there, not
     * floating as its own top-level popup). Without this, every window
     * created with a real parent became an independent top-level QWidget
     * regardless of its requested parent. Not restricted to W_DOC (or
     * even a fixed type list) since createPositionedWindow (builder.c)
     * itself forces W_NO_BORDER for any non-TASK_WIN/SCREEN_WIN parent,
     * so "given a real parent" is already the app's own signal for
     * "nest this," regardless of which WIN_TYPE it happens to be. Dialogs
     * (via xvt_dlg_create_res/_def) are unaffected since those are always
     * created with parent == NULL_WIN, so they still float independently
     * as intended. */
    /* Two different nesting strategies once a real parent is involved:
     * - type == W_NO_BORDER or W_PLAIN: a plain reparented child widget,
     *   no window chrome. W_NO_BORDER is used for content nested WITHIN
     *   another window's own frame (e.g. createPositionedWindow in
     *   builder.c forces this for each event type's Form/Scale/etc
     *   sub-panels nested inside EVENT_WINDOW). W_PLAIN is the real XVT
     *   SDK's "window with plain border" (xvt_type.h) -- a simple bordered
     *   child, NOT a full MDI document window -- used by
     *   createPreviewWindow (builder.c) for the 3D preview canvas embedded
     *   at the EVENT_PREVIEW placeholder's exact rect. Both must stay
     *   seamless, not get their own draggable title bar.
     * - type == W_DOC with a real parent ("nestAsChild"): wrapped in a
     *   real QMdiSubWindow (see ensureMdiArea()) so it gets a proper
     *   title bar, dragging, and resizing within the parent's frame,
     *   matching real XVT/Windows MDI behavior (Toolbar, History, and
     *   per-event dialogs like EVENT_WINDOW all matched this in the
     *   reference screenshot, tectonique.net/noddy/pics/demo2.gif).
     * Dialogs (via xvt_dlg_create_res/_def) are unaffected since those
     * are always created with parent == NULL_WIN, so they still float as
     * independent top-level windows as intended.
     * (Bug fixed here: W_PLAIN was previously treated the same as W_DOC,
     * so createPreviewWindow's preview canvas got wrapped in its own
     * QMdiSubWindow -- with its own QMdiArea sized to fill the ENTIRE
     * EVENT_WINDOW client area, painted OVER all of EVENT_WINDOW's own
     * pre-populated sibling controls (Event Name box, OK/Cancel buttons,
     * etc), making the whole dialog look blank except for a stray
     * "Preview" mini-window floating on top.) */
    bool nestAsChild = parent != NULL_WIN && parentWidget && type == W_DOC;
    bool flatChild = (type == W_NO_BORDER || type == W_PLAIN) && parentWidget;
    auto *win = new XvtWindow(flatChild ? parentWidget : nullptr);
    /* Deliberately NOT setting Qt::SubWindow here (a prior version of
     * this code did): that flag signals "behave like a QMdiSubWindow's
     * internal widget," which is meaningless for a flatChild -- these
     * are plain reparented child widgets, never added to a QMdiArea.
     * Setting it anyway made Qt treat the widget's geometry in an
     * unexpected (screen/desktop-relative rather than parent-relative)
     * coordinate space -- confirmed via gdb: STRATIGRAPHY_WINDOW's own
     * frameGeometry() came back as {top=32767,...}, a clear `short`
     * overflow of some much larger absolute value -- while its properly-
     * populated child list widget (verified: real 7 items) sat
     * invisible inside it, matching a user report of the Stratigraphy
     * layer list not showing despite the list itself now working. */
    o->widget = win;

    const DialogEntry *dlg = findDialogEntry(resId);
    const DialogPositionEntry *titlePosEntry = findPositionEntry(resId);
    if (titlePosEntry && titlePosEntry->titleOverride)
        win->setWindowTitle(QString::fromLocal8Bit(titlePosEntry->titleOverride));
    else if (dlg && dlg->title)
        win->setWindowTitle(QString::fromLocal8Bit(dlg->title));
    if (title) win->setWindowTitle(QString::fromLocal8Bit(title));

    QMdiSubWindow *subWin = nullptr;
    if (nestAsChild) {
        auto *parentXvt = dynamic_cast<XvtWindow *>(parentWidget);
        QMdiArea *area = parentXvt ? parentXvt->ensureMdiArea() : nullptr;
        if (area) {
            subWin = area->addSubWindow(win);
            subWin->setWindowTitle(win->windowTitle());
            subWin->installEventFilter(win);
        }
    }

    if (rct) {
        if (subWin) {
            /* rct is the caller's intended CONTENT size (e.g. builder.c's
             * toolbar strip computes menuSize.bottom-top as exactly
             * ICON_SIZE+4, expecting that to be the icon row's usable
             * client height). Applying it to subWin's own outer geometry
             * would also consume it for the QMdiSubWindow's title-bar
             * decoration, squeezing the real content area down by that
             * much -- e.g. toolbar icons rendered clipped/cut off
             * vertically. Size the CONTENT widget to rct, then let the
             * subwindow grow to fit around it (title bar added on top),
             * positioned at rct's top-left. */
            win->resize(rct->right - rct->left, rct->bottom - rct->top);
            /* subWin->adjustSize() would size the subwindow from its
             * embedded widget's sizeHint() -- but `win` has no QLayout
             * (dialogs positioned via dialog_positions.cpp use absolute
             * setGeometry, not a layout manager), so its sizeHint() is
             * invalid and adjustSize() shrinks the whole subwindow down
             * to some small Qt default instead of preserving the resize()
             * just above (verified via gdb: produced a garbage/negative-
             * height rect). Compute the title-bar/frame overhead directly
             * from the style instead, so the outer size is always
             * exactly win's content size plus real decoration -- robust
             * regardless of whether win has a layout. */
            int titleBar = subWin->style()->pixelMetric(QStyle::PM_TitleBarHeight, nullptr, subWin);
            int frame = subWin->style()->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth, nullptr, subWin);
            /* One atomic setGeometry() call instead of separate resize()+
             * move() -- some Qt versions only fully re-run a
             * QMdiSubWindow's internal title-bar layout/paint on a true
             * geometry CHANGE event, and the window ends up showing with
             * no visible title bar until manually resized by the user
             * (which finally forces the missed layout pass). Matches a
             * user report of Section/Map (and other MDI-child) windows
             * appearing with no header bar until enlarged vertically. */
            subWin->setGeometry(rct->left, rct->top,
                                 win->width() + 2 * frame, win->height() + titleBar + 2 * frame);
        } else {
            win->setGeometry(toQRect(rct));
        }
    }
    WINDOW h = allocHandle(o);
    win->handle = h;

    if (dlg) {
        /* Pre-populate every known control now (rather than only on first
         * xvt_win_get_ctl access) so the dialog looks complete as soon as
         * it's shown, matching the real app's expectation that all its
         * controls already exist by E_CREATE time. If a manual-screenshot-
         * derived position override exists for this resId, place controls
         * at their reconstructed pixel positions (no layout manager);
         * otherwise fall back to the QFormLayout auto-flow. */
        const DialogPositionEntry *posEntry = findPositionEntry(resId);
        QFormLayout *layout = nullptr;
        if (!posEntry) {
            layout = new QFormLayout();
            o->formLayout = layout;
            win->setLayout(layout);
        }
        for (int i = 0; i < dlg->numCtls; i++) {
            const DialogCtlEntry &ce = dlg->ctls[i];
            const DialogCtlPosition *ctlPos = posEntry ? findCtlPosition(posEntry, ce.ctlId) : nullptr;
            /* skipAutoCreate: this ctlId is a custom control the app
             * creates itself via xvtcm_create/graph_create (e.g.
             * PROFILE_WINDOW_CUSTOM_50, the Profile curve canvas) once its
             * own E_CREATE fires -- auto-populating a generic placeholder
             * here too would leave two different widgets both claiming
             * the same ctlId, and xvt_win_get_ctl's child search would
             * find whichever was created first (this one), never the
             * real custom control. */
            if (ctlPos && ctlPos->skipAutoCreate) continue;
            WIN_TYPE ctlType = (ctlPos && ctlPos->typeOverride != W_NONE) ? ctlPos->typeOverride
                              : (ce.type == W_NONE ? WC_EDIT : ce.type);
            QString label = (ctlPos && ctlPos->labelOverride) ? QString::fromLocal8Bit(ctlPos->labelOverride)
                           : QString::fromLocal8Bit(ce.label);
            createControlWidget(h, win, layout, ce.ctlId,
                                 ctlType,
                                 label, ctlPos, i);
        }
        if (!rct && posEntry && posEntry->width > 0)
            win->resize(posEntry->width, posEntry->height);
        else if (!rct)
            win->resize(420, qMax(120, 28 * dlg->numCtls + 60));
    }

    if (!dlg && !rct) win->resize(320, 200);
    ensureBacking(o, qMax(1, win->width()), qMax(1, win->height()));
    if (subWin) {
        if (!rct) {
            /* Same reasoning as the rct branch above: win's size here was
             * just set via explicit resize() calls (posEntry size, the
             * numCtls heuristic, or the 320x200 fallback), not
             * necessarily backed by a valid sizeHint() (only the
             * QFormLayout auto-flow case actually has one) --
             * subWin->adjustSize() would ignore that and shrink to
             * whatever Qt's default fallback is. Compute the outer size
             * directly from win's current size plus real style overhead
             * instead of trusting adjustSize(). */
            int titleBar = subWin->style()->pixelMetric(QStyle::PM_TitleBarHeight, nullptr, subWin);
            int frame = subWin->style()->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth, nullptr, subWin);
            subWin->resize(win->width() + 2 * frame, win->height() + titleBar + 2 * frame);
        }
        subWin->show();
    } else {
        win->show();
    }
    /* Simulate modality (see pushModalDialog) for genuine "dialog" windows:
     * either a W_DOC resource window nested as an MDI subwindow (e.g.
     * EVENT_WINDOW, created via xvt_win_create_def with a real parent), or
     * a fully independent top-level window from the xvt_dlg_create_def/_res
     * "real dialog" API (parent == NULL_WIN by construction there). Plain
     * document/diagram windows (History, Toolbar, Block Diagram, Section...)
     * go through xvt_win_create instead, so isDialogPath is false for them
     * regardless of type/parent -- and createPositionedWindow's nested
     * per-event sub-panels (STRATIGRAPHY_WINDOW etc) are forced W_NO_BORDER
     * with a real parent, so they take the flatChild path and never get a
     * subWin, correctly excluding them too. */
    if (isDialogPath && (subWin || parent == NULL_WIN)) {
        o->isModalDialog = true;
        pushModalDialog(h);
    }
    return h;
}

/* ==========================================================================
 * Menu bar construction -- see qt_compat/menu_registry.cpp (auto-generated
 * from mainMenu.c's XVT-Design case-comment labels by
 * scratchpad/gen_menu_registry.py). Builds a real QMenuBar the first time
 * a window is created with a non-zero menu_res_id (this app only has one
 * menu bar resource, TASK_MENUBAR, so we don't need to distinguish which
 * one -- any non-zero id gets the same tree).
 * ========================================================================== */
struct MenuNodeEntry { const char *path; const char *parentPath; const char *label; long tag; bool isLeaf; bool initiallyDisabled = false; };
extern const MenuNodeEntry g_menuTree[];
extern const int g_menuTreeCount;

/* Looked up by xvt_menu_set_item_enabled/_checked so those calls affect
 * the real, visible menu (not just the MENU_ITEM bookkeeping tree, which
 * nothing ever renders). Tags are unique across the whole app. */
static QHash<long, QAction *> g_menuActionsByTag;

static void populateMenu(QMenu *menu, const char *parentPath, WINDOW handle)
{
    for (int i = 0; i < g_menuTreeCount; i++) {
        const MenuNodeEntry &n = g_menuTree[i];
        if (!n.parentPath || strcmp(n.parentPath, parentPath) != 0) continue;
        if (n.isLeaf) {
            QAction *act = menu->addAction(QString::fromLocal8Bit(n.label));
            long tag = n.tag;
            QObject::connect(act, &QAction::triggered, [handle, tag](bool) {
                XvtObj *o = objFor(handle);
                if (!o || !o->handler) return;
                EVENT ev{}; ev.type = E_COMMAND;
                ev.v.cmd.tag = tag;
                o->handler(handle, &ev);
            });
            g_menuActionsByTag[n.tag] = act;
            if (n.initiallyDisabled) act->setEnabled(false);
        } else {
            QMenu *sub = menu->addMenu(QString::fromLocal8Bit(n.label));
            populateMenu(sub, n.path, handle);
        }
    }
}

static void buildMenuBar(XvtWindow *win, WINDOW handle)
{
    if (win->menuBar) return; /* already built (e.g. a second xvt_win_create_def call) */
    auto *bar = new QMenuBar(win);
    for (int i = 0; i < g_menuTreeCount; i++) {
        const MenuNodeEntry &n = g_menuTree[i];
        if (n.parentPath) continue; /* only the 6 top-level roots here */
        QMenu *menu = bar->addMenu(QString::fromLocal8Bit(n.label));
        populateMenu(menu, n.path, handle);
    }
    int barHeight = bar->sizeHint().height();
    bar->setGeometry(0, 0, win->width(), barHeight);
    bar->show();
    win->menuBar = bar;
    /* If this window already has an mdiArea (created by an earlier child
     * window nesting into it -- e.g. the "History" window itself is the
     * one that requests TASK_MENUBAR, but by the time xvt_win_create()
     * gets here, makeWindow() has ALREADY nested History into TASK_WIN's
     * mdiArea, sized assuming no menu bar existed yet, i.e. top=0/full
     * height), shrink it down to make room for the bar we just added --
     * otherwise the mdiArea (and everything nested inside it) overlaps
     * the menu bar's row instead of starting below it. Mirrors the
     * top-margin logic in ensureMdiArea() itself, just re-applied now
     * that menuBar exists. */
    if (win->mdiArea) {
        int bottom = win->statBar ? win->statBar->sizeHint().height() : 0;
        win->mdiArea->setGeometry(0, barHeight, win->width(), win->height() - barHeight - bottom);
    }
}

/* Real implementations of qt_compat/statbar.h's status-bar API, backed by
 * a genuine QStatusBar attached to the bottom of the given parent window
 * (in practice always TASK_WIN, the main window -- see builder.c's
 * historyWindow E_CREATE and nodwork2.c's various statbar_set_title calls
 * when a block diagram/section/etc. gains focus). Real XVT's title string
 * is tab-delimited into multiple fields (e.g. "Hold Left Button to
 * Rotate...\t150 Rot.\t30 Az.\t100 %", see nodwork2.c's sprintf with
 * STATUS_BLOCK_HELP) -- the first field is the free-form message, the rest
 * are short fixed fields shown as separate boxes on the right, matching a
 * real screenshot of the status bar. */
static void statbarRebuild(QStatusBar *bar, const char *title)
{
    if (!bar) return;
    const QList<QLabel *> old = bar->findChildren<QLabel *>(QStringLiteral("xvtStatbarSeg"),
                                                              Qt::FindDirectChildrenOnly);
    for (QLabel *l : old) { bar->removeWidget(l); delete l; }
    QStringList parts = QString::fromLocal8Bit(title ? title : "").split(QLatin1Char('\t'));
    bar->showMessage(parts.isEmpty() ? QString() : parts.takeFirst());
    for (const QString &part : parts) {
        auto *lbl = new QLabel(part, bar);
        lbl->setObjectName(QStringLiteral("xvtStatbarSeg"));
        lbl->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        lbl->setContentsMargins(4, 0, 4, 0);
        bar->addPermanentWidget(lbl);
    }
}

WINDOW statbar_create(int, int, int, int, int, int, char **, WINDOW parent, int, long, const char *)
{
    auto *xw = dynamic_cast<XvtWindow *>(widgetFor(parent));
    if (!xw) return NULL_WIN;
    if (!xw->statBar) {
        xw->statBar = new QStatusBar(xw);
        xw->statBar->setSizeGripEnabled(false);
        int h = xw->statBar->sizeHint().height();
        xw->statBar->setGeometry(0, xw->height() - h, xw->width(), h);
        xw->statBar->show();
        if (xw->mdiArea) {
            int top = xw->menuBar ? xw->menuBar->sizeHint().height() : 0;
            xw->mdiArea->setGeometry(0, top, xw->width(), xw->height() - top - h);
        }
    }
    XvtObj *o = new XvtObj();
    o->widget = xw->statBar;
    o->parent = parent;
    /* Deliberately left at the default ctlId == -1 (i.e. counted by
     * xvt_scr_list_wins() the same as a real top-level window): nodLib1.c's
     * tileWindows/cascadeWindows explicitly expect their window list to
     * include a statusBar entry -- they both subtract one from their
     * window count (`if (statusBar) numWindows--;`) AND explicitly skip
     * it by identity in their move loop (`win != statusBar`). Excluding it
     * here instead would make that existing app-level compensation
     * over-subtract by one. Real XVT's status bar apparently *was*
     * enumerable the same way. */
    return allocHandle(o);
}

void statbar_set_title(WINDOW sb, const char *title)
{
    statbarRebuild(qobject_cast<QStatusBar *>(widgetFor(sb)), title);
}

void statbar_set_default_title(WINDOW sb, const char *title)
{
    statbarRebuild(qobject_cast<QStatusBar *>(widgetFor(sb)), title);
}

void statbar_autosize(WINDOW) {}

WINDOW xvt_win_create(WIN_TYPE type, RCT *rct, const char *title, long menu_res_id,
                       WINDOW parent, unsigned long /*style*/, EVENT_MASK /*mask*/,
                       WIN_EVENT_HANDLER eh, long user_data)
{
    WINDOW h = makeWindow(type, rct, title, parent, eh, user_data);
    /* menu_res_id == 1000 (TASK_MENUBAR, nodInc.h) is the only menu
     * resource with real reconstructed data (see menu_registry.cpp's
     * provenance) -- other values (e.g. MENU_BAR_2 == 1025, used by the
     * Line Map window) have no known real menu structure, and giving
     * them the SAME TASK_MENUBAR tree would be wrong (the full main
     * app menu on a small diagram window instead of its own smaller
     * one). Those windows build their own menu separately anyway (e.g.
     * nodLib1.c's createLineMapMenubar, called right after
     * xvt_win_create in nodwork1.c's updateLineMap) via xvt_res_get_menu/
     * xvt_menu_set_tree, independent of this function. */
    if (menu_res_id == 1000) {
        /* noddy.c creates the main content window ("History"/" Noddy ")
         * with menu_res_id=TASK_MENUBAR and parent=TASK_WIN -- classic
         * Windows MDI menu-merging semantics: an MDI CHILD can specify a
         * menu resource, but the FRAME displays it (one menu bar at the
         * top of the outer app window, not floating inside each child).
         * Building the QMenuBar as a child of the REQUESTING window (as
         * this used to do) put it inside History's own QMdiSubWindow
         * instead of at the top of the actual outer TASK_WIN frame --
         * user-visible as "menus have disappeared" (buried inside a
         * nested child instead of the natural top-of-app location) and
         * "menus should always be on parent window, never on maps or
         * history window". Attach the bar to the PARENT's widget when a
         * real parent exists (falling back to the requesting window
         * itself otherwise, e.g. if TASK_WIN somehow has no parent of
         * its own) -- `handle` (h, not the parent) is still threaded
         * through for E_COMMAND dispatch, since the menu's COMMANDS
         * conceptually still belong to the window that requested them. */
        QWidget *parentWidget = widgetFor(parent);
        QWidget *menuHost = parentWidget ? parentWidget : widgetFor(h);
        if (menuHost)
            buildMenuBar(static_cast<XvtWindow *>(menuHost), h);
    }
    return h;
}

WINDOW xvt_win_create_def(WIN_DEF *def, WINDOW parent, EVENT_MASK /*mask*/,
                           WIN_EVENT_HANDLER eh, long user_data)
{
    WIN_TYPE type = def ? def->wtype : W_DOC;
    RCT *rct = def ? &def->rct : nullptr;
    const char *title = def ? def->text : nullptr;
    long resId = def ? def->resId : 0;
    return makeWindow(type, rct, title, parent, eh, user_data, resId, true);
}

WINDOW xvt_win_get_ctl(WINDOW win, int ctl_id)
{
    QWidget *parent = widgetFor(win);
    if (!parent) return NULL_WIN;
    const auto children = parent->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget *c : children) {
        if (c->property("xvtCtlId").toInt() == ctl_id)
            return (WINDOW)c->property("xvtHandle").toLongLong();
    }
    /* Not pre-populated from the registry (unknown dialog, or a control ID
     * the registry didn't have) -- auto-vivify a generic, upgradeable
     * placeholder so the app doesn't just get NULL_WIN back. EXCEPT for
     * custom controls (see XvtObj::isCustomCtl) -- callers querying one of
     * those for a ctlId that isn't really its own child expect a genuine
     * NULL_WIN so they can retry against its parent window instead. */
    XvtObj *parentObj = objFor(win);
    if (!parentObj) return NULL_WIN;
    if (parentObj->isCustomCtl) return NULL_WIN;
    if (!parentObj->formLayout) {
        auto *layout = new QFormLayout();
        parentObj->formLayout = layout;
        if (!parent->layout()) parent->setLayout(layout);
    }
    const DialogEntry *dlg = findDialogEntry(parentObj->resId);
    const DialogCtlEntry *ce = findCtlEntry(dlg, ctl_id);
    WIN_TYPE type = (ce && ce->type != W_NONE) ? ce->type : WC_EDIT;
    QString label = ce ? QString::fromLocal8Bit(ce->label) : QString("Ctl %1").arg(ctl_id);
    return createControlWidget(win, parent, parentObj->formLayout, ctl_id, type, label);
}

void xvt_win_set_cursor(WINDOW win, int cursor_id)
{
    QWidget *w = widgetFor(win);
    if (!w) return;
    switch (cursor_id) {
    case CURSOR_CROSS: w->setCursor(Qt::CrossCursor); break;
    case CURSOR_WAIT:  w->setCursor(Qt::WaitCursor); break;
    case CURSOR_ARROW:
    default:           w->setCursor(Qt::ArrowCursor); break;
    }
}

long xvt_win_dispatch_event(WINDOW win, EVENT *ev)
{
    XvtObj *o = objFor(win);
    if (o && o->handler && ev) return o->handler(win, ev);
    return -1L;
}

/* TXEDIT: real XVT's multi-line/multi-paragraph text-display control --
 * distinct from the single-line WC_EDIT controls the rest of this file
 * maps to plain QLineEdit (which can't show multiple lines). Only one
 * real, live call site in the whole app (nodLib2.c's XYZ-import file
 * preview: reads the first 10 lines of the file being imported into a
 * read-only multi-line preview box) -- xvt_tx_process_event's "always a
 * correct no-op" reasoning does NOT apply here, this is a genuinely
 * different control type that needs real backing. Find-or-create by
 * ctlId, same pattern as xvt_win_get_ctl, but backed by QPlainTextEdit
 * and not routed through the dialog_registry.cpp/dialog_positions.cpp
 * system (this accessor is a separate real-XVT API, not xvt_win_get_ctl,
 * so no registry entry would apply anyway). */
TXEDIT xvt_win_get_tx(WINDOW win, int ctl_id)
{
    QWidget *parent = widgetFor(win);
    if (!parent) return 0;
    const auto children = parent->findChildren<QPlainTextEdit *>(QString(), Qt::FindDirectChildrenOnly);
    for (QPlainTextEdit *c : children)
        if (c->property("xvtCtlId").toInt() == ctl_id)
            return (TXEDIT)c->property("xvtHandle").toLongLong();
    auto *w = new QPlainTextEdit(parent);
    w->setReadOnly(true);
    w->setGeometry(10, 10, 260, 100);
    w->show();
    XvtObj *o = new XvtObj();
    o->widget = w;
    o->parent = win;
    o->type = WC_EDIT;
    o->ctlId = ctl_id;
    WINDOW h = allocHandle(o);
    w->setProperty("xvtCtlId", ctl_id);
    return (TXEDIT)h;
}
void xvt_win_trap_pointer(WINDOW win) { g_trappedWindow = win; }
void xvt_win_release_pointer(void) { g_trappedWindow = NULL_WIN; }

WINDOW xvt_dlg_create_res(WIN_TYPE dlg_type, long dlg_res_id, EVENT_MASK /*mask*/,
                           WIN_EVENT_HANDLER eh, long user_data)
{
    return makeWindow(dlg_type, nullptr, nullptr, NULL_WIN, eh, user_data, dlg_res_id, true);
}

WINDOW xvt_dlg_create_def(WIN_DEF *def, EVENT_MASK /*mask*/, WIN_EVENT_HANDLER eh, long user_data)
{
    WIN_TYPE type = def ? def->wtype : WD_MODAL;
    RCT *rct = def ? &def->rct : nullptr;
    long resId = def ? def->resId : 0;
    return makeWindow(type, rct, nullptr, NULL_WIN, eh, user_data, resId, true);
}

/* ==========================================================================
 * Controls
 * ========================================================================== */
WINDOW xvt_ctl_create(WIN_TYPE type, RCT *rct, const char *title, WINDOW parent,
                       long flags, long /*font_id*/, int ctl_id)
{
    /* Direct ad-hoc control creation (not part of a registered dialog),
     * used e.g. by builder.c's createPreviewWindow for the small preview
     * popup's "On"/"Preview Type" controls. Reuses the same
     * createControlWidget() the dialog registry system uses, positioned
     * at the caller's explicit rct instead of registry-driven placement. */
    QWidget *parentWidget = widgetFor(parent);
    if (!parentWidget) return NULL_WIN;
    DialogCtlPosition pos{ ctl_id, 0, 0, 0, 0 };
    if (rct) { pos.left = rct->left; pos.top = rct->top; pos.right = rct->right; pos.bottom = rct->bottom; }
    QString label = title ? QString::fromLocal8Bit(title) : QString();
    WINDOW h = createControlWidget(parent, parentWidget, nullptr, ctl_id, type, label, rct ? &pos : nullptr);
    /* `flags` was previously discarded entirely -- createPreviewWindow
     * creates its "On" checkbox with CTL_FLAG_CHECKED expecting it to
     * start checked (update3dPreview immediately returns without drawing
     * anything if it isn't, see xvt_ctl_is_checked's only real caller),
     * and its "Preview Type Options" combo with CTL_FLAG_INVISIBLE. Both
     * silently no-op'd, matching a user report of no preview ever
     * rendering in any dialog (block/event/options), including the
     * checkbox itself never appearing checked. */
    QWidget *w = widgetFor(h);
    if (w) {
        if (flags & CTL_FLAG_CHECKED) {
            if (auto *cb = qobject_cast<QCheckBox *>(w)) cb->setChecked(true);
            else if (auto *rb = qobject_cast<QRadioButton *>(w)) rb->setChecked(true);
        }
        if (flags & CTL_FLAG_INVISIBLE) w->setVisible(false);
    }
    return h;
}

/* Backs xvtcm_create() (qt_compat/xvtcm.h), real XVT's "Custom Control
 * Manager" API for hand-coded custom-drawn controls -- graph.c's
 * graph_create() (the Profile window's point-editing curve canvas,
 * PROFILE_WINDOW_CUSTOM_50) is the one real call site. xvtcm_create was a
 * complete no-op stub (always returned NULL_WIN), so the canvas was never
 * actually created at all -- graph_eh (the handler with all the curve-
 * drawing/point-drag logic) never got attached to a real window, matching
 * a user report of the Profile curve editor's drawing panel being blank.
 * Unlike xvt_ctl_create (createControlWidget, for standard widget types),
 * this needs a genuine XvtWindow -- backing QImage, paintEvent, full
 * E_MOUSE_DOWN/MOVE/UP/E_UPDATE dispatch -- since graph_eh both draws
 * arbitrary vector content (DrawProfileGraph) and handles free-form mouse
 * dragging (profilePointMove), neither of which a plain Qt control
 * supports. W_NO_BORDER (like createPositionedWindow's sub-panels) makes
 * makeWindow reparent it as a flat, chromeless child of parent_win rather
 * than a floating top-level window. ctlId is set afterward so
 * xvt_win_get_ctl(parent_win, cid) can find it, matching every other
 * control's lookup convention. */
WINDOW xvt_custom_ctl_create(int cid, RCT *rct, WINDOW parent, WIN_EVENT_HANDLER eh)
{
    WINDOW h = makeWindow(W_NO_BORDER, rct, nullptr, parent, eh, 0L);
    XvtObj *o = objFor(h);
    if (o) { o->ctlId = cid; o->isCustomCtl = true; }
    if (QWidget *w = widgetFor(h)) w->setProperty("xvtCtlId", cid);
    return h;
}

BOOLEAN xvt_ctl_is_checked(WINDOW ctl_win)
{
    QWidget *w = widgetFor(ctl_win);
    if (auto *cb = qobject_cast<QCheckBox *>(w)) return cb->isChecked() ? TRUE : FALSE;
    if (auto *rb = qobject_cast<QRadioButton *>(w)) return rb->isChecked() ? TRUE : FALSE;
    return FALSE;
}

void xvt_ctl_set_checked(WINDOW ctl_win, BOOLEAN checked)
{
    QWidget *w = widgetFor(ctl_win);
    if (auto *cb = qobject_cast<QCheckBox *>(w)) cb->setChecked(checked != FALSE);
    else if (auto *rb = qobject_cast<QRadioButton *>(w)) rb->setChecked(checked != FALSE);
}

void xvt_ctl_check_radio_button(WINDOW checked, WINDOW *group, int count)
{
    for (int i = 0; i < count; i++)
        xvt_ctl_set_checked(group[i], group[i] == checked ? TRUE : FALSE);
}

long xvt_ctl_get_id(WINDOW ctl_win)
{
    QWidget *w = widgetFor(ctl_win);
    return w ? w->property("xvtCtlId").toInt() : -1;
}

/* ==========================================================================
 * Lists
 * ========================================================================== */
/* xvt_list_* works against either a QListWidget (WC_LBOX/WC_LISTEDIT) or
 * a QComboBox (WC_LISTBUTTON, see createControlWidget) -- real XVT's
 * list-manipulation API is the same regardless of which concrete list
 * control it's backing. Each function below tries QListWidget first,
 * then QComboBox. */
/* Defined further down (after the SLIST struct itself is fully declared,
 * see s_xvt_slist) -- forward-declared here so xvt_list_add can use it. */
static void bulkAddFromSlist(QWidget *w, SLIST l);

void xvt_list_add(WINDOW list_win, int index, const char *text)
{
    QWidget *w = widgetFor(list_win);
    if (!text) return;
    /* index == -1 is genuinely ambiguous across this codebase: most call
     * sites (e.g. eventlib.c's updateStratigraphyOptions, looping over
     * layer names) just use it as a generic "append, don't care about
     * position" convention with perfectly normal text -- confirmed via a
     * real crash (gdb backtrace) when an earlier version of this function
     * treated EVERY index==-1 call as "text is really an SLIST handle"
     * unconditionally, blindly reinterpret_casting a real C string
     * ("Top", a stratigraphy layer name) as an SLIST* and dereferencing
     * garbage. Only nodGraph.c's makeColorList actually uses index==-1
     * for that (real) SLIST-bulk-add idiom (`xvt_list_add(win, -1, (char
     * far *) sortedList)`), and only against ONE specific control
     * (DEFINE_COLOUR_WINDOW_LISTEDIT_41, ctlId 11, inside
     * DEFINE_COLOUR_WINDOW resId 104) -- checking for exactly that
     * control identity distinguishes the two cases safely, without
     * guessing from the pointer value itself (not portable/safe to probe
     * arbitrary memory) and without touching nodGraph.c's original call. */
    XvtObj *listObj = objFor(list_win);
    bool isColourGridBulkAdd = false;
    if (index == -1 && listObj && listObj->ctlId == 11) {
        if (XvtObj *parentObj = objFor(listObj->parent))
            isColourGridBulkAdd = (parentObj->resId == 104);
    }
    if (isColourGridBulkAdd) {
        bulkAddFromSlist(w, (SLIST)(void *)text);
        return;
    }
    if (auto *lw = qobject_cast<QListWidget *>(w)) {
        auto *item = new QListWidgetItem(QString::fromLocal8Bit(text));
        bool old = lw->blockSignals(true);
        if (index < 0 || index >= lw->count()) lw->addItem(item);
        else lw->insertItem(index, item);
        lw->blockSignals(old);
    } else if (auto *combo = qobject_cast<QComboBox *>(w)) {
        /* Adding a combo's very first item makes Qt auto-select it (a
         * combo can't have no current index once non-empty), which fires
         * currentIndexChanged -- same fake "user changed this" problem as
         * xvt_list_set_sel, just triggered by population instead of
         * selection. Every dialog that fills a dropdown in a loop (most
         * of them) hit this on the first xvt_list_add call, before the
         * caller's own xvt_list_set_sel ever got a chance to run. */
        QString qtext = QString::fromLocal8Bit(text);
        bool old = combo->blockSignals(true);
        if (index < 0 || index >= combo->count()) combo->addItem(qtext);
        else combo->insertItem(index, qtext);
        combo->blockSignals(old);
    }
}

void xvt_list_clear(WINDOW list_win)
{
    QWidget *w = widgetFor(list_win);
    if (auto *lw = qobject_cast<QListWidget *>(w)) lw->clear();
    else if (auto *combo = qobject_cast<QComboBox *>(w)) combo->clear();
}

int xvt_list_get_sel_index(WINDOW list_win)
{
    QWidget *w = widgetFor(list_win);
    if (auto *lw = qobject_cast<QListWidget *>(w)) return lw->currentRow();
    if (auto *combo = qobject_cast<QComboBox *>(w)) return combo->currentIndex();
    return -1;
}

BOOLEAN xvt_list_get_first_sel(WINDOW list_win, char *buf, int maxlen)
{
    QWidget *w = widgetFor(list_win);
    if (!buf) return FALSE;
    QString text;
    if (auto *lw = qobject_cast<QListWidget *>(w)) {
        QList<QListWidgetItem *> sel = lw->selectedItems();
        if (sel.isEmpty()) return FALSE;
        text = sel.first()->text();
    } else if (auto *combo = qobject_cast<QComboBox *>(w)) {
        if (combo->currentIndex() < 0) return FALSE;
        text = combo->currentText();
    } else {
        return FALSE;
    }
    QByteArray b = text.toLocal8Bit();
    int n = qMin(maxlen - 1, b.size());
    memcpy(buf, b.constData(), n);
    buf[n] = '\0';
    return TRUE;
}

BOOLEAN xvt_list_set_sel(WINDOW list_win, int index, BOOLEAN sel)
{
    /* blockSignals around the programmatic selection: QComboBox's
     * currentIndexChanged (and QListWidget's itemSelectionChanged) are
     * wired straight to sendControlEvent() as if the user had operated
     * the control -- every dialog's E_CREATE handler calls this to
     * initialize dropdowns/lists to match already-stored option data, so
     * without blocking, simply opening or refreshing a dialog fake-fired
     * a full "user changed this" E_CONTROL event for every such control.
     * Several of those handlers call update3dPreview() (cursor
     * WAIT/ARROW flicker matches a user report of the cursor flickering
     * "the whole time any dialog is open") and/or re-load all the
     * dialog's other widgets from stored state, stomping on anything the
     * user had just manually toggled a moment earlier (matches a report
     * of checkboxes "check/uncheck but not stick"). Same class of bug
     * already fixed for xvt_sbar_set_pos/_set_range earlier -- this was
     * the same gap for list/combo selection. */
    QWidget *w = widgetFor(list_win);
    if (auto *lw = qobject_cast<QListWidget *>(w)) {
        if (index < 0 || index >= lw->count()) return FALSE;
        bool old = lw->blockSignals(true);
        lw->item(index)->setSelected(sel != FALSE);
        if (sel) lw->setCurrentRow(index);
        lw->blockSignals(old);
        return TRUE;
    }
    if (auto *combo = qobject_cast<QComboBox *>(w)) {
        if (index < 0 || index >= combo->count()) return FALSE;
        if (sel) {
            bool old = combo->blockSignals(true);
            combo->setCurrentIndex(index);
            combo->blockSignals(old);
        }
        return TRUE;
    }
    return FALSE;
}

BOOLEAN xvt_list_is_sel(WINDOW list_win, int index)
{
    QWidget *w = widgetFor(list_win);
    if (auto *lw = qobject_cast<QListWidget *>(w))
        return (index >= 0 && index < lw->count() && lw->item(index)->isSelected()) ? TRUE : FALSE;
    if (auto *combo = qobject_cast<QComboBox *>(w))
        return (index >= 0 && index == combo->currentIndex()) ? TRUE : FALSE;
    return FALSE;
}

int xvt_list_count_all(WINDOW list_win)
{
    QWidget *w = widgetFor(list_win);
    if (auto *lw = qobject_cast<QListWidget *>(w)) return lw->count();
    if (auto *combo = qobject_cast<QComboBox *>(w)) return combo->count();
    return 0;
}

void xvt_list_suspend(WINDOW list_win)
{
    QWidget *w = widgetFor(list_win);
    if (!w) return;
    w->setUpdatesEnabled(false);
    /* Real XVT's suspend/resume also suppresses the control's own change
     * notifications during bulk population -- e.g. viewop.c's E_CREATE
     * populates a cube-size combo via a run of xvt_list_add() calls
     * between suspend/resume, expecting no callback until the caller is
     * done. Without blockSignals, QComboBox::currentIndexChanged fires the
     * moment the first item is added (selection goes from -1 to 0),
     * synthesizing an E_CONTROL event that reenters the window's handler
     * *during* E_CREATE, before setup (e.g. xvt_vobj_set_data) has run --
     * a reentrancy bug, not just a cosmetic flicker. */
    w->blockSignals(true);
}

void xvt_list_resume(WINDOW list_win)
{
    QWidget *w = widgetFor(list_win);
    if (!w) return;
    w->setUpdatesEnabled(true);
    w->blockSignals(false);
}

/* ==========================================================================
 * Scrollbars
 * ========================================================================== */
int xvt_sbar_get_pos(WINDOW win, SCROLL_TYPE /*axis*/)
{
    auto *sb = qobject_cast<QScrollBar *>(widgetFor(win));
    return sb ? sb->value() : 0;
}

void xvt_sbar_set_pos(WINDOW win, SCROLL_TYPE /*axis*/, int pos)
{
    auto *sb = qobject_cast<QScrollBar *>(widgetFor(win));
    if (!sb) return;
    /* Real XVT's xvt_sbar_set_pos is a silent, programmatic set -- it does
     * NOT synthesize an E_HSCROLL/E_VSCROLL back to the app (only genuine
     * user drag gestures do). App code relies on this: e.g. chairwin.c's
     * CHAIR_WINDOW_eh, on receiving a scroll E_CONTROL, calls
     * updateScrollField -> setScrollPosition -> xvt_sbar_set_pos to sync
     * the bar/text-field pair. Without blocking signals here,
     * QAbstractSlider::setValue's valueChanged fired unconditionally,
     * re-entering the SAME E_CONTROL handler recursively -- confirmed via
     * a user-supplied gdb backtrace, thousands of frames deep
     * (xvt_sbar_set_pos -> setValue -> valueChanged -> sendControlEvent ->
     * CHAIR_WINDOW_eh -> updateScrollField -> setScrollPosition ->
     * xvt_sbar_set_pos -> ...), a real stack-overflow crash. */
    sb->blockSignals(true);
    sb->setValue(pos);
    sb->blockSignals(false);
}

void xvt_sbar_set_range(WINDOW win, SCROLL_TYPE /*axis*/, int lo, int hi)
{
    auto *sb = qobject_cast<QScrollBar *>(widgetFor(win));
    if (!sb) return;
    /* Same reasoning as xvt_sbar_set_pos: setRange() can silently clamp
     * (and thus change) the current value, which would fire valueChanged
     * and re-enter the app's E_CONTROL handler just like the unguarded
     * setValue did. */
    sb->blockSignals(true);
    sb->setRange(lo, hi);
    sb->blockSignals(false);
}

void xvt_sbar_get_range(WINDOW win, SCROLL_TYPE, int *lo, int *hi)
{
    auto *sb = qobject_cast<QScrollBar *>(widgetFor(win));
    if (lo) *lo = sb ? sb->minimum() : 0;
    if (hi) *hi = sb ? sb->maximum() : 0;
}

/* ==========================================================================
 * Menus -- a real QMenuBar is not yet wired to each top-level XvtWindow (no
 * visible menu appears yet), but the app's own menu-bookkeeping code (see
 * nodLib1.c's updateWindowsMenu/updateMenuOptions) expects xvt_menu_get_tree
 * to hand back a real, persistent, mutable MENU_ITEM array it can realloc
 * children onto -- returning nullptr (the old stub) segfaulted the very
 * first time an interactive window was shown. Each window gets its own
 * lazily-allocated tree, sized generously (32 top-level entries; the app's
 * own NUM_MENUS is 6) since our compat layer has no visibility into the
 * app's menu-bar layout constants.
 * ========================================================================== */
#define XVT_MENU_TREE_SIZE 32

static MENU_ITEM *findMenuItemByTag(MENU_ITEM *tree, int count, MENU_TAG tag)
{
    /* Only scans the top-level array, not into .child sub-arrays: those
     * are app-managed/realloc'd (see updateWindowsMenu in nodLib1.c) with
     * no count stored alongside them here, and much of that memory is
     * left uninitialized by the app itself (it originally expected
     * resource-file-provided static entries qt_compat has no equivalent
     * of yet -- see dialog_registry.cpp's README for the same root issue
     * with dialogs). Scanning into it without a trustworthy bound risks
     * reading garbage. Top-level-only lookup is safe and covers the
     * common enable/disable-a-known-menu-item case. */
    if (!tree) return nullptr;
    for (int i = 0; i < count; i++)
        if (tree[i].tag == tag) return &tree[i];
    return nullptr;
}

void xvt_menu_set_item_enabled(WINDOW win, MENU_TAG tag, BOOLEAN enabled)
{
    if (QAction *act = g_menuActionsByTag.value(tag, nullptr))
        act->setEnabled(enabled);
    XvtObj *o = objFor(win);
    if (!o || !o->menuTree) return;
    if (MENU_ITEM *m = findMenuItemByTag(o->menuTree, XVT_MENU_TREE_SIZE, tag))
        m->enabled = enabled;
}

void xvt_menu_set_item_checked(WINDOW win, MENU_TAG tag, BOOLEAN checked)
{
    /* TASK_MENUBAR_77_18 ("Legend", Windows menu): real XVT reuses this one
     * static tag as a stand-in for a dynamically-populated "list of open
     * windows" section (see mainMenu.c's raiseWindow(tag-...-2001) tag
     * arithmetic) that this port doesn't build. With only the static
     * "Legend" entry present, toggling it checkable just leaves a
     * meaningless checkbox permanently in the Windows menu -- user-reported
     * as "no reason for it". Skip the checkable styling for this tag only;
     * every other caller of this function (event/symbol type selection
     * menus in nodwork2.c) still gets real checkmarks. */
    if (tag == 1018 /* TASK_MENUBAR_77_18, "Legend" -- nodInc.h isn't included here */) return;
    if (QAction *act = g_menuActionsByTag.value(tag, nullptr)) {
        act->setCheckable(true);
        act->setChecked(checked);
    }
    XvtObj *o = objFor(win);
    if (!o || !o->menuTree) return;
    if (MENU_ITEM *m = findMenuItemByTag(o->menuTree, XVT_MENU_TREE_SIZE, tag))
        m->checked = checked;
}

/* Populates a real QMenu from a MENU_ITEM sibling run (real XVT's array
 * convention -- walk until a tag==0 sentinel entry, see the s_mitem
 * comment in the real SDK header; .child != nullptr means a submenu, so
 * recurse instead of adding a leaf action). Mirrors populateMenu() above,
 * which does the same thing from the static g_menuTree/MenuNodeEntry data
 * -- this one walks the app's own dynamically-built MENU_ITEM trees
 * instead (xvt_menu_set_tree's whole point in real XVT). */
static void populateMenuFromItems(QMenu *menu, MENU_ITEM *items, WINDOW handle)
{
    if (!items) return;
    for (int i = 0; items[i].tag != 0; i++) {
        MENU_ITEM &m = items[i];
        if (!m.text) continue;
        if (m.child) {
            QMenu *sub = menu->addMenu(QString::fromLocal8Bit(m.text));
            populateMenuFromItems(sub, m.child, handle);
        } else {
            QAction *act = menu->addAction(QString::fromLocal8Bit(m.text));
            act->setEnabled(m.enabled != FALSE);
            if (m.checkable) { act->setCheckable(true); act->setChecked(m.checked != FALSE); }
            long tag = m.tag;
            QObject::connect(act, &QAction::triggered, [handle, tag](bool) {
                XvtObj *o = objFor(handle);
                if (!o || !o->handler) return;
                EVENT ev{}; ev.type = E_COMMAND;
                ev.v.cmd.tag = tag;
                o->handler(handle, &ev);
            });
            g_menuActionsByTag[m.tag] = act;
        }
    }
}

void xvt_menu_set_tree(WINDOW win, DATA_PTR tree)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    o->menuTree = (MENU_ITEM *)tree;
    /* Real XVT's xvt_menu_set_tree attaches a REAL, visible menu bar to
     * the window -- this previously only updated the bookkeeping pointer
     * (used by xvt_menu_set_item_enabled/_checked), never built anything
     * on screen. That's the reason Section/Line-Map windows (which build
     * their own menu via createLineMapMenubar -> xvt_menu_set_tree
     * instead of going through xvt_win_create's menu_res_id, see that
     * function's own comment) appeared to have no menu bar at all --
     * confirmed by reading createLineMapMenubar in nodLib1.c, which does
     * call this, just to no visible effect. Only top-level windows
     * (ctlId == -1) are ever actually XvtWindow*; controls share the same
     * handle table but are plain Qt widget subclasses. */
    if (o->ctlId != -1 || !o->widget) return;
    auto *xw = dynamic_cast<XvtWindow *>(o->widget);
    if (!xw) return;
    QMenuBar *bar = xw->menuBar;
    if (bar) {
        /* clear() deletes this bar's QActions -- g_menuActionsByTag is
         * shared with the main app menu bar (populateMenu/buildMenuBar
         * above), so it must NOT be blanket-cleared here; entries for
         * tags in the tree we're about to populate get overwritten below
         * either way. */
        bar->clear();
    } else {
        bar = new QMenuBar(xw);
        xw->menuBar = bar;
    }
    /* Top-level entries are always menu roots (QMenuBar only ever holds
     * QMenus, not leaf QActions), so add one QMenu per top-level item and
     * populate its children -- mirrors buildMenuBar()/populateMenu()
     * above for the static g_menuTree case. */
    for (MENU_ITEM *items = (MENU_ITEM *)tree; items && items->tag != 0; items++) {
        if (!items->text) continue;
        QMenu *menu = bar->addMenu(QString::fromLocal8Bit(items->text));
        populateMenuFromItems(menu, items->child, win);
    }
    int barHeight = bar->sizeHint().height();
    bar->setGeometry(0, 0, xw->width(), barHeight);
    bar->show();
    if (xw->mdiArea)
        xw->mdiArea->setGeometry(0, barHeight, xw->width(), xw->height() - barHeight);
}

DATA_PTR xvt_menu_get_tree(WINDOW win)
{
    XvtObj *o = objFor(win);
    if (!o) return nullptr;
    if (!o->menuTree)
        o->menuTree = (MENU_ITEM *)calloc(XVT_MENU_TREE_SIZE, sizeof(MENU_ITEM));
    return o->menuTree;
}

void xvt_menu_update(WINDOW win)
{
    /* xvt_menu_set_item_enabled/_checked already update the real QAction
     * live (Qt repaints menus on its own), so this doesn't need to do
     * more than nudge a repaint in case anything is pending. Only
     * top-level windows (ctlId == -1) are ever actually XvtWindow*
     * instances -- controls share the same handle table but are plain
     * Qt widget subclasses, so guard the cast. */
    XvtObj *o = objFor(win);
    if (!o || !o->widget || o->ctlId != -1) return;
    auto *xw = static_cast<XvtWindow *>(o->widget);
    if (xw->menuBar) xw->menuBar->update();
}

/* ==========================================================================
 * Message / file dialogs
 * ========================================================================== */
static QString vformat(const char *fmt, va_list ap)
{
    char buf[2048];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    return QString::fromLocal8Bit(buf);
}

void xvt_dm_post_error(const char *fmt, ...)
{
    ensureQApp();
    va_list ap; va_start(ap, fmt);
    QString msg = vformat(fmt, ap);
    va_end(ap);
    QMessageBox::critical(nullptr, "Noddy", msg);
}

void xvt_dm_post_note(const char *fmt, ...)
{
    ensureQApp();
    va_list ap; va_start(ap, fmt);
    QString msg = vformat(fmt, ap);
    va_end(ap);
    QMessageBox::information(nullptr, "Noddy", msg);
}

int xvt_dm_post_ask(const char *b1, const char *b2, const char *b3, const char *fmt, ...)
{
    ensureQApp();
    va_list ap; va_start(ap, fmt);
    QString msg = vformat(fmt, ap);
    va_end(ap);
    QMessageBox box(QMessageBox::Question, "Noddy", msg);
    QPushButton *btn1 = b1 ? box.addButton(QString::fromLocal8Bit(b1), QMessageBox::AcceptRole) : nullptr;
    QPushButton *btn2 = b2 ? box.addButton(QString::fromLocal8Bit(b2), QMessageBox::RejectRole) : nullptr;
    if (b3) box.addButton(QString::fromLocal8Bit(b3), QMessageBox::ActionRole);
    box.exec();
    if (box.clickedButton() == btn1) return 1;
    if (box.clickedButton() == btn2) return 2;
    return 0; /* RESP_DEFAULT */
}

FL_STATUS xvt_dm_post_file_open(FILE_SPEC *fs, const char *prompt)
{
    ensureQApp();
    if (!fs) return FL_BAD;
    QString name = QFileDialog::getOpenFileName(nullptr, prompt ? QString::fromLocal8Bit(prompt) : "Open");
    if (name.isEmpty()) return FL_CANCEL;
    QFileInfo fi(name);
    strncpy(fs->name, fi.fileName().toLocal8Bit().constData(), SZ_FNAME);
    fs->name[SZ_FNAME] = '\0';
    strncpy(fs->dir.path, fi.absolutePath().toLocal8Bit().constData(), sizeof(fs->dir.path) - 1);
    return FL_OK;
}

FL_STATUS xvt_dm_post_file_save(FILE_SPEC *fs, const char *prompt)
{
    ensureQApp();
    if (!fs) return FL_BAD;
    QString name = QFileDialog::getSaveFileName(nullptr, prompt ? QString::fromLocal8Bit(prompt) : "Save");
    if (name.isEmpty()) return FL_CANCEL;
    QFileInfo fi(name);
    strncpy(fs->name, fi.fileName().toLocal8Bit().constData(), SZ_FNAME);
    fs->name[SZ_FNAME] = '\0';
    strncpy(fs->dir.path, fi.absolutePath().toLocal8Bit().constData(), sizeof(fs->dir.path) - 1);
    return FL_OK;
}

BOOLEAN xvt_dm_post_string_prompt(const char *prompt, char *buf, int maxlen)
{
    ensureQApp();
    bool ok = false;
    QString defaultText = (buf && buf[0]) ? QString::fromLocal8Bit(buf) : QString();
    QString text = QInputDialog::getText(nullptr, "Noddy", prompt ? QString::fromLocal8Bit(prompt) : "", QLineEdit::Normal, defaultText, &ok);
    if (!ok || !buf) return FALSE;
    QByteArray b = text.toLocal8Bit();
    int n = qMin(maxlen - 1, b.size());
    memcpy(buf, b.constData(), n);
    buf[n] = '\0';
    return TRUE;
}

/* Real implementation lives beside the rest of the print machinery,
 * further down (needs XvtPrintState, defined there). Declared here only
 * because its natural home in the file (grouped with other xvt_dm_post_*
 * dialogs) is far above that. */
BOOLEAN xvt_dm_post_page_setup(PRINT_RCD *printer_setup);

/* ==========================================================================
 * Drawing
 * ========================================================================== */
void xvt_dwin_clear(WINDOW win, COLOR color)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    ensureBacking(o, o->widget ? qMax(1, o->widget->width()) : 1, o->widget ? qMax(1, o->widget->height()) : 1);
    o->backing.fill(toQColor(color));
    requestRepaint(o);
}

void xvt_dwin_update(WINDOW win)
{
    QWidget *w = widgetFor(win);
    if (w) w->repaint();
}

void xvt_dwin_invalidate_rect(WINDOW win, RCT *rct)
{
    QWidget *w = widgetFor(win);
    if (!w) return;
    /* Same self-triggering-repaint-loop guard as requestRepaint() -- an
     * explicit invalidate call made by an E_UPDATE handler on its own
     * window (a less common but real pattern, e.g. invalidating a
     * sub-region after drawing it) would otherwise queue another paint
     * event on top of the one currently running, same infinite loop. */
    XvtObj *o = objFor(win);
    if (o && !o->updateRegion.isEmpty()) return;
    if (rct) w->update(toQRect(rct));
    else w->update();
}

BOOLEAN xvt_dwin_is_update_needed(WINDOW win, RCT *rct)
{
    XvtObj *o = objFor(win);
    if (!o) return FALSE;
    /* Not currently inside an E_UPDATE dispatch (updateRegion cleared by
     * paintEvent once E_UPDATE returns) -- no restriction in effect, so
     * everything "needs" updating. */
    if (o->updateRegion.isNull()) return TRUE;
    if (!rct) return !o->updateRegion.isEmpty();
    return o->updateRegion.intersects(toQRect(rct));
}

void xvt_dwin_draw_set_pos(WINDOW win, PNT pos)
{
    XvtObj *o = objFor(win);
    if (o) o->drawPos = pos;
}

void xvt_dwin_draw_line(WINDOW win, PNT to)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    QPainter p(&o->backing);
    p.setPen(toQColor(o->foreColor));
    p.drawLine(o->drawPos.h, o->drawPos.v, to.h, to.v);
    o->drawPos = to;
    requestRepaint(o);
}

void xvt_dwin_draw_aline(WINDOW win, PNT to, BOOLEAN /*close*/, BOOLEAN arrowHead)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    QPainter p(&o->backing);
    QColor c = toQColor(o->foreColor);
    p.setPen(c);
    p.drawLine(o->drawPos.h, o->drawPos.v, to.h, to.v);
    if (arrowHead) {
        /* Real XVT's "arrow line" draws a filled arrowhead at the 'to' end
         * -- this port only ever drew the shaft, e.g. legend.c's North
         * Arrow (createBlockDimLegend) ends up as a bare line with an "N"
         * label and no arrowhead. Build a small filled triangle pointing
         * along the line direction. */
        double dx = to.h - o->drawPos.h, dy = to.v - o->drawPos.v;
        double len = std::sqrt(dx * dx + dy * dy);
        if (len > 0.5) {
            dx /= len; dy /= len;
            double headLen = 10.0, headWidth = 4.0;
            double bx = to.h - dx * headLen, by = to.v - dy * headLen;
            double px = -dy, py = dx;
            QPolygonF head;
            head << QPointF(to.h, to.v)
                 << QPointF(bx + px * headWidth, by + py * headWidth)
                 << QPointF(bx - px * headWidth, by - py * headWidth);
            p.setBrush(c);
            p.drawPolygon(head);
        }
    }
    o->drawPos = to;
    requestRepaint(o);
}

/* Real XVT's draw_rect/draw_roundrect/draw_oval/draw_polygon are FILLED
 * shapes (fill = current back color, outline = current fore color) --
 * the classic QuickDraw/GDI-era "draw" convention (as opposed to a
 * separate outline-only "frame" variant), matching why XVT has both
 * xvt_dwin_set_fore_color AND xvt_dwin_set_back_color as distinct
 * concepts. These were only ever setting a pen, never a brush -- Qt's
 * QPainter defaults to Qt::NoBrush (fully transparent fill), so every
 * shape silently rendered as an unfilled outline only. This was the
 * root cause of the block diagram (and any other solid-shaded 3D/2D
 * rendering, e.g. Section/Map diagrams) appearing as wireframe instead
 * of lithology-colored solids -- 3dSolids.c's RenderPolyFacet/
 * RenderRectFacet call xvt_dwin_draw_polygon to fill each rendered
 * facet with its material's color (set via xvt_dwin_set_back_color
 * beforehand). */
void xvt_dwin_draw_rect(WINDOW win, RCT *rct)
{
    XvtObj *o = objFor(win);
    if (!o || !rct) return;
    if (o->drawMode == M_XOR) {
        /* Real XVT's M_XOR draw mode is a classic GDI-style "invert
         * whatever's already there" technique. Two distinct real call
         * sites use it very differently:
         * - builder.c's drawIcon (toolbar selected-tool highlight): a
         *   FILLED invert of the whole rect -- handled below via a manual
         *   pixel-invert loop, unchanged from before.
         * - builder.c's rubberRect (drag-to-create/drag-to-move outline,
         *   PAT_RUBBER pen + PAT_HOLLOW brush): an OUTLINE-only invert, so
         *   calling it twice at the same rect (once to draw, once to
         *   erase before the next position) leaves the interior content
         *   untouched. The filled-invert loop above ignores brush
         *   pattern entirely and would paint a solid inverted blob
         *   instead -- effectively an invisible flash during the fast
         *   erase+redraw-elsewhere cycle of a live drag (matches a user
         *   report: dragging a toolbar icon into History to place a new
         *   event showed no visible drag rectangle at all, unlike the
         *   dashed-rectangle placeholder in the reference app). Use Qt's
         *   raster XOR composition mode with a real stroked (dashable)
         *   rect instead of a manual pixel loop for this case -- draws
         *   only the border, and PAT_RUBBER maps to a dashed line so it
         *   visually matches too. */
        if (o->brushPat == PAT_HOLLOW || o->brushPat == PAT_NONE) {
            QPainter p(&o->backing);
            p.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
            QPen pen(Qt::white); /* colour is irrelevant under XOR -- white flips every channel */
            pen.setWidth(1);
            if (o->penPat == PAT_RUBBER) pen.setStyle(Qt::DashLine);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawRect(toQRect(rct));
            requestRepaint(o);
            return;
        }
        QRect r = toQRect(rct).intersected(o->backing.rect());
        for (int y = r.top(); y <= r.bottom(); y++) {
            uchar *line = o->backing.scanLine(y);
            auto *pixels = reinterpret_cast<QRgb *>(line);
            for (int x = r.left(); x <= r.right(); x++)
                pixels[x] = pixels[x] ^ 0x00FFFFFFu;
        }
        requestRepaint(o);
        return;
    }
    QPainter p(&o->backing);
    if (o->penPat == PAT_HOLLOW || o->penPat == PAT_NONE) p.setPen(Qt::NoPen);
    else p.setPen(toQColor(o->foreColor));
    if (o->brushPat == PAT_HOLLOW || o->brushPat == PAT_NONE) p.setBrush(Qt::NoBrush);
    else p.setBrush(toQColor(o->backColor));
    p.drawRect(toQRect(rct));
    requestRepaint(o);
}

void xvt_dwin_draw_roundrect(WINDOW win, RCT *rct, int ovalw, int ovalh)
{
    XvtObj *o = objFor(win);
    if (!o || !rct) return;
    QPainter p(&o->backing);
    if (o->penPat == PAT_HOLLOW || o->penPat == PAT_NONE) p.setPen(Qt::NoPen);
    else p.setPen(toQColor(o->foreColor));
    if (o->brushPat == PAT_HOLLOW || o->brushPat == PAT_NONE) p.setBrush(Qt::NoBrush);
    else p.setBrush(toQColor(o->backColor));
    p.drawRoundedRect(toQRect(rct), ovalw, ovalh);
    requestRepaint(o);
}

void xvt_dwin_draw_oval(WINDOW win, RCT *rct)
{
    XvtObj *o = objFor(win);
    if (!o || !rct) return;
    QPainter p(&o->backing);
    if (o->penPat == PAT_HOLLOW || o->penPat == PAT_NONE) p.setPen(Qt::NoPen);
    else p.setPen(toQColor(o->foreColor));
    if (o->brushPat == PAT_HOLLOW || o->brushPat == PAT_NONE) p.setBrush(Qt::NoBrush);
    else p.setBrush(toQColor(o->backColor));
    p.drawEllipse(toQRect(rct));
    requestRepaint(o);
}

void xvt_dwin_draw_polygon(WINDOW win, PNT *pts, int n)
{
    XvtObj *o = objFor(win);
    if (!o || !pts || n <= 0) return;
    QVector<QPoint> qpts;
    qpts.reserve(n);
    for (int i = 0; i < n; i++) qpts.append(QPoint(pts[i].h, pts[i].v));
    QPainter p(&o->backing);
    if (o->penPat == PAT_HOLLOW || o->penPat == PAT_NONE) p.setPen(Qt::NoPen);
    else p.setPen(toQColor(o->foreColor));
    if (o->brushPat == PAT_HOLLOW || o->brushPat == PAT_NONE) p.setBrush(Qt::NoBrush);
    else p.setBrush(toQColor(o->backColor));
    p.drawPolygon(qpts.constData(), n);
    requestRepaint(o);
}

void xvt_dwin_draw_polyline(WINDOW win, PNT *pts, int n)
{
    XvtObj *o = objFor(win);
    if (!o || !pts || n <= 0) return;
    QVector<QPoint> qpts;
    qpts.reserve(n);
    for (int i = 0; i < n; i++) qpts.append(QPoint(pts[i].h, pts[i].v));
    QPainter p(&o->backing);
    p.setPen(toQColor(o->foreColor));
    p.drawPolyline(qpts.constData(), n);
    requestRepaint(o);
}

void xvt_dwin_draw_text(WINDOW win, short x, short y, const char *text, int len)
{
    XvtObj *o = objFor(win);
    if (!o || !text) return;
    QString s = len >= 0 ? QString::fromLocal8Bit(text, len) : QString::fromLocal8Bit(text);
    QPainter p(&o->backing);
    p.setPen(toQColor(o->foreColor));
    p.setFont(o->font);
    p.drawText(x, y, s);
    requestRepaint(o);
}

int xvt_dwin_get_text_width(WINDOW win, const char *text, int len)
{
    XvtObj *o = objFor(win);
    if (!o || !text) return 0;
    QString s = len >= 0 ? QString::fromLocal8Bit(text, len) : QString::fromLocal8Bit(text);
    return QFontMetrics(o->font).horizontalAdvance(s);
}

int xvt_dwin_text_width(WINDOW win, const char *text, int len) { return xvt_dwin_get_text_width(win, text, len); }

/* Real icon bitmaps aren't in the missing compiled resource file after
 * all -- the original individual .ICO files used to build that resource
 * are still sitting in the repo root (DYKE.ICO, FAULT.ICO, FOLD.ICO, ...)
 * and were copied into qt_compat/icons/ this session. Icon-ID constants
 * in builder.h (FOLD_ICON=10002 etc.) map directly onto these filenames.
 * A couple of icon IDs (SURFACE_ICON, ELLIPSOID_ICON) have no matching
 * .ICO file -- those, and any load failure (e.g. Qt's ICO plugin isn't
 * installed), fall back to a numbered placeholder glyph. */
static const char *iconFileForId(int icon_id)
{
    /* Authoritative mapping from the real noddy.rc resource file (see
     * qt_compat/original_resources/noddy_shell.rc) -- NOT a guess from
     * builder.h's constant names. Notably corrects one real bug that
     * guessing got wrong: 10005 (SHEAR_ICON in builder.h) actually maps
     * to kink.ico in the shipped resource, not shear.ico -- the
     * SHEAR.ICO file present in the repo is an orphaned/unused leftover
     * (and isn't even a valid ICO file, see the XBM fallback below).
     * Also adds surface.ico/ellscale.ico (10205/10206), which the .rc
     * references but weren't among the 23 .ICO files found so far --
     * falls through to the placeholder glyph if not present on disk. */
    switch (icon_id) {
    case 10000: return "POINTER.ICO";
    case 10001: return "STRAT.ICO";
    case 10002: return "FOLD.ICO";
    case 10003: return "FAULT.ICO";
    case 10004: return "UNCONF.ICO";
    case 10005: return "KINK.ICO";
    case 10006: return "DYKE.ICO";
    case 10007: return "PLUG.ICO";
    case 10008: return "STRAIN.ICO";
    case 10009: return "TILT.ICO";
    case 10010: return "FOLIAT.ICO";
    case 10011: return "LINEAT.ICO";
    case 10012: return "STOP.ICO";
    case 10013: return "IMPORT.ICO";
    case 10014: return "GENERIC.ICO";
    case 10100: return "EPOS.ICO";
    case 10200: return "FORM.ICO";
    case 10201: return "POSITION.ICO";
    case 10202: return "ORIENT.ICO";
    case 10203: return "SCALE.ICO";
    case 10204: return "TIME.ICO";
    case 10205: return "SURFACE.ICO";
    case 10206: return "ELLSCALE.ICO";
    default:    return nullptr;
    }
}

/* Case-insensitive lookup of qt_compat/icons/png/<stem>.png -- the source
 * PNGs live on a Windows/NTFS-backed path (case-preserving but NOT
 * case-sensitive for lookups: renaming "ellscale.png" to "ELLSCALE.png"
 * there doesn't reliably stick, Explorer/Dropbox just treat it as the
 * same file and it can revert), but get copied onto WSL's native Linux
 * filesystem to actually build/run against, which IS case-sensitive --
 * so a source file saved as e.g. "ellscale.png" would silently fail to
 * match an exact-case "ELLSCALE.png" lookup at runtime. Scan the
 * directory once and cache a lowercase-stem -> real-filename map so any
 * case the user happens to save a PNG in still resolves. */
static QString findPngCaseInsensitive(const QString &dirPath, const QString &stem)
{
    static QHash<QString, QHash<QString, QString>> dirCache; /* dirPath -> {lowercase stem -> real filename} */
    auto dit = dirCache.find(dirPath);
    if (dit == dirCache.end()) {
        QHash<QString, QString> map;
        QDir dir(dirPath);
        const auto entries = dir.entryList(QStringList() << "*.png" << "*.PNG", QDir::Files);
        for (const QString &entry : entries) {
            QString entryStem = entry;
            int d = entryStem.lastIndexOf('.');
            if (d >= 0) entryStem = entryStem.left(d);
            map.insert(entryStem.toLower(), entry);
        }
        dit = dirCache.insert(dirPath, map);
    }
    return dit->value(stem.toLower());
}

static const QPixmap *loadIcon(int icon_id)
{
    static QHash<int, QPixmap> cache;
    auto it = cache.constFind(icon_id);
    if (it != cache.constEnd()) return it->isNull() ? nullptr : &(*it);

    QPixmap pm;
    if (const char *fname = iconFileForId(icon_id)) {
        QString base = QCoreApplication::applicationDirPath() + "/qt_compat/icons/";
        /* Prefer a PNG version if present (qt_compat/icons/png/<NAME>.png)
         * -- the original .ICO files are 16-colour indexed and hard to
         * recolor; all 23 were converted to PNG once (ImageMagick) as a
         * starting point for the user to repaint them in color. Loading
         * PNG-first (when present) means any future edit to the PNG
         * takes effect without touching this code again. */
        QString stem = QString::fromLocal8Bit(fname);
        int dot = stem.lastIndexOf('.');
        if (dot >= 0) stem = stem.left(dot);
        QString pngDir = base + "png/";
        QString realName = findPngCaseInsensitive(pngDir, stem);
        QString pngPath = pngDir + (realName.isEmpty() ? stem + ".png" : realName);
        pm.load(pngPath);
        if (pm.isNull()) {
            QString path = base + fname;
            pm.load(path);
            if (pm.isNull()) {
                /* SHEAR.ICO turned out not to be a real .ICO binary at
                 * all -- it's leftover X11/XBM C-source (`#define
                 * shear_icon_width 48` ...) from a different platform
                 * build of this cross-platform app, just carrying a
                 * misleading .ICO extension. Qt has built-in XBM support
                 * (no plugin needed), it just needs the format told to
                 * it explicitly since the extension doesn't match the
                 * real content. Moot now that shear/kink has a real PNG,
                 * kept as a general safety net. */
                pm.load(path, "XBM");
            }
        }
    }
    cache.insert(icon_id, pm);
    return pm.isNull() ? nullptr : &cache[icon_id];
}

void xvt_dwin_draw_icon(WINDOW win, short x, short y, int icon_id)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    QPainter p(&o->backing);
    if (const QPixmap *pm = loadIcon(icon_id)) {
        /* Draw at the .ICO's own native resolution (32x32 for all of
         * these) instead of forcing a fixed box -- forcing e.g. 20x20
         * squashed/distorted them since that doesn't match their actual
         * pixel size. */
        p.drawPixmap(x, y, *pm);
    } else {
        const int size = 20;
        /* No .ICO file for this id (or it failed to load) -- numbered
         * placeholder so the toolbar still shows *something* there. */
        QRect r(x, y, size, size);
        p.setPen(QPen(Qt::darkGray));
        p.setBrush(QColor(220, 220, 220));
        p.drawRoundedRect(r, 3, 3);
        p.setPen(Qt::black);
        QFont f = p.font(); f.setPointSize(7); p.setFont(f);
        p.drawText(r, Qt::AlignCenter, QString::number(icon_id));
    }
    requestRepaint(o);
}

void xvt_dwin_draw_pmap(WINDOW win, XVT_PIXMAP pmap, RCT *dst_rct, RCT *src_rct)
{
    XvtObj *dst = objFor(win);
    XvtObj *src = objFor(pmap);
    if (!dst || !src) return;
    QRect dstRect = dst_rct ? toQRect(dst_rct) : src->backing.rect();
    QRect srcRect = src_rct ? toQRect(src_rct) : src->backing.rect();
    if (dst->printPainter) {
        /* nodLib1.c's printPicture draws the on-screen pixmap straight
         * into the print "window" -- route to the real printer QPainter
         * instead of a (nonexistent) backing image. dstRect there is
         * computed purely from the SOURCE pixmap's own screen-pixel size
         * (scaled by projectOptions.printScalingFactor, default 1.0) with
         * no awareness of the actual page size -- real XVT's banded
         * printing model presumably handled page-fitting via the band
         * iteration itself, which this port collapsed to a single band
         * (see xvt_print_get_next_band). A source diagram taller/wider
         * than the printable page at screen resolution (~96-100dpi, see
         * xvt_print_create's QPrinter::ScreenResolution choice) previously
         * just got drawn past the page edge and silently clipped by Qt --
         * matches a user report of the printed image being "clipped at
         * top and bottom". Scale dstRect down (preserving aspect ratio,
         * never up) to fit the printer's actual printable area, centered,
         * before drawing. */
        QRect pageRect;
        if (auto *printer = dynamic_cast<QPrinter *>(dst->printPainter->device()))
            pageRect = printer->pageLayout().paintRectPixels(printer->resolution());
        if (!pageRect.isEmpty() &&
            (dstRect.width() > pageRect.width() || dstRect.height() > pageRect.height())) {
            double scale = qMin((double)pageRect.width() / dstRect.width(),
                                 (double)pageRect.height() / dstRect.height());
            int fitW = qMax(1, (int)(dstRect.width() * scale));
            int fitH = qMax(1, (int)(dstRect.height() * scale));
            dstRect = QRect((pageRect.width() - fitW) / 2, (pageRect.height() - fitH) / 2, fitW, fitH);
        }
        dst->printPainter->drawImage(dstRect, src->backing, srcRect);
        return;
    }
    QPainter p(&dst->backing);
    if (dst->clipRect.isValid()) p.setClipRect(dst->clipRect);
    p.drawImage(dstRect, src->backing, srcRect);
    requestRepaint(dst);
}

void xvt_dwin_draw_image(WINDOW win, XVT_IMAGE image, RCT *dst, RCT *src)
{
    XvtObj *o = objFor(win);
    auto *impl = reinterpret_cast<XvtImageImpl *>(image);
    if (!o || !impl) return;
    QPainter p(&o->backing);
    if (o->clipRect.isValid()) p.setClipRect(o->clipRect);
    QRect dstRect = dst ? toQRect(dst) : impl->img.rect();
    QRect srcRect = src ? toQRect(src) : impl->img.rect();
    p.drawImage(dstRect, impl->img, srcRect);
    requestRepaint(o);
}
void xvt_dwin_scroll_rect(WINDOW win, RCT *rct, int dh, int dv)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    QRect r = rct ? toQRect(rct) : o->backing.rect();
    QImage snapshot = o->backing.copy(r);
    QPainter p(&o->backing);
    p.drawImage(r.translated(dh, dv), snapshot);
    requestRepaint(o);
}

void xvt_dwin_set_clip(WINDOW win, RCT *rct)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    o->clipRect = rct ? toQRect(rct) : QRect();
}

void xvt_dwin_set_fore_color(WINDOW win, COLOR color)
{
    XvtObj *o = objFor(win);
    if (o) o->foreColor = color;
}

void xvt_dwin_set_back_color(WINDOW win, COLOR color)
{
    XvtObj *o = objFor(win);
    if (o) o->backColor = color;
}

void xvt_dwin_set_font_family(WINDOW win, const char *family)
{
    XvtObj *o = objFor(win);
    if (o && family) o->font.setFamily(QString::fromLocal8Bit(family));
}

void xvt_dwin_set_font_style(WINDOW win, int style)
{
    XvtObj *o = objFor(win);
    if (!o) return;
    o->font.setBold(style & 0x1);
    o->font.setItalic(style & 0x2);
    o->font.setUnderline(style & 0x4);
}

void xvt_dwin_set_font_size(WINDOW win, int size)
{
    XvtObj *o = objFor(win);
    if (o) o->font.setPointSize(size);
}

void xvt_dwin_set_font(WINDOW, DATA_PTR) { XVT_STUB(); }
DATA_PTR xvt_dwin_get_font(WINDOW) { XVT_STUB(); return nullptr; }
void xvt_dwin_set_draw_mode(WINDOW win, int mode)
{
    XvtObj *o = objFor(win);
    if (o) o->drawMode = mode;
}
void xvt_dwin_set_cpen(WINDOW win, CPEN *pen)
{
    XvtObj *o = objFor(win);
    if (o && pen) { o->foreColor = pen->color; o->penPat = pen->pat; }
}
void xvt_dwin_set_std_cpen(WINDOW win, int std_pen)
{
    /* Call sites (e.g. legend.c) pass TL_PEN_BLACK/TL_PEN_RUBBER, which
     * are just COLOR values (see xvt_types.h) truncated through this
     * `int` parameter -- same as xvt_dwin_set_fore_color. */
    XvtObj *o = objFor(win);
    if (o) o->foreColor = (COLOR)std_pen;
}
void xvt_dwin_set_std_cbrush(WINDOW win, int std_brush)
{
    XvtObj *o = objFor(win);
    if (o) o->backColor = (COLOR)std_brush;
}
void xvt_dwin_get_font_metrics(WINDOW win, int *leading, int *ascent, int *descent)
{
    /* Every call site in the whole app (builder.c, legend.c, nodwork1.c,
     * plot.c, proflib.c) declares its ascent/descent/leading locals as
     * plain `int`, never `short` -- this header used to declare `short *`
     * params instead, so each call wrote only the low 2 bytes of the
     * caller's 4-byte int, leaving the upper 2 bytes as uninitialized
     * stack garbage. Text positioned via "point.v + fAscent" (e.g. plot.c's
     * Stratigraphic Column column headers, drawn on two rows offset by
     * fAscent) could then land at a wildly wrong y-offset -- matches a
     * user report of overlapping/garbled header text in that window, and
     * likely the same root cause behind similarly garbled text in other
     * font-metric-driven preview panels. */
    XvtObj *o = objFor(win);
    QFontMetrics fm(o ? o->font : QFont());
    if (leading) *leading = fm.leading();
    if (ascent) *ascent = fm.ascent();
    if (descent) *descent = fm.descent();
}
void xvt_font_get_metrics(DATA_PTR, DATA_PTR) { XVT_STUB(); }

void xvt_dwin_set_draw_ctools(WINDOW win, DRAW_CTOOLS *tools)
{
    XvtObj *o = objFor(win);
    if (!o || !tools) return;
    /* Previously only captured pen.color, silently dropping brush.color
     * and mode -- builder.c's drawIcon uses this to select the toolbar's
     * currently-selected-tool highlight (mode = M_XOR, brush = black),
     * which real XVT renders as an INVERT of whatever's already drawn
     * there (classic GDI-XOR-pen selection-rectangle technique). Losing
     * `mode` meant xvt_dwin_draw_rect always painted a normal filled
     * rect using the stale backColor (white, set moments earlier in the
     * same function) -- covering the selected icon in solid white
     * instead of inverting it (user-visible: the default-selected
     * pointer tool icon appeared to have vanished). */
    o->foreColor = tools->pen.color;
    o->backColor = tools->brush.color;
    o->drawMode = tools->mode;
    /* Real call sites rely on pen/brush PATTERN, not just color, to mean
     * "draw no outline"/"draw no fill" (e.g. DoSect.c's per-cell section
     * fill sets pen.pat = PAT_HOLLOW with an unused placeholder pen.color
     * of COLOR_WHITE, expecting NO border -- draw_rect/polygon/oval/
     * roundrect previously always stroked a solid border in whatever
     * color happened to be set, ignoring pat entirely. That drew a solid
     * white outline around every section/block-diagram cell, i.e. a
     * visible white grid between what should be seamlessly tiled colored
     * rects -- matches user-reported screenshots of both). */
    o->penPat = tools->pen.pat;
    o->brushPat = tools->brush.pat;
}

void xvt_dwin_get_draw_ctools(WINDOW win, DRAW_CTOOLS *tools)
{
    XvtObj *o = objFor(win);
    if (!tools) return;
    tools->pen.color = o ? o->foreColor : COLOR_BLACK;
    tools->pen.width = 1;
    tools->pen.pat = o ? o->penPat : PAT_SOLID;
    tools->brush.color = o ? o->backColor : COLOR_WHITE;
    tools->brush.pat = o ? o->brushPat : PAT_SOLID;
    tools->mode = o ? (DRAW_MODE)o->drawMode : M_COPY;
}

/* ==========================================================================
 * Pixmaps (off-screen drawing surfaces)
 * ========================================================================== */
XVT_PIXMAP xvt_pmap_create(WINDOW /*win*/, int /*type*/, short width, short height, DATA_PTR /*palette*/)
{
    XvtObj *o = new XvtObj();
    o->type = W_PIXMAP;
    o->backing = QImage(qMax(1, (int)width), qMax(1, (int)height), QImage::Format_RGB32);
    o->backing.fill(Qt::white);
    return allocHandle(o);
}

void xvt_pmap_destroy(XVT_PIXMAP pmap)
{
    XvtObj *o = objFor(pmap);
    if (!o) return;
    g_objs.remove(pmap);
    delete o;
}

/* XVT_IMAGE: real call sites (nodGraph.c, nodwork1.c) use it to capture a
 * rendered XVT_PIXMAP region into a portable image and write it out (e.g.
 * "save diagram as image"). Backed directly by QImage via the same
 * opaque-handle-through-reinterpret_cast pattern as XvtPaletteImpl
 * (struct defined near the top of the file, see XvtPaletteImpl's own
 * comment for why). */
XVT_IMAGE xvt_image_create(int /*type*/, short w, short h, DATA_PTR)
{
    auto *impl = new XvtImageImpl();
    impl->img = QImage(qMax(1, (int)w), qMax(1, (int)h), QImage::Format_RGB32);
    impl->img.fill(Qt::white);
    return reinterpret_cast<XVT_IMAGE>(impl);
}
void xvt_image_destroy(XVT_IMAGE h) { delete reinterpret_cast<XvtImageImpl *>(h); }

XVT_IMAGE xvt_image_read(const char *path)
{
    if (!path) return nullptr;
    QImage img(QString::fromLocal8Bit(path));
    if (img.isNull()) return nullptr;
    auto *impl = new XvtImageImpl();
    impl->img = img;
    return reinterpret_cast<XVT_IMAGE>(impl);
}

void xvt_image_get_dimensions(XVT_IMAGE h, short *w, short *hh)
{
    auto *impl = reinterpret_cast<XvtImageImpl *>(h);
    if (w) *w = impl ? (short)impl->img.width() : 0;
    if (hh) *hh = impl ? (short)impl->img.height() : 0;
}

void xvt_image_set_pixel(XVT_IMAGE h, short x, short y, COLOR c)
{
    auto *impl = reinterpret_cast<XvtImageImpl *>(h);
    if (impl && impl->img.rect().contains(x, y)) impl->img.setPixelColor(x, y, toQColor(c));
}

COLOR xvt_image_get_pixel(XVT_IMAGE h, short x, short y)
{
    auto *impl = reinterpret_cast<XvtImageImpl *>(h);
    if (!impl || !impl->img.rect().contains(x, y)) return COLOR_BLACK;
    QColor c = impl->img.pixelColor(x, y);
    return XVT_MAKE_COLOR(c.red(), c.green(), c.blue());
}

void xvt_image_get_from_pmap(XVT_IMAGE h, XVT_PIXMAP pmap, RCT *srcRect, RCT *dstRect)
{
    auto *impl = reinterpret_cast<XvtImageImpl *>(h);
    XvtObj *src = objFor(pmap);
    if (!impl || !src) return;
    QRect s = srcRect ? toQRect(srcRect) : src->backing.rect();
    QRect d = dstRect ? toQRect(dstRect) : impl->img.rect();
    QPainter p(&impl->img);
    p.drawImage(d, src->backing, s);
}

/* Real XVT palette-reduction/CLUT calls for indexed-color images -- this
 * compat layer only ever creates true-color (Format_RGB32) images, so
 * these are correctly no-ops (nothing to reduce/recolor). */
void xvt_image_set_ncolors(XVT_IMAGE, int) {}
void xvt_image_set_clut(XVT_IMAGE, COLOR *, int) {}

BOOLEAN xvt_image_write_bmp_to_iostr(XVT_IMAGE h, DATA_PTR iostr)
{
    /* [User-requested format change] Despite the name (kept as-is since
     * it matches the real XVT function every caller here uses), this
     * writes PNG now, not BMP -- both "Save Picture (BMP)" and
     * "Movie->Create..." shared this one writer (nodwork1.c's
     * savePixmap), and both were reported producing unreadable/corrupted
     * BMP output. Callers construct their filenames with a matching
     * ".png" extension now (see mainMenu.c / nodwork1.c's createMovie). */
    auto *impl = reinterpret_cast<XvtImageImpl *>(h);
    auto *f = reinterpret_cast<QFile *>(iostr);
    if (!impl || !f) return FALSE;
    return impl->img.save(f, "PNG") ? TRUE : FALSE;
}
BOOLEAN xvt_image_write_macpict_to_iostr(XVT_IMAGE h, DATA_PTR iostr)
{
    /* MacPICT is a legacy Mac-only format with no Qt writer, and this
     * app's own code only ever takes this branch on XVTWS==MACWS builds
     * (see the #ifdef around the two write calls in nodwork1.c) -- not
     * reachable on this Linux/Qt build, where the BMP branch above is
     * what actually runs. Falls back to the same BMP writer just so the
     * symbol exists and does something sane if ever exercised. */
    return xvt_image_write_bmp_to_iostr(h, iostr);
}

/* ==========================================================================
 * Rects / geometry -- pure arithmetic, no Qt needed
 * ========================================================================== */
void xvt_rect_set(RCT *r, short left, short top, short right, short bottom)
{
    if (!r) return;
    r->left = left; r->top = top; r->right = right; r->bottom = bottom;
}

void xvt_rect_set_pos(RCT *r, PNT pos)
{
    if (!r) return;
    short w = r->right - r->left, h = r->bottom - r->top;
    r->left = pos.h; r->top = pos.v;
    r->right = r->left + w; r->bottom = r->top + h;
}

void xvt_rect_offset(RCT *r, short dh, short dv)
{
    if (!r) return;
    r->left += dh; r->right += dh; r->top += dv; r->bottom += dv;
}

void xvt_rect_set_width(RCT *r, short w) { if (r) r->right = r->left + w; }
void xvt_rect_set_height(RCT *r, short h) { if (r) r->bottom = r->top + h; }
short xvt_rect_get_width(const RCT *r) { return r ? (short)(r->right - r->left) : 0; }
short xvt_rect_get_height(const RCT *r) { return r ? (short)(r->bottom - r->top) : 0; }

BOOLEAN xvt_rect_has_point(const RCT *r, PNT pt)
{
    if (!r) return FALSE;
    return (pt.h >= r->left && pt.h < r->right && pt.v >= r->top && pt.v < r->bottom) ? TRUE : FALSE;
}

BOOLEAN xvt_rect_intersect(RCT *dst, const RCT *a, const RCT *b)
{
    if (!dst || !a || !b) return FALSE;
    QRect qa(a->left, a->top, a->right - a->left, a->bottom - a->top);
    QRect qb(b->left, b->top, b->right - b->left, b->bottom - b->top);
    QRect qi = qa.intersected(qb);
    fromQRect(dst, qi);
    return qi.isValid() ? TRUE : FALSE;
}

/* ==========================================================================
 * Memory -- plain heap, matching XVT's own documented behavior of being a
 * thin portable wrapper over malloc/free (no calculation logic here).
 * ========================================================================== */
DATA_PTR xvt_mem_alloc(size_t size) { return malloc(size); }
DATA_PTR xvt_mem_zalloc(size_t size) { return calloc(1, size); }
DATA_PTR xvt_mem_realloc(DATA_PTR p, size_t size)
{
    /* realloc(NULL, size) == malloc(size): uninitialized. Some app code
     * (e.g. updateWindowsMenu in nodLib1.c) grows a tree from a null
     * pointer and only memsets part of the new region, expecting the
     * rest to already hold resource-file-provided static data qt_compat
     * has no equivalent of. Zero-filling this specific well-defined case
     * (growing from NULL) turns "uninitialized garbage" into a
     * deterministic, harmless all-zero block instead. */
    if (!p) return calloc(1, size);
    return realloc(p, size);
}
void xvt_mem_free(DATA_PTR p) { free(p); }
DATA_PTR xvt_mem_halloc(long count, size_t elemsize) { return calloc((size_t)count, elemsize); }
DATA_PTR xvt_mem_rep(DATA_PTR p, size_t, size_t newsize) { return realloc(p, newsize); }

/* ==========================================================================
 * Singly-linked lists (XVT SLIST) -- thin QList-backed stub implementation
 * ========================================================================== */
/* text is an owned buffer (kept alive for the element's lifetime) so
 * xvt_slist_get can hand back a real char* the app can strlen/strcpy from
 * immediately, matching real XVT's documented SLIST semantics (each
 * element pairs an optional string with a long). */
struct s_xvt_slist_elt { long data; QByteArray text; };
struct s_xvt_slist { QList<SLIST_ELT> items; int iter = -1; };
SLIST xvt_slist_create(void) { return new s_xvt_slist(); }
void xvt_slist_destroy(SLIST l)
{
    if (!l) return;
    for (SLIST_ELT e : l->items) delete e;
    delete l;
}
SLIST_ELT xvt_slist_get_first(SLIST l)
{
    if (!l || l->items.isEmpty()) return nullptr;
    l->iter = 0;
    return l->items[0];
}
SLIST_ELT xvt_slist_get_next(SLIST l, SLIST_ELT)
{
    if (!l) return nullptr;
    l->iter++;
    return (l->iter >= 0 && l->iter < l->items.size()) ? l->items[l->iter] : nullptr;
}
char *xvt_slist_get(SLIST, SLIST_ELT elt, long *data)
{
    if (data) *data = elt ? elt->data : 0;
    return elt ? elt->text.data() : nullptr;
}
void xvt_slist_add_sorted(SLIST l, const char *text, long data, BOOLEAN, BOOLEAN)
{
    if (!l) return;
    SLIST_ELT e = new s_xvt_slist_elt();
    e->data = data;
    if (text) e->text = QByteArray(text);
    l->items.append(e);
}
long xvt_slist_count(SLIST l) { return l ? l->items.size() : 0; }

/* See xvt_list_add's index==-1 case: real XVT bulk-populates a list
 * control from a pre-sorted SLIST in one call instead of one xvt_list_add
 * per item (nodGraph.c's makeColorList is the real call site -- adds all
 * predefined + custom colour names this way). */
static void bulkAddFromSlist(QWidget *w, SLIST l)
{
    if (!l) return;
    if (auto *lw = qobject_cast<QListWidget *>(w)) {
        bool old = lw->blockSignals(true);
        for (SLIST_ELT e : l->items)
            lw->addItem(new QListWidgetItem(QString::fromLocal8Bit(e->text)));
        lw->blockSignals(old);
    } else if (auto *combo = qobject_cast<QComboBox *>(w)) {
        bool old = combo->blockSignals(true);
        for (SLIST_ELT e : l->items)
            combo->addItem(QString::fromLocal8Bit(e->text));
        combo->blockSignals(old);
    }
}

static void slistAppendLong(SLIST l, long v, const QByteArray &text = QByteArray())
{
    SLIST_ELT e = new s_xvt_slist_elt();
    e->data = v;
    e->text = text;
    l->items.append(e);
}

/* ==========================================================================
 * File system
 * ========================================================================== */
BOOLEAN xvt_fsys_get_default_dir(DIRECTORY *dir)
{
    if (!dir) return FALSE;
    strncpy(dir->path, QDir::currentPath().toLocal8Bit().constData(), sizeof(dir->path) - 1);
    return TRUE;
}

BOOLEAN xvt_fsys_set_dir(DIRECTORY *dir)
{
    if (!dir) return FALSE;
    return QDir::setCurrent(QString::fromLocal8Bit(dir->path)) ? TRUE : FALSE;
}

BOOLEAN xvt_fsys_get_dir(DIRECTORY *dir) { return xvt_fsys_get_default_dir(dir); }
BOOLEAN xvt_fsys_set_dir_startup(void)
{
    /* Real XVT sets the current directory to wherever the application's
     * executable/resources live, so relative loads (default setup file,
     * rock property database, icons, etc.) resolve regardless of the
     * directory the app was launched from. */
    return QDir::setCurrent(QCoreApplication::applicationDirPath()) ? TRUE : FALSE;
}

BOOLEAN xvt_fsys_convert_str_to_dir(const char *str, DIRECTORY *dir)
{
    if (!dir || !str) return FALSE;
    strncpy(dir->path, str, sizeof(dir->path) - 1);
    dir->path[sizeof(dir->path) - 1] = '\0';
    return TRUE;
}

BOOLEAN xvt_fsys_convert_dir_to_str(DIRECTORY *dir, char *buf, int maxlen)
{
    if (!dir || !buf) return FALSE;
    strncpy(buf, dir->path, maxlen - 1);
    buf[maxlen - 1] = '\0';
    return TRUE;
}

BOOLEAN xvt_fsys_get_file_attr(FILE_SPEC *fs, unsigned long attr)
{
    if (!fs) return FALSE;
    QFileInfo fi(QDir(QString::fromLocal8Bit(fs->dir.path)), QString::fromLocal8Bit(fs->name));
    if (attr == XVT_FILE_ATTR_READ) return fi.isReadable() ? TRUE : FALSE;
    if (attr == XVT_FILE_ATTR_DIRECTORY) return fi.isDir() ? TRUE : FALSE;
    return FALSE;
}

/* Correct no-op, not a missed port: every real call site passes
 * XVT_FILE_ATTR_TYPESTR/_CREATORSTR with a 4-char Mac Finder type/creator
 * code (e.g. "TEXT"/"Nody", passed as `(long)"TEXT"` -- a string pointer
 * disguised as a long, hence this function's 3rd param really needs to
 * be `long`, not `BOOLEAN`, to avoid truncating that pointer on LP64).
 * Mac Finder type/creator metadata has no Linux/Windows filesystem
 * equivalent, so there is nothing to actually do here -- matches this
 * file's existing xvt_vobj_set_palet "no-op is correct" precedent. */
BOOLEAN xvt_fsys_set_file_attr(FILE_SPEC *, unsigned long, long) { return TRUE; }
BOOLEAN xvt_fsys_rem_file(FILE_SPEC *fs)
{
    if (!fs) return FALSE;
    QFileInfo fi(QDir(QString::fromLocal8Bit(fs->dir.path)), QString::fromLocal8Bit(fs->name));
    return QFile::remove(fi.absoluteFilePath()) ? TRUE : FALSE;
}

/* ==========================================================================
 * Resources / icons / palettes / clipboard / text-edit / timers / help /
 * printing / iostreams -- all [STUB] for this pass (see xvt.h coverage
 * table). None of these are reachable from the batch/CLI calculation path.
 * ========================================================================== */
void xvt_xres_build_icon(ICON_RESOURCE *, unsigned, unsigned, DATA_PTR) { XVT_STUB(); }
void xvt_res_free_menu_tree(DATA_PTR)
{
    /* Deliberate no-op, not a missed port: real XVT's xvt_menu_get_tree
     * hands back a copy loaded from the resource file, so the app frees
     * that copy after xvt_menu_set_tree() has taken its own copy. Here,
     * xvt_menu_get_tree/xvt_menu_set_tree operate on the SAME live,
     * persistent tree (see XvtObj::menuTree) rather than making a second
     * copy -- so this pointer is still in active use and must not
     * actually be freed. */
}
void xvt_res_free_win_def(WIN_DEF *def) { free(def); }

/* [STUB-placeholder] The real layout data (control positions/sizes/types
 * for each of this app's ~150 dialogs) lives in a compiled XVT resource
 * file that isn't in this repo -- see qt_compat/README.md. Real control
 * *types* (though not positions/sizes) are recovered from nodInc.h via
 * qt_compat/dialog_registry.cpp; this returns a minimal window-only WIN_DEF
 * stamped with resId, which xvt_win_create_def/xvt_dlg_create_def use to
 * pull that dialog's real control list out of the registry. */
static WIN_DEF *placeholderWinDef(long resId)
{
    WIN_DEF *def = (WIN_DEF *)calloc(2, sizeof(WIN_DEF));
    def[0].wtype = W_DOC;
    def[0].resId = resId;
    /* Callers (createEventWindow/createPositionedWindow/createCenteredWindow
     * in builder.c) all compute their OWN winWidth/winHeight as (rct.right -
     * rct.left)/(rct.bottom - rct.top) from what this function returns, then
     * build their real final rect at that SAME size (just repositioned/
     * centered) -- they never consult dialog_positions.cpp themselves. A
     * flat hardcoded 400x300 here silently overrode every reconstructed
     * dialog's real size (e.g. EVENT_WINDOW's g_pos_137 entry is 500x335)
     * AND, more damagingly, every NESTED sub-panel's size too (e.g.
     * createPositionedWindow(STRATIGRAPHY_WINDOW, ...) also went through
     * this same 400x300 default instead of g_pos_169's real 270x262) --
     * an oversized sub-panel offset inside an also-undersized parent
     * clipped virtually all of its content outside the visible parent
     * bounds, matching a user report of an entirely blank Stratigraphy
     * dialog. Consult the real reconstructed size when we have one. */
    const DialogPositionEntry *pos = findPositionEntry(resId);
    short w = (pos && pos->width > 0) ? pos->width : 400;
    short h = (pos && pos->height > 0) ? pos->height : 300;
    xvt_rect_set(&def[0].rct, 100, 100, 100 + w, 100 + h);
    def[1].wtype = W_NONE;
    return def;
}
WIN_DEF *xvt_res_get_win_def(long resId) { return placeholderWinDef(resId); }
WIN_DEF *xvt_res_get_dlg_def(long resId) { return placeholderWinDef(resId); }

/* Recursively materializes a real MENU_ITEM tree from g_menuTree (the
 * same real menu data buildMenuBar() uses to construct the actual visible
 * QMenuBar, see menu_registry.cpp) -- nodLib1.c's updateWindowsMenu calls
 * xvt_res_get_menu(TASK_MENUBAR) to clone the real main menu structure
 * into a per-window "line map" menu variant it then edits. text/child
 * arrays are freshly allocated on every call (matching real XVT's
 * contract of returning a resource-loaded copy the caller owns) --
 * xvt_res_free_menu_tree is a documented no-op elsewhere in this file, so
 * these leak a small, bounded amount of memory per call; acceptable
 * since this is only called when line-map windows are created, not a
 * hot path. */
static MENU_ITEM *buildMenuItemArray(const char *parentPath, int *outCount)
{
    int count = 0;
    for (int i = 0; i < g_menuTreeCount; i++) {
        const MenuNodeEntry &n = g_menuTree[i];
        bool isRoot = parentPath ? (n.parentPath && strcmp(n.parentPath, parentPath) == 0)
                                  : (n.parentPath == nullptr);
        if (isRoot) count++;
    }
    if (outCount) *outCount = count;
    if (count == 0) return nullptr;
    /* Allocate one extra slot beyond `count`, left zeroed by calloc --
     * real XVT's MENU_ITEM arrays are terminated by a tag==0 sentinel
     * entry (confirmed from the real SDK header's s_mitem docs), which
     * xvt_menu_set_tree's real-QMenuBar builder below relies on to know
     * where a flat sibling run ends. */
    auto *arr = (MENU_ITEM *)calloc((size_t)count + 1, sizeof(MENU_ITEM));
    int idx = 0;
    for (int i = 0; i < g_menuTreeCount; i++) {
        const MenuNodeEntry &n = g_menuTree[i];
        bool isRoot = parentPath ? (n.parentPath && strcmp(n.parentPath, parentPath) == 0)
                                  : (n.parentPath == nullptr);
        if (!isRoot) continue;
        arr[idx].tag = n.tag;
        arr[idx].text = strdup(n.label);
        arr[idx].enabled = TRUE;
        arr[idx].checkable = FALSE;
        arr[idx].checked = FALSE;
        arr[idx].mkey = 0;
        arr[idx].child = n.isLeaf ? nullptr : buildMenuItemArray(n.path, nullptr);
        idx++;
    }
    return arr;
}

DATA_PTR xvt_res_get_menu(long resId)
{
    /* TASK_MENUBAR == 1000 (nodInc.h) is the only menu resource this app
     * ever had real reconstructed data for (see menu_registry.cpp's
     * provenance -- the real noddy.rc "100 MENU" block). */
    if (resId == 1000) return (DATA_PTR)buildMenuItemArray(nullptr, nullptr);
    if (resId == 1025) {
        /* MENU_BAR_2 -- the Section/Line-Map window's own small menu
         * resource. No real structure recovered for this one (not in any
         * source found so far), but returning nullptr here was the actual
         * bug behind Section/Map windows never getting a menu bar at all:
         * nodLib1.c's createLineMapMenubar does
         * `if (!(lineMapMenubar = xvt_res_get_menu(MENU_BAR_2))) return;`
         * -- an early return that skipped its xvt_menu_set_tree() call
         * entirely (confirmed by reading that function). On Windows
         * (nodLib1.c ~line 1719) it then merges the real TASK_MENUBAR's
         * first NUM_MENUS(6) items with lineMapMenubar[0..3] into one
         * combined bar, so as long as this hands back >= 4 valid slots,
         * the window ends up with the REAL main app menu plus these 4 --
         * items [1] and [2] get their .child replaced with dynamically-
         * built event-navigation submenus by that same function (their
         * placeholder child here is discarded, xvt_res_free_menu_tree is
         * a no-op so that's safe). Labels are a reasonable placeholder,
         * not recovered from any original source. */
        auto *arr = (MENU_ITEM *)calloc(5, sizeof(MENU_ITEM));
        const char *labels[4] = { "Window", "Events", "Groups", "Display" };
        for (int i = 0; i < 4; i++) {
            arr[i].tag = 2100 + i;
            arr[i].text = strdup(labels[i]);
            arr[i].enabled = TRUE;
        }
        return (DATA_PTR)arr;
    }
    return nullptr;
}

XVT_PALETTE xvt_palet_create(int, DATA_PTR)
{
    return reinterpret_cast<XVT_PALETTE>(new XvtPaletteImpl());
}
void xvt_palet_destroy(XVT_PALETTE p)
{
    delete reinterpret_cast<XvtPaletteImpl *>(p);
}
void xvt_palet_set_tolerance(XVT_PALETTE p, long tol)
{
    if (p) reinterpret_cast<XvtPaletteImpl *>(p)->tolerance = tol;
}
void xvt_palet_add_colors(XVT_PALETTE p, COLOR *colors, short n)
{
    if (!p || !colors) return;
    auto *impl = reinterpret_cast<XvtPaletteImpl *>(p);
    for (short i = 0; i < n; i++) impl->colors.append(colors[i]);
}
short xvt_palet_get_colors(XVT_PALETTE p, COLOR *colorsp, short maxcolors)
{
    /* Real signature confirmed against the actual XVT SDK header
     * (xvtpalet.h): fills the caller's buffer, returns the count copied
     * -- NOT "return a pointer to internal storage" as this was
     * originally (wrongly) stubbed. Also matches the one real call site
     * (nodwork1.c:2665, `xvt_palet_get_colors(palette, colors, 256)`). */
    if (!p || !colorsp) return 0;
    auto *impl = reinterpret_cast<XvtPaletteImpl *>(p);
    short n = (short)qMin((int)maxcolors, impl->colors.size());
    for (short i = 0; i < n; i++) colorsp[i] = impl->colors[i];
    return n;
}

/* Real, working in-process clipboard (single slot -- the only caller,
 * nodLib1.c's putObjectsInClipboard/getObjectsFromClipboard, only ever uses
 * one CB_APPL/"Nod" format at a time, so no per-format table is needed).
 * Previously fully stubbed: xvt_cb_open() unconditionally returned FALSE,
 * which nodLib1.c surfaces straight to the user as "Error opening
 * clipboard." on every Cut/Copy/Paste -- and xvt_cb_put_data() also always
 * returned FALSE regardless of what it was given, so even if open()
 * hadn't already bailed out, Copy would still report "Error putting data
 * onto clipboard." Real usage pattern: xvt_cb_alloc_data(size) hands back
 * a scratch buffer, the caller memcpy's the real payload directly into
 * that pointer, then xvt_cb_put_data(format, name, size, picture) commits
 * whatever was written into the most recent alloc_data buffer -- note
 * `picture` is NOT the payload (the one real call site always passes
 * (PICTURE)NULL there; that parameter is for a separate picture-format
 * clipboard payload this app never uses) -- and xvt_cb_free_data()
 * releases the scratch buffer once the commit is done. */
/* PICTURE is a `long` opaque-handle type (xvt_types.h), not a real
 * pointer -- nodLib1.c only gets away with assigning it straight to a
 * `char far *` because that file is plain C, which permits the implicit
 * int<->pointer conversion; this file is C++ and needs explicit
 * reinterpret_casts between the two. */
static void *g_clipboardData = nullptr;
static long g_clipboardSize = 0;
static void *g_cbPendingAlloc = nullptr;

BOOLEAN xvt_cb_open(BOOLEAN) { return TRUE; }
BOOLEAN xvt_cb_close(void) { return TRUE; }

DATA_PTR xvt_cb_alloc_data(size_t size)
{
    free(g_cbPendingAlloc);
    g_cbPendingAlloc = malloc(size);
    return g_cbPendingAlloc;
}

void xvt_cb_free_data(void)
{
    free(g_cbPendingAlloc);
    g_cbPendingAlloc = nullptr;
}

BOOLEAN xvt_cb_put_data(long, const char *, long size, PICTURE)
{
    if (!g_cbPendingAlloc || size <= 0) return FALSE;
    void *copy = malloc((size_t)size);
    if (!copy) return FALSE;
    memcpy(copy, g_cbPendingAlloc, (size_t)size);
    free(g_clipboardData);
    g_clipboardData = copy;
    g_clipboardSize = size;
    return TRUE;
}

PICTURE xvt_cb_get_data(long, const char *, long *size)
{
    if (size) *size = g_clipboardSize;
    return reinterpret_cast<PICTURE>(g_clipboardData);
}

/* Correctly a no-op here, not a missed port: nearly every *win.c dialog
 * handler calls this from its default/fallthrough case to let XVT's
 * built-in TXEDIT text-control widget handle whatever event it didn't
 * explicitly claim (cursor movement, selection, etc). This port doesn't
 * use TXEDIT-style controls anywhere -- text entry is real QLineEdit,
 * which already handles all of that natively -- so there's nothing for
 * this to do. */
long xvt_tx_process_event(WINDOW, EVENT *) { return 0; }
void xvt_tx_suspend(WINDOW win) { if (auto *w = qobject_cast<QPlainTextEdit *>(widgetFor(win))) w->setUpdatesEnabled(false); }
void xvt_tx_resume(WINDOW win) { if (auto *w = qobject_cast<QPlainTextEdit *>(widgetFor(win))) { w->setUpdatesEnabled(true); w->update(); } }
void xvt_tx_clear(WINDOW win) { if (auto *w = qobject_cast<QPlainTextEdit *>(widgetFor(win))) w->clear(); }
void xvt_tx_add_par(TXEDIT tx, T_PNUM, const char *text)
{
    if (auto *w = qobject_cast<QPlainTextEdit *>(widgetFor((WINDOW)tx)))
        w->appendPlainText(text ? QString::fromLocal8Bit(text) : QString());
}

DATA_PTR xvt_timer_create(WINDOW, unsigned long) { XVT_STUB(); return nullptr; }
void xvt_timer_destroy(DATA_PTR) { XVT_STUB(); }

XVT_HELP_INFO xvt_help_open_helpfile(FILE_SPEC *, long) { XVT_STUB(); return nullptr; }
void xvt_help_close_helpfile(XVT_HELP_INFO) { XVT_STUB(); }
BOOLEAN xvt_help_process_event(XVT_HELP_INFO, WINDOW, EVENT *) { XVT_STUB(); return FALSE; }
void xvt_help_set_win_assoc(XVT_HELP_INFO, WINDOW, long, long) { XVT_STUB(); }
void xvt_help_set_menu_assoc(XVT_HELP_INFO, WINDOW, long, long, long) { XVT_STUB(); }

/* Real printing, backed by QPrinter/QPrintDialog -- replaces real XVT's
 * banded-printing model (memory-constrained 1994-era chunked page
 * rendering) with a single QPainter pass, since Qt/the OS print
 * subsystem already handles spooling. This app has exactly one print
 * flow (nodLib1.c's printWindow -> xvt_print_start_thread(printPicture,
 * pixmap) -> printPicture), so one global in-flight print job is
 * sufficient -- matches xvt_print_get_next_band()'s real signature
 * (takes no window/job argument at all, implying real XVT also assumed a
 * single ambient print job).
 *
 * PRINT_RCD is declared fully opaque in xvt_types.h (`{int placeholder;}`,
 * never touched by app code except via these functions), so it's safe to
 * give it a completely different real layout here. */
struct XvtPrintState {
    QPrinter *printer = nullptr;
    QPainter *painter = nullptr;
    bool bandReturned = false;
    RCT bandRect{};
    WINDOW printWin = NULL_WIN;  /* the handle xvt_print_create_win returned */
};
/* xvt_print_get_next_band() genuinely takes no argument at all in real
 * XVT (confirmed by its one real call site, nodLib1.c's printPicture,
 * which calls it bare) -- ambient single-job state is a deliberate match
 * to that, not a shortcut; this app only ever has one print job in
 * flight at a time. */
static XvtPrintState *g_activePrint = nullptr;

PRINT_RCD *xvt_print_create(long *size)
{
    auto *st = new XvtPrintState();
    /* ScreenResolution, not HighResolution: nodLib1.c's printPicture draws
     * the picture using its screen-pixel dimensions directly (dstFrame is
     * scrFrame scaled only by projectOptions.printScalingFactor, no DPI
     * awareness at all) -- at HighResolution's typical 1000+ DPI, a few
     * hundred screen-pixel-sized units painted a tiny speck in the corner
     * of an otherwise blank page (matches a user screenshot). Screen
     * resolution (~96-100 DPI) keeps the same 1:1-ish pixel scale the app
     * already assumes. */
    st->printer = new QPrinter(QPrinter::ScreenResolution);
    if (size) *size = (long)sizeof(XvtPrintState);
    return reinterpret_cast<PRINT_RCD *>(st);
}

BOOLEAN xvt_dm_post_page_setup(PRINT_RCD *printer_setup)
{
    auto *st = reinterpret_cast<XvtPrintState *>(printer_setup);
    if (!st || !st->printer) return FALSE;
    QPageSetupDialog dlg(st->printer);
    return dlg.exec() == QDialog::Accepted ? TRUE : FALSE;
}

WINDOW xvt_print_create_win(PRINT_RCD *printer_setup, const char *title)
{
    auto *st = reinterpret_cast<XvtPrintState *>(printer_setup);
    if (!st || !st->printer) return NULL_WIN;
    QPrintDialog dlg(st->printer);
    if (title) dlg.setWindowTitle(QString::fromLocal8Bit(title));
    if (dlg.exec() != QDialog::Accepted) return NULL_WIN;
    g_activePrint = st;
    XvtObj *o = new XvtObj();
    o->type = W_DOC;
    WINDOW h = allocHandle(o);
    st->printWin = h;
    return h;
}

/* Real XVT's xvt_print_open_page/xvt_print_close_page take the PRINT_RCD*
 * (confirmed by their one real call site, nodLib1.c's printPicture, which
 * passes `printerSetup` -- the PRINT_RCD* from xvt_print_create -- to
 * both, NOT the WINDOW returned by xvt_print_create_win). This header
 * previously declared both as taking a WINDOW; since PRINT_RCD* and
 * WINDOW are different-sized/different-kind values, the app's actual
 * pointer argument got silently reinterpreted as a bogus WINDOW handle,
 * so objFor() on it always found nothing -- printPainter never actually
 * got attached to the real print window object, so every draw call
 * during printing fell through to painting into a nonexistent backing
 * image instead of the real QPrinter, producing a blank page/PDF. */
BOOLEAN xvt_print_open_page(PRINT_RCD *printer_setup)
{
    auto *st = reinterpret_cast<XvtPrintState *>(printer_setup);
    if (!st || !st->printer) return FALSE;
    st->painter = new QPainter(st->printer);
    st->bandReturned = false;
    QRect pageRect = st->printer->pageLayout().paintRectPixels(st->printer->resolution());
    st->bandRect.left = 0;
    st->bandRect.top = 0;
    st->bandRect.right = (short)pageRect.width();
    st->bandRect.bottom = (short)pageRect.height();
    if (XvtObj *o = objFor(st->printWin)) o->printPainter = st->painter;
    return TRUE;
}

RCT *xvt_print_get_next_band(void)
{
    if (!g_activePrint || g_activePrint->bandReturned) return nullptr;
    g_activePrint->bandReturned = true;
    return &g_activePrint->bandRect;
}

void xvt_print_close_page(PRINT_RCD *printer_setup)
{
    auto *st = reinterpret_cast<XvtPrintState *>(printer_setup);
    if (!st) return;
    if (XvtObj *o = objFor(st->printWin)) o->printPainter = nullptr;
    if (st->painter) {
        st->painter->end();
        delete st->painter;
        st->painter = nullptr;
    }
}

void xvt_print_start_thread(DATA_PTR draw_fn, long user_data)
{
    /* Real XVT runs this on a background thread; this app's one call
     * site just needs the callback invoked, and Qt's print dialogs
     * already pump their own event loop synchronously, so there's no
     * need for real threading here. */
    if (!draw_fn) return;
    auto fn = reinterpret_cast<BOOLEAN (*)(long)>(draw_fn);
    fn(user_data);
}

/* Real XVT's IOSTR is a generic seekable byte-stream abstraction; the
 * only real consumer in this app (xvt_image_write_bmp_to_iostr, see
 * below) just wants somewhere to hand QImage::save() a QIODevice. The
 * real call site (nodwork1.c's savePixmap, shared by both "Save Picture"
 * and "Movie->Create...") already does its own fopen(filename, "wb+") and
 * passes that live FILE* straight in -- this header used to declare the
 * parameter as `FILE_SPEC *fs` instead of `FILE *fp`, a mismatched
 * pointer type that plain C only warns about instead of rejecting. At
 * runtime this reinterpreted the FILE*'s raw bytes as a FILE_SPEC struct
 * and derived a garbage path from whatever those bytes happened to be,
 * so the image got written to that bogus path (the "corrupted name" file
 * from a user report) while the real, correctly-named file opened by the
 * app's own fopen() was left at 0 bytes (never actually written through,
 * only truncated by "wb+") -- exactly the reported "two files, one empty
 * one corrupted" symptom. Fixed by taking the real FILE*, wrapping its
 * underlying fd in a QFile Qt can write through -- QFile::open(fd, ...)
 * does NOT take ownership of the descriptor, matching this app's existing
 * "xvt_iostr_destroy() then fclose(fo) itself" cleanup order. */
DATA_PTR xvt_iostr_create_fwrite(FILE *fp)
{
    if (!fp) return nullptr;
    auto *f = new QFile();
    if (!f->open(fileno(fp), QIODevice::WriteOnly)) { delete f; return nullptr; }
    return reinterpret_cast<DATA_PTR>(f);
}
void xvt_iostr_destroy(DATA_PTR p) { delete reinterpret_cast<QFile *>(p); }

/* ==========================================================================
 * Misc / screen
 * ========================================================================== */
void xvt_scr_set_focus_vobj(WINDOW win)
{
    QWidget *w = widgetFor(win);
    if (w) w->setFocus();
}

WINDOW xvt_scr_get_focus_vobj(void)
{
    QWidget *w = QApplication::focusWidget();
    return w ? (WINDOW)w->property("xvtHandle").toLongLong() : NULL_WIN;
}

SLIST xvt_scr_list_wins(void)
{
    /* g_objs holds a handle for EVERY object this port hands out -- top-
     * level windows AND every individual control inside them alike (see
     * createControlWidget's allocHandle call). Real XVT's
     * xvt_scr_list_wins() enumerates actual top-level windows only.
     * Returning every control too meant blklayop.c's E_CREATE handler
     * (LAYER_DISPLAY_WINDOW, a W_DBL popup) -- which walks this list to
     * simulate modality by disabling "every window but this one" -- was
     * disabling every control in the entire application, including the
     * popup's own contents (matches a user report of an entire popup
     * dialog appearing fully disabled). Only top-level windows are ever
     * actually XvtWindow* with ctlId == -1 (controls always have a real,
     * non-negative ctlId); same convention already relied on elsewhere
     * (e.g. xvt_menu_update). */
    /* Also exclude TASK_WIN itself: real XVT's task/frame window is never
     * one of "the windows" this enumerates (it's the shell the document
     * windows live inside, not a document window) -- nodLib1.c's
     * tileWindows/cascadeWindows walk this list and explicitly skip a
     * handful of special windows by name (History, floating toolbar, job
     * status, status bar) but never TASK_WIN, meaning real XVT never
     * handed it back here in the first place. Including it meant Cascade/
     * Tile occasionally repositioned/resized the actual main application
     * window itself along with the MDI child windows -- user-reported as
     * "rearranges main window not windows within program area". */
    SLIST l = xvt_slist_create();
    for (auto it = g_objs.begin(); it != g_objs.end(); ++it)
        if (it.value()->widget && it.value()->ctlId == -1 && it.key() != TASK_WIN)
            slistAppendLong(l, it.key(), it.value()->widget->windowTitle().toLocal8Bit());
    return l;
}

void xvt_str_copy(char *dst, const char *src) { if (dst && src) strcpy(dst, src); }

void xvt_errmsg_sig_if(BOOLEAN cond, WINDOW, int severity, long, const char *arg1, long, const char *msg)
{
    if (!cond) return;
    qWarning("XVT fatal assert (sev=%d, arg=%s): %s", severity, arg1 ? arg1 : "", msg ? msg : "");
    if (severity >= SEV_FATAL) abort();
}
