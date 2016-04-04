/**
*  \file  ggdkdraw.c
*  \brief GDK drawing backend.
*  \author Jeremy Tan
*  \license MIT
*/

#include "ggdkdrawP.h"
#include "gresource.h"
#include <assert.h>

// Private member functions (file-level)

static GGC *_GGDKDrawNewGGC() {
    GGC *ggc = calloc(1,sizeof(GGC));
    if (ggc == NULL) {
        fprintf(stderr, "GGC: Memory allocation failed!\n");
        return NULL;
    }

    ggc->clip.width = ggc->clip.height = 0x7fff;
    ggc->fg = 0;
    ggc->bg = 0xffffff;
    return ggc;
}

static void GGDKDrawInit(GDisplay *gdisp) {

}

//static void GGDKDrawTerm(GDisplay *gdisp){}
//static void* GGDKDrawNativeDisplay(GDisplay *gdisp){}

static void GGDKDrawSetDefaultIcon(GWindow icon) {
    // Closest is: gdk_window_set_icon_list
    // Store an icon list???
}

static GWindow GGDKDrawCreateTopWindow(GDisplay *gdisp, GRect *pos, int (*eh)(GWindow gw, GEvent *), void *user_data, GWindowAttrs *gattrs){
    fprintf(stderr, "GDKCALL: GGDKDrawCreateTopWindow\n"); assert(false);
}

static GWindow GGDKDrawCreateSubWindow(GWindow gw, GRect *pos, int (*eh)(GWindow gw, GEvent *), void *user_data, GWindowAttrs *gattrs){
    fprintf(stderr, "GDKCALL: GGDKDrawCreateSubWindow\n"); assert(false);
}

static GWindow GGDKDrawCreatePixmap(GDisplay *gdisp, uint16 width, uint16 height){
    fprintf(stderr, "GDKCALL: GGDKDrawCreatePixmap\n"); assert(false);
}

static GWindow GGDKDrawCreateBitmap(GDisplay *gdisp, uint16 width, uint16 height, uint8 *data){
    fprintf(stderr, "GDKCALL: GGDKDrawCreateBitmap\n"); assert(false);
}

static GCursor GGDKDrawCreateCursor(GWindow src, GWindow mask, Color fg, Color bg, int16 x, int16 y){
    fprintf(stderr, "GDKCALL: GGDKDrawCreateCursor\n"); assert(false);
}

static void GGDKDrawDestroyWindow(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawDestroyWindow\n"); assert(false);
}

static void GGDKDrawDestroyCursor(GDisplay *gdisp, GCursor gcursor){
    fprintf(stderr, "GDKCALL: GGDKDrawDestroyCursor\n"); assert(false);
}

static int GGDKDrawNativeWindowExists(GDisplay *gdisp, void *native_window){
    fprintf(stderr, "GDKCALL: GGDKDrawNativeWindowExists\n"); assert(false);
}

static void GGDKDrawSetZoom(GWindow gw, GRect *size, enum gzoom_flags flags){
    fprintf(stderr, "GDKCALL: GGDKDrawSetZoom\n"); assert(false);
}

static void GGDKDrawSetWindowBorder(GWindow gw, int width, Color gcol){
    fprintf(stderr, "GDKCALL: GGDKDrawSetWindowBorder\n"); assert(false);
}

static void GGDKDrawSetWindowBackground(GWindow gw, Color gcol){
    fprintf(stderr, "GDKCALL: GGDKDrawSetWindowBackground\n"); assert(false);
}

static int GGDKDrawSetDither(GDisplay *gdisp, int set){
    fprintf(stderr, "GDKCALL: GGDKDrawSetDither\n"); assert(false);
}


static void GGDKDrawReparentWindow(GWindow gw1, GWindow gw2, int x, int y){
    fprintf(stderr, "GDKCALL: GGDKDrawReparentWindow\n"); assert(false);
}

