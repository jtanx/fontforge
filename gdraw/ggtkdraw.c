/* Copyright (C) 2016-2019 by Jeremy Tan */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
*  \file  ggtkdraw.c
*  \brief GTK drawing backend.
*/

#include "ggtkdrawP.h"

#ifdef FONTFORGE_CAN_USE_GTK
#include "gkeysym.h"
#include "gresource.h"
#include "ustring.h"
#include <assert.h>
#include <math.h>
#include <string.h>

static void GGTKDrawInit(GDisplay *gdisp) {
    Log(LOGDEBUG, "");
}

static void GGTKDrawSetDefaultIcon(GWindow icon) {
}

static GWindow GGTKDrawCreateTopWindow(GDisplay *gdisp, GRect *pos, int (*eh)(GWindow gw, GEvent *), void *user_data, GWindowAttrs *gattrs) {
    Log(LOGVERBOSE, " ");
	
	return NULL;
}

static GWindow GGTKDrawCreateSubWindow(GWindow gw, GRect *pos, int (*eh)(GWindow gw, GEvent *), void *user_data, GWindowAttrs *gattrs) {
    Log(LOGVERBOSE, " ");
	return NULL;
}

static GWindow GGTKDrawCreatePixmap(GDisplay *gdisp, GWindow similar, uint16 width, uint16 height) {
    Log(LOGVERBOSE, " ");
	return NULL;
}

static GWindow GGTKDrawCreateBitmap(GDisplay *gdisp, uint16 width, uint16 height, uint8 *data) {
    Log(LOGVERBOSE, " ");
	return NULL;
}

static GCursor GGTKDrawCreateCursor(GWindow src, GWindow mask, Color fg, Color bg, int16 x, int16 y) {
    Log(LOGVERBOSE, " ");
    return 0;
}

static void GGTKDrawDestroyCursor(GDisplay *disp, GCursor gcursor) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawDestroyWindow(GWindow w) {
    Log(LOGVERBOSE, " ");
}

static int GGTKDrawNativeWindowExists(GDisplay *UNUSED(gdisp), void *native_window) {
    Log(LOGVERBOSE, " ");
    return false;
}

static void GGTKDrawSetZoom(GWindow UNUSED(gw), GRect *UNUSED(size), enum gzoom_flags UNUSED(flags)) {
    //Log(LOGVERBOSE, " ");
    // Not implemented.
}

// Not possible?
static void GGTKDrawSetWindowBorder(GWindow UNUSED(gw), int UNUSED(width), Color UNUSED(gcol)) {
    Log(LOGWARN, "GGTKDrawSetWindowBorder unimplemented!");
}

static void GGTKDrawSetWindowBackground(GWindow w, Color gcol) {
    Log(LOGVERBOSE, " ");
}

static int GGTKDrawSetDither(GDisplay *UNUSED(gdisp), int UNUSED(set)) {
    // Not implemented; does nothing.
    return false;
}

static void GGTKDrawReparentWindow(GWindow child, GWindow newparent, int x, int y) {
    Log(LOGWARN, "GGTKDrawReparentWindow called: Reparenting should NOT be used!");
}

static void GGTKDrawSetVisible(GWindow w, int show) {
    Log(LOGDEBUG, "0x%p %d", w, show);
}

static void GGTKDrawMove(GWindow gw, int32 x, int32 y) {
    //Log(LOGDEBUG, "%p:%s, %d %d", gw, ((GGTKWindow) gw)->window_title, x, y);
}

static void GGTKDrawTrueMove(GWindow w, int32 x, int32 y) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawResize(GWindow gw, int32 w, int32 h) {
    //Log(LOGDEBUG, "%p:%s, %d %d", gw, ((GGTKWindow) gw)->window_title, w, h);
}

static void GGTKDrawMoveResize(GWindow gw, int32 x, int32 y, int32 w, int32 h) {
    //Log(LOGDEBUG, "%p:%s, %d %d %d %d", gw, ((GGTKWindow) gw)->window_title, x, y, w, h);
}

static void GGTKDrawRaise(GWindow w) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawRaiseAbove(GWindow gw1, GWindow gw2) {
    Log(LOGVERBOSE, " ");
}

// Only used once in gcontainer - force it to call GDrawRaiseAbove
static int GGTKDrawIsAbove(GWindow UNUSED(gw1), GWindow UNUSED(gw2)) {
    Log(LOGVERBOSE, " ");
    return false;
}

static void GGTKDrawLower(GWindow gw) {
    Log(LOGVERBOSE, " ");
}

// Icon title is ignored.
static void GGTKDrawSetWindowTitles8(GWindow w, const char *title, const char *UNUSED(icontitle)) {
    Log(LOGVERBOSE, " "); // assert(false);
}

static void GGTKDrawSetWindowTitles(GWindow gw, const unichar_t *title, const unichar_t *UNUSED(icontitle)) {
    Log(LOGVERBOSE, " ");
}

static unichar_t *GGTKDrawGetWindowTitle(GWindow gw) {
    Log(LOGVERBOSE, " "); // assert(false);
    return NULL;
}

static char *GGTKDrawGetWindowTitle8(GWindow gw) {
    Log(LOGVERBOSE, " ");
    return NULL;
}

static void GGTKDrawSetTransientFor(GWindow transient, GWindow owner) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawGetPointerPosition(GWindow w, GEvent *ret) {
    Log(LOGVERBOSE, " ");
}

static GWindow GGTKDrawGetPointerWindow(GWindow gw) {
    Log(LOGVERBOSE, " ");
    return NULL;
}

static void GGTKDrawSetCursor(GWindow w, GCursor gcursor) {
    Log(LOGVERBOSE, " ");
}

static GCursor GGTKDrawGetCursor(GWindow gw) {
    Log(LOGVERBOSE, " ");
    return (GCursor)0;
}

static GWindow GGTKDrawGetRedirectWindow(GDisplay *UNUSED(gdisp)) {
    Log(LOGVERBOSE, " ");
    // Not implemented.
    return NULL;
}

static void GGTKDrawTranslateCoordinates(GWindow from, GWindow to, GPoint *pt) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawBeep(GDisplay *gdisp) {
    //Log(LOGVERBOSE, " ");
}

static void GGTKDrawFlush(GDisplay *gdisp) {
    //Log(LOGVERBOSE, " ");
}

static void GGTKDrawScroll(GWindow w, GRect *rect, int32 hor, int32 vert) {
    //Log(LOGVERBOSE, " ");
}

static GIC *GGTKDrawCreateInputContext(GWindow UNUSED(gw), enum gic_style UNUSED(style)) {
    Log(LOGVERBOSE, " ");
    return NULL;
}

static void GGTKDrawSetGIC(GWindow UNUSED(gw), GIC *UNUSED(gic), int UNUSED(x), int UNUSED(y)) {
    Log(LOGVERBOSE, " ");
}

static int GGTKDrawKeyState(GWindow w, int keysym) {
    //Log(LOGVERBOSE, " ");
	
	return false;
}

static void GGTKDrawGrabSelection(GWindow w, enum selnames sn) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawAddSelectionType(GWindow w, enum selnames sel, char *type, void *data, int32 cnt, int32 unitsize,
                                     void *gendata(void *, int32 *len), void freedata(void *)) {
    Log(LOGVERBOSE, " ");
}

static void *GGTKDrawRequestSelection(GWindow w, enum selnames sn, char *typename, int32 *len) {
    return NULL;
}

static int GGTKDrawSelectionHasType(GWindow w, enum selnames sn, char *typename) {
    Log(LOGVERBOSE, " ");
	
	return false;
}

static void GGTKDrawBindSelection(GDisplay *disp, enum selnames sn, char *atomname) {
    Log(LOGVERBOSE, " ");
}

static int GGTKDrawSelectionHasOwner(GDisplay *disp, enum selnames sn) {
    Log(LOGVERBOSE, " ");
    return false;
}