static void GGDKDrawSetVisible(GWindow gw, int set){
    fprintf(stderr, "GDKCALL: GGDKDrawSetVisible\n"); assert(false);
}

static void GGDKDrawMove(GWindow gw, int32 x, int32 y){
    fprintf(stderr, "GDKCALL: GGDKDrawMove\n"); assert(false);
}

static void GGDKDrawTrueMove(GWindow gw, int32 x, int32 y){
    fprintf(stderr, "GDKCALL: GGDKDrawTrueMove\n"); assert(false);
}

static void GGDKDrawResize(GWindow gw, int32 w, int32 h){
    fprintf(stderr, "GDKCALL: GGDKDrawResize\n"); assert(false);
}

static void GGDKDrawMoveResize(GWindow gw, int32 x, int32 y, int32 w, int32 h){
    fprintf(stderr, "GDKCALL: GGDKDrawMoveResize\n"); assert(false);
}

static void GGDKDrawRaise(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawRaise\n"); assert(false);
}

static void GGDKDrawRaiseAbove(GWindow gw1, GWindow gw2){
    fprintf(stderr, "GDKCALL: GGDKDrawRaiseAbove\n"); assert(false);
}

static int GGDKDrawIsAbove(GWindow gw1, GWindow gw2){
    fprintf(stderr, "GDKCALL: GGDKDrawIsAbove\n"); assert(false);
}

static void GGDKDrawLower(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawLower\n"); assert(false);
}

static void GGDKDrawSetWindowTitles(GWindow gw, const unichar_t *title, const unichar_t *icontitle){
    fprintf(stderr, "GDKCALL: GGDKDrawSetWindowTitles\n"); assert(false);
}

static void GGDKDrawSetWindowTitles8(GWindow gw, const char *title, const char *icontitle){
    fprintf(stderr, "GDKCALL: GGDKDrawSetWindowTitles8\n"); assert(false);
}

static unichar_t* GGDKDrawGetWindowTitle(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawGetWindowTitle\n"); assert(false);
}

static char* GGDKDrawGetWindowTitle8(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawGetWindowTitle8\n"); assert(false);
}

static void GGDKDrawSetTransientFor(GWindow gw1, GWindow gw2){
    fprintf(stderr, "GDKCALL: GGDKDrawSetTransientFor\n"); assert(false);
}

static void GGDKDrawGetPointerPosition(GWindow gw, GEvent *gevent){
    fprintf(stderr, "GDKCALL: GGDKDrawGetPointerPos\n"); assert(false);
}

static GWindow GGDKDrawGetPointerWindow(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawGetPointerWindow\n"); assert(false);
}

static void GGDKDrawSetCursor(GWindow gw, GCursor gcursor){
    fprintf(stderr, "GDKCALL: GGDKDrawSetCursor\n"); assert(false);
}

static GCursor GGDKDrawGetCursor(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawGetCursor\n"); assert(false);
}

static GWindow GGDKDrawGetRedirectWindow(GDisplay *gdisp){
    fprintf(stderr, "GDKCALL: GGDKDrawGetRedirectWindow\n"); assert(false);
}

static void GGDKDrawTranslateCoordinates(GWindow from, GWindow to, GPoint *pt){
    fprintf(stderr, "GDKCALL: GGDKDrawTranslateCoordinates\n"); assert(false);
}


static void GGDKDrawBeep(GDisplay *gdisp){
    fprintf(stderr, "GDKCALL: GGDKDrawBeep\n"); assert(false);
}

static void GGDKDrawFlush(GDisplay *gdisp){
    fprintf(stderr, "GDKCALL: GGDKDrawFlush\n"); assert(false);
}


static void GGDKDrawPushClip(GWindow gw, GRect *rct, GRect *old){
    fprintf(stderr, "GDKCALL: GGDKDrawPushClip\n"); assert(false);
}

static void GGDKDrawPopClip(GWindow gw, GRect *old){
    fprintf(stderr, "GDKCALL: GGDKDrawPopClip\n"); assert(false);
}


static void GGDKDrawSetDifferenceMode(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawSetDifferenceMode\n"); assert(false);
}


static void GGDKDrawClear(GWindow gw, GRect *rect){
    fprintf(stderr, "GDKCALL: GGDKDrawClear\n"); assert(false);
}

static void GGDKDrawDrawLine(GWindow gw, int32 x, int32 y, int32 xend, int32 yend, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawLine\n"); assert(false);
}

static void GGDKDrawDrawArrow(GWindow gw, int32 x, int32 y, int32 xend, int32 yend, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawArrow\n"); assert(false);
}

static void GGDKDrawDrawRect(GWindow gw, GRect *rect, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawRect\n"); assert(false);
}

static void GGDKDrawFillRect(GWindow gw, GRect *rect, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawFillRect\n"); assert(false);
}

static void GGDKDrawFillRoundRect(GWindow gw, GRect *rect, int radius, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawFillRoundRect\n"); assert(false);
}

static void GGDKDrawDrawEllipse(GWindow gw, GRect *rect, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawEllipse\n"); assert(false);
}

static void GGDKDrawFillEllipse(GWindow gw, GRect *rect, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawFillEllipse\n"); assert(false);
}

static void GGDKDrawDrawArc(GWindow gw, GRect *rect, int32 sangle, int32 eangle, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawArc\n"); assert(false);
}

static void GGDKDrawDrawPoly(GWindow gw, GPoint *pts, int16 cnt, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawPoly\n"); assert(false);
}

static void GGDKDrawFillPoly(GWindow gw, GPoint *pts, int16 cnt, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawFillPoly\n"); assert(false);
}

static void GGDKDrawScroll(GWindow gw, GRect *rect, int32 hor, int32 vert){
    fprintf(stderr, "GDKCALL: GGDKDrawScroll\n"); assert(false);
}


static void GGDKDrawDrawImage(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawImage\n"); assert(false);
}

static void GGDKDrawTileImage(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y){
    fprintf(stderr, "GDKCALL: GGDKDrawTileImage\n"); assert(false);
}

static void GGDKDrawDrawGlyph(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawGlyph\n"); assert(false);
}

static void GGDKDrawDrawImageMagnified(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y, int32 width, int32 height){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawImageMag\n"); assert(false);
}

static GImage* GGDKDrawCopyScreenToImage(GWindow gw, GRect *rect){
    fprintf(stderr, "GDKCALL: GGDKDrawCopyScreenToImage\n"); assert(false);
}

static void GGDKDrawDrawPixmap(GWindow gw1, GWindow gw2, GRect *src, int32 x, int32 y){
    fprintf(stderr, "GDKCALL: GGDKDrawDrawPixmap\n"); assert(false);
}

static void GGDKDrawTilePixmap(GWindow gw1, GWindow gw2, GRect *src, int32 x, int32 y){
    fprintf(stderr, "GDKCALL: GGDKDrawTilePixmap\n"); assert(false);
}


static GIC* GGDKDrawCreateInputContext(GWindow gw, enum gic_style style){
    fprintf(stderr, "GDKCALL: GGDKDrawCreateInputContext\n"); assert(false);
}

static void GGDKDrawSetGIC(GWindow gw, GIC *gic, int x, int y){
    fprintf(stderr, "GDKCALL: GGDKDrawSetGIC\n"); assert(false);
}


static void GGDKDrawGrabSelection(GWindow w, enum selnames sel){
    fprintf(stderr, "GDKCALL: GGDKDrawGrabSelection\n"); assert(false);
}

static void GGDKDrawAddSelectionType(GWindow w, enum selnames sel, char *type, void *data, int32 cnt, int32 unitsize, void *gendata(void *, int32 *len), void freedata(void *)){
    fprintf(stderr, "GDKCALL: GGDKDrawAddSelectionType\n"); assert(false);
}