static void GGTKDrawPointerUngrab(GDisplay *gdisp) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawPointerGrab(GWindow w) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawRequestExpose(GWindow w, GRect *rect, int UNUSED(doclear)) {
    //Log(LOGDEBUG, "%p [%s]", w, ((GGTKWindow) w)->window_title);
}

static void GGTKDrawForceUpdate(GWindow gw) {
    //Log(LOGVERBOSE, " ");
}

static void GGTKDrawSync(GDisplay *gdisp) {
    //Log(LOGVERBOSE, " ");
}

static void GGTKDrawSkipMouseMoveEvents(GWindow UNUSED(gw), GEvent *UNUSED(gevent)) {
    //Log(LOGVERBOSE, " ");
    // Not implemented, not needed.
}

static void GGTKDrawProcessPendingEvents(GDisplay *gdisp) {
    //Log(LOGVERBOSE, " ");
}

static void GGTKDrawProcessWindowEvents(GWindow w) {
    Log(LOGWARN, "This function SHOULD NOT BE CALLED! Window: %p", w);
}

static void GGTKDrawProcessOneEvent(GDisplay *gdisp) {
    //Log(LOGVERBOSE, " ");
}

static void GGTKDrawEventLoop(GDisplay *gdisp) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawPostEvent(GEvent *e) {
    //Log(LOGVERBOSE, " ");
}

static void GGTKDrawPostDragEvent(GWindow w, GEvent *mouse, enum event_type et) {
    Log(LOGVERBOSE, " ");
}

static int GGTKDrawRequestDeviceEvents(GWindow w, int devcnt, struct gdeveventmask *de) {
    Log(LOGVERBOSE, " ");
    return 0; //Not sure how to handle... For tablets...
}

static GTimer *GGTKDrawRequestTimer(GWindow w, int32 time_from_now, int32 frequency, void *userdata) {
    //Log(LOGVERBOSE, " ");
    return NULL;
}

static void GGTKDrawCancelTimer(GTimer *timer) {
    //Log(LOGVERBOSE, " ");
}

static void GGTKDrawSyncThread(GDisplay *UNUSED(gdisp), void (*func)(void *), void *UNUSED(data)) {
    Log(LOGVERBOSE, " "); // For some shitty gio impl. Ignore ignore ignore!
}

static GWindow GGTKDrawPrinterStartJob(GDisplay *UNUSED(gdisp), void *UNUSED(user_data), GPrinterAttrs *UNUSED(attrs)) {
    Log(LOGERR, " ");
    assert(false);
	
	return NULL;
}

static void GGTKDrawPrinterNextPage(GWindow UNUSED(w)) {
    Log(LOGERR, " ");
    assert(false);
}

static int GGTKDrawPrinterEndJob(GWindow UNUSED(w), int UNUSED(cancel)) {
    Log(LOGERR, " ");
    assert(false);
	
	return 0;
}

// Our function VTable
static struct displayfuncs gtkfuncs = {
    GGTKDrawInit,
    NULL, // GGTKDrawTerm,
    NULL, // GGTKDrawNativeDisplay,

    GGTKDrawSetDefaultIcon,

    GGTKDrawCreateTopWindow,
    GGTKDrawCreateSubWindow,
    GGTKDrawCreatePixmap,
    GGTKDrawCreateBitmap,
    GGTKDrawCreateCursor,
    GGTKDrawDestroyWindow,
    GGTKDrawDestroyCursor,
    GGTKDrawNativeWindowExists, //Not sure what this is meant to do...
    GGTKDrawSetZoom,
    GGTKDrawSetWindowBorder,
    GGTKDrawSetWindowBackground,
    GGTKDrawSetDither,

    GGTKDrawReparentWindow,
    GGTKDrawSetVisible,
    GGTKDrawMove,
    GGTKDrawTrueMove,
    GGTKDrawResize,
    GGTKDrawMoveResize,
    GGTKDrawRaise,
    GGTKDrawRaiseAbove,
    GGTKDrawIsAbove,
    GGTKDrawLower,
    GGTKDrawSetWindowTitles,
    GGTKDrawSetWindowTitles8,
    GGTKDrawGetWindowTitle,
    GGTKDrawGetWindowTitle8,
    GGTKDrawSetTransientFor,
    GGTKDrawGetPointerPosition,
    GGTKDrawGetPointerWindow,
    GGTKDrawSetCursor,
    GGTKDrawGetCursor,
    GGTKDrawGetRedirectWindow,
    GGTKDrawTranslateCoordinates,

    GGTKDrawBeep,
    GGTKDrawFlush,

    GGTKDrawPushClip,
    GGTKDrawPopClip,

    GGTKDrawSetDifferenceMode,

    GGTKDrawClear,
    GGTKDrawDrawLine,
    GGTKDrawDrawArrow,
    GGTKDrawDrawRect,
    GGTKDrawFillRect,
    GGTKDrawFillRoundRect,
    GGTKDrawDrawEllipse,
    GGTKDrawFillEllipse,
    GGTKDrawDrawArc,
    GGTKDrawDrawPoly,
    GGTKDrawFillPoly,
    GGTKDrawScroll,

    GGTKDrawDrawImage,
    NULL, // GGTKDrawTileImage - Unused function
    GGTKDrawDrawGlyph,
    GGTKDrawDrawImageMagnified,
    NULL, // GGTKDrawCopyScreenToImage - Unused function
    GGTKDrawDrawPixmap,
    NULL, // GGTKDrawTilePixmap - Unused function

    GGTKDrawCreateInputContext,
    GGTKDrawSetGIC,
    GGTKDrawKeyState,

    GGTKDrawGrabSelection,
    GGTKDrawAddSelectionType,
    GGTKDrawRequestSelection,
    GGTKDrawSelectionHasType,
    GGTKDrawBindSelection,
    GGTKDrawSelectionHasOwner,

    GGTKDrawPointerUngrab,
    GGTKDrawPointerGrab,
    GGTKDrawRequestExpose,
    GGTKDrawForceUpdate,
    GGTKDrawSync,
    GGTKDrawSkipMouseMoveEvents,
    GGTKDrawProcessPendingEvents,
    GGTKDrawProcessWindowEvents,
    GGTKDrawProcessOneEvent,
    GGTKDrawEventLoop,
    GGTKDrawPostEvent,
    GGTKDrawPostDragEvent,
    GGTKDrawRequestDeviceEvents,

    GGTKDrawRequestTimer,
    GGTKDrawCancelTimer,

    GGTKDrawSyncThread,

    GGTKDrawPrinterStartJob,
    GGTKDrawPrinterNextPage,
    GGTKDrawPrinterEndJob,

    GGTKDrawGetFontMetrics,

    GGTKDrawHasCairo,
    GGTKDrawPathStartNew,
    GGTKDrawPathClose,
    GGTKDrawPathMoveTo,
    GGTKDrawPathLineTo,
    GGTKDrawPathCurveTo,
    GGTKDrawPathStroke,
    GGTKDrawPathFill,
    GGTKDrawPathFillAndStroke,

    GGTKDrawLayoutInit,
    GGTKDrawLayoutDraw,
    GGTKDrawLayoutIndexToPos,
    GGTKDrawLayoutXYToIndex,
    GGTKDrawLayoutExtents,
    GGTKDrawLayoutSetWidth,
    GGTKDrawLayoutLineCount,
    GGTKDrawLayoutLineStart,
    GGTKDrawStartNewSubPath,
    GGTKDrawFillRuleSetWinding,

    GGTKDrawDoText8,

    GGTKDrawPushClipOnly,
    GGTKDrawClipPreserve
};

// Protected member functions (package-level)

GDisplay *_GGTKDraw_CreateDisplay(char *displayname, char *UNUSED(programname)) {
    return NULL;
}

void _GGTKDraw_DestroyDisplay(GDisplay *disp) {
    Log(LOGDEBUG, "");
    return;
}

#endif // FONTFORGE_CAN_USE_GTK