static void* GGDKDrawRequestSelection(GWindow w, enum selnames sn, char *typename, int32 *len){
    fprintf(stderr, "GDKCALL: GGDKDrawRequestSelection\n"); assert(false);
}

static int GGDKDrawSelectionHasType(GWindow w, enum selnames sn, char *typename){
    fprintf(stderr, "GDKCALL: GGDKDrawSelectionHasType\n"); assert(false);
}

static void GGDKDrawBindSelection(GDisplay *gdisp, enum selnames sn, char *atomname){
    fprintf(stderr, "GDKCALL: GGDKDrawBindSelection\n"); assert(false);
}

static int GGDKDrawSelectionHasOwner(GDisplay *gdisp, enum selnames sn){
    fprintf(stderr, "GDKCALL: GGDKDrawSelectionHasOwner\n"); assert(false);
}


static void GGDKDrawPointerUngrab(GDisplay *gdisp){
    fprintf(stderr, "GDKCALL: GGDKDrawPointerUngrab\n"); assert(false);
}

static void GGDKDrawPointerGrab(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawPointerGrab\n"); assert(false);
}

static void GGDKDrawRequestExpose(GWindow gw, GRect *gr, int set){
    fprintf(stderr, "GDKCALL: GGDKDrawRequestExpose\n"); assert(false);
}

static void GGDKDrawForceUpdate(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawForceUpdate\n"); assert(false);
}

static void GGDKDrawSync(GDisplay *gdisp){
    fprintf(stderr, "GDKCALL: GGDKDrawSync\n"); assert(false);
}

static void GGDKDrawSkipMouseMoveEvents(GWindow gw, GEvent *gevent){
    fprintf(stderr, "GDKCALL: GGDKDrawSkipMouseMoveEvents\n"); assert(false);
}

static void GGDKDrawProcessPendingEvents(GDisplay *gdisp){
    fprintf(stderr, "GDKCALL: GGDKDrawProcessPendingEvents\n"); assert(false);
}

static void GGDKDrawProcessWindowEvents(GWindow gw){
    fprintf(stderr, "GDKCALL: GGDKDrawProcessWindowEvents\n"); assert(false);
}

static void GGDKDrawProcessOneEvent(GDisplay *gdisp){
    fprintf(stderr, "GDKCALL: GGDKDrawProcessOneEvent\n"); assert(false);
}

static void GGDKDrawEventLoop(GDisplay *gdisp){
    fprintf(stderr, "GDKCALL: GGDKDrawEventLoop\n"); assert(false);
}

static void GGDKDrawPostEvent(GEvent *e){
    fprintf(stderr, "GDKCALL: GGDKDrawPostEvent\n"); assert(false);
}

static void GGDKDrawPostDragEvent(GWindow w, GEvent *mouse, enum event_type et){
    fprintf(stderr, "GDKCALL: GGDKDrawPostDragEvent\n"); assert(false);
}

static int GGDKDrawRequestDeviceEvents(GWindow w, int devcnt, struct gdeveventmask *de){
    fprintf(stderr, "GDKCALL: GGDKDrawRequestDeviceEvents\n"); assert(false);
}


static GTimer* GGDKDrawRequestTimer(GWindow w, int32 time_from_now, int32 frequency, void *userdata){
    fprintf(stderr, "GDKCALL: GGDKDrawRequestTimer\n"); assert(false);
}

static void GGDKDrawCancelTimer(GTimer *timer){
    fprintf(stderr, "GDKCALL: GGDKDrawCancelTimer\n"); assert(false);
}


static void GGDKDrawSyncThread(GDisplay *gdisp, void (*func)(void *), void *data){
    fprintf(stderr, "GDKCALL: GGDKDrawSyncThread\n"); assert(false);
}


static GWindow GGDKDrawPrinterStartJob(GDisplay *gdisp, void *user_data, GPrinterAttrs *attrs){
    fprintf(stderr, "GDKCALL: GGDKDrawStartJob\n"); assert(false);
}

static void GGDKDrawPrinterNextPage(GWindow w){
    fprintf(stderr, "GDKCALL: GGDKDrawNextPage\n"); assert(false);
}

static int GGDKDrawPrinterEndJob(GWindow w, int cancel){
    fprintf(stderr, "GDKCALL: GGDKDrawEndJob\n"); assert(false);
}


static void GGDKDrawGetFontMetrics(GWindow gw, GFont *gfont, int *as, int *ds, int *ld){
    fprintf(stderr, "GDKCALL: GGDKDrawGetFontMetrics\n"); assert(false);
}


static enum gcairo_flags GGDKDrawHasCairo(GWindow w){
    fprintf(stderr, "GDKCALL: gcairo_flags GGDKDrawHasCairo\n"); assert(false);
}


static void GGDKDrawPathStartNew(GWindow w){
    fprintf(stderr, "GDKCALL: GGDKDrawStartNewPath\n"); assert(false);
}

static void GGDKDrawPathClose(GWindow w){
    fprintf(stderr, "GDKCALL: GGDKDrawClosePath\n"); assert(false);
}

static void GGDKDrawPathMoveTo(GWindow w, double x, double y){
    fprintf(stderr, "GDKCALL: GGDKDrawMoveto\n"); assert(false);
}

static void GGDKDrawPathLineTo(GWindow w, double x, double y){
    fprintf(stderr, "GDKCALL: GGDKDrawLineto\n"); assert(false);
}

static void GGDKDrawPathCurveTo(GWindow w, double cx1, double cy1, double cx2, double cy2, double x, double y){
    fprintf(stderr, "GDKCALL: GGDKDrawCurveto\n"); assert(false);
}

static void GGDKDrawPathStroke(GWindow w, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawStroke\n"); assert(false);
}

static void GGDKDrawPathFill(GWindow w, Color col){
    fprintf(stderr, "GDKCALL: GGDKDrawFill\n"); assert(false);
}

static void GGDKDrawPathFillAndStroke(GWindow w, Color fillcol, Color strokecol){
    fprintf(stderr, "GDKCALL: GGDKDrawFillAndStroke\n"); assert(false);
}


static void GGDKDrawLayoutInit(GWindow w, char *text, int cnt, GFont *fi){
    fprintf(stderr, "GDKCALL: GGDKDrawLayoutInit\n"); assert(false);
}

static void GGDKDrawLayoutDraw(GWindow w, int32 x, int32 y, Color fg){
    fprintf(stderr, "GDKCALL: GGDKDrawLayoutDraw\n"); assert(false);
}

static void GGDKDrawLayoutIndexToPos(GWindow w, int index, GRect *pos){
    fprintf(stderr, "GDKCALL: GGDKDrawLayoutIndexToPos\n"); assert(false);
}

static int GGDKDrawLayoutXYToIndex(GWindow w, int x, int y){
    fprintf(stderr, "GDKCALL: GGDKDrawLayoutXYToIndex\n"); assert(false);
}

static void GGDKDrawLayoutExtents(GWindow w, GRect *size){
    fprintf(stderr, "GDKCALL: GGDKDrawLayoutExtents\n"); assert(false);
}

static void GGDKDrawLayoutSetWidth(GWindow w, int width){
    fprintf(stderr, "GDKCALL: GGDKDrawLayoutSetWidth\n"); assert(false);
}

static int GGDKDrawLayoutLineCount(GWindow w){
    fprintf(stderr, "GDKCALL: GGDKDrawLayoutLineCount\n"); assert(false);
}

static int GGDKDrawLayoutLineStart(GWindow w, int line){
    fprintf(stderr, "GDKCALL: GGDKDrawLayoutLineStart\n"); assert(false);
}

static void GGDKDrawStartNewSubPath(GWindow w){
    fprintf(stderr, "GDKCALL: GGDKDrawStartNewSubPath\n"); assert(false);
}

static int GGDKDrawFillRuleSetWinding(GWindow w){
    fprintf(stderr, "GDKCALL: GGDKDrawFillRuleSetWinding\n"); assert(false);
}


static void GGDKDrawPushClipOnly(GWindow w){
    fprintf(stderr, "GDKCALL: GGDKDrawPushClipOnly\n"); assert(false);
}

static void GGDKDrawClipPreserve(GWindow w){
    fprintf(stderr, "GDKCALL: GGDKDrawClipPreserve\n"); assert(false);
}


// Our function VTable
static struct displayfuncs gdkfuncs = {
    GGDKDrawInit,
    NULL, // GGDKDrawTerm,
    NULL, // GGDKDrawNativeDisplay,

    GGDKDrawSetDefaultIcon,

    GGDKDrawCreateTopWindow,
    GGDKDrawCreateSubWindow,
    GGDKDrawCreatePixmap,
    GGDKDrawCreateBitmap,
    GGDKDrawCreateCursor,
    GGDKDrawDestroyWindow,
    GGDKDrawDestroyCursor,
    GGDKDrawNativeWindowExists, //Not sure what this is meant to do...
    GGDKDrawSetZoom,
    GGDKDrawSetWindowBorder,
    GGDKDrawSetWindowBackground,
    GGDKDrawSetDither,

    GGDKDrawReparentWindow,
    GGDKDrawSetVisible,
    GGDKDrawMove,
    GGDKDrawTrueMove,
    GGDKDrawResize,
    GGDKDrawMoveResize,
    GGDKDrawRaise,
    GGDKDrawRaiseAbove,
    GGDKDrawIsAbove,
    GGDKDrawLower,
    GGDKDrawSetWindowTitles,
    GGDKDrawSetWindowTitles8,
    GGDKDrawGetWindowTitle,
    GGDKDrawGetWindowTitle8,
    GGDKDrawSetTransientFor,
    GGDKDrawGetPointerPosition,
    GGDKDrawGetPointerWindow,
    GGDKDrawSetCursor,
    GGDKDrawGetCursor,
    GGDKDrawGetRedirectWindow,
    GGDKDrawTranslateCoordinates,

    GGDKDrawBeep,
    GGDKDrawFlush,

    GGDKDrawPushClip,
    GGDKDrawPopClip,

    GGDKDrawSetDifferenceMode,

    GGDKDrawClear,
    GGDKDrawDrawLine,
    GGDKDrawDrawArrow,
    GGDKDrawDrawRect,
    GGDKDrawFillRect,
    GGDKDrawFillRoundRect,
    GGDKDrawDrawEllipse,
    GGDKDrawFillEllipse,
    GGDKDrawDrawArc,
    GGDKDrawDrawPoly,
    GGDKDrawFillPoly,
    GGDKDrawScroll,

    GGDKDrawDrawImage,
    GGDKDrawTileImage,
    GGDKDrawDrawGlyph,
    GGDKDrawDrawImageMagnified,
    GGDKDrawCopyScreenToImage,
    GGDKDrawDrawPixmap,
    GGDKDrawTilePixmap,

    GGDKDrawCreateInputContext,
    GGDKDrawSetGIC,

    GGDKDrawGrabSelection,
    GGDKDrawAddSelectionType,
    GGDKDrawRequestSelection,
    GGDKDrawSelectionHasType,
    GGDKDrawBindSelection,
    GGDKDrawSelectionHasOwner,

    GGDKDrawPointerUngrab,
    GGDKDrawPointerGrab,
    GGDKDrawRequestExpose,
    GGDKDrawForceUpdate,
    GGDKDrawSync,
    GGDKDrawSkipMouseMoveEvents,
    GGDKDrawProcessPendingEvents,
    GGDKDrawProcessWindowEvents,
    GGDKDrawProcessOneEvent,
    GGDKDrawEventLoop,
    GGDKDrawPostEvent,
    GGDKDrawPostDragEvent,
    GGDKDrawRequestDeviceEvents,

    GGDKDrawRequestTimer,
    GGDKDrawCancelTimer,

    GGDKDrawSyncThread,

    GGDKDrawPrinterStartJob,
    GGDKDrawPrinterNextPage,
    GGDKDrawPrinterEndJob,

    GGDKDrawGetFontMetrics,

    GGDKDrawHasCairo,
    GGDKDrawPathStartNew,
    GGDKDrawPathClose,
    GGDKDrawPathMoveTo,
    GGDKDrawPathLineTo,
    GGDKDrawPathCurveTo,
    GGDKDrawPathStroke,
    GGDKDrawPathFill,
    GGDKDrawPathFillAndStroke,

    GGDKDrawLayoutInit,
    GGDKDrawLayoutDraw,
    GGDKDrawLayoutIndexToPos,
    GGDKDrawLayoutXYToIndex,
    GGDKDrawLayoutExtents,
    GGDKDrawLayoutSetWidth,
    GGDKDrawLayoutLineCount,
    GGDKDrawLayoutLineStart,
    GGDKDrawStartNewSubPath,
    GGDKDrawFillRuleSetWinding,

    GGDKDrawPushClipOnly,
    GGDKDrawClipPreserve
};

// Protected member functions (package-level)

GDisplay *_GGDKDraw_CreateDisplay(char *displayname, char *programname) {
    GGDKDisplay *gdisp;
    GdkDisplay *display;
    GGDKWindow groot;

    display = gdk_display_open(displayname);
    if (display == NULL) {
        return NULL;
    }

    gdisp = (GGDKDisplay*)calloc(1, sizeof(GGDKDisplay));
    if (gdisp == NULL) {
        gdk_display_close(display);
        return NULL;
    }

    gdisp->funcs = &gdkfuncs;
    gdisp->display = display;
    gdisp->screen = gdk_display_get_default_screen(display);
    gdisp->root = gdk_screen_get_root_window(gdisp->screen);
    gdisp->res = gdk_screen_get_resolution(gdisp->screen);
    gdisp->scale_screen_by = 1;

    groot = (GGDKWindow)calloc(1, sizeof(struct ggdkwindow));
    if (groot == NULL) {
        free(gdisp);
        gdk_display_close(display);
        return NULL;
    }

    gdisp->groot = (GWindow)groot;
    groot->ggc = _GGDKDrawNewGGC();
    groot->display = (GDisplay*)gdisp;
    groot->w = gdisp->root;
    groot->pos.width = gdk_screen_get_width(gdisp->screen);
    groot->pos.height = gdk_screen_get_height(gdisp->screen);
    groot->is_toplevel = true;
    groot->is_visible = true;

    //GGDKResourceInit(gdisp,programname);

    gdisp->def_background = GResourceFindColor("Background", COLOR_CREATE(0xf5,0xff,0xfa));
    gdisp->def_foreground = GResourceFindColor("Foreground", COLOR_CREATE(0x00,0x00,0x00));
    if (GResourceFindBool("Synchronize", false)) {
        gdk_display_sync(gdisp->display);
    }

    (gdisp->funcs->init)((GDisplay *) gdisp);
    //gdisp->top_window_count = 0; //Reference counting toplevel windows?

    _GDraw_InitError((GDisplay *) gdisp);

    return (GDisplay *)gdisp;
}

void _GGDKDraw_DestroyDisplay(GDisplay *gdisp) {
    GGDKDisplay* gdispc = (GGDKDisplay*)(gdisp);

    if (gdispc->groot != NULL) {
        if (gdispc->groot->ggc != NULL) {
            free(gdispc->groot->ggc);
            gdispc->groot->ggc = NULL;
        }
        free(gdispc->groot);
        gdispc->groot = NULL;
    }

    if (gdispc->display != NULL) {
        gdk_display_close(gdispc->display);
        gdispc->display = NULL;
    }
    return;
}

