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

#include <fontforge-config.h>

/**
*  \file  ggtkdraw.c
*  \brief GTK drawing backend.
*/

#ifdef FONTFORGE_CAN_USE_GTK
#include "fontP.h"
#include "ggtkdrawP.h"
#include "gkeysym.h"
#include "gresource.h"
#include "ustring.h"

#include <assert.h>
#include <math.h>
#include <string.h>

// Forward declarations
static void GGTKDrawSetCursor(GWindow w, GCursor gcursor);
static void GGTKDrawSetTransientFor(GWindow transient, GWindow owner);
static void GGTKDrawSetWindowBackground(GWindow w, Color gcol);


// Private member functions (file-level)

static GGC *_GGTKDraw_NewGGC(void) {
    GGC *ggc = calloc(1, sizeof(GGC));
    if (ggc == NULL) {
        Log(LOGDEBUG, "GGC: Memory allocation failed!");
        return NULL;
    }

    ggc->clip.width = ggc->clip.height = 0x7fff;
    ggc->fg = 0;
    ggc->bg = 0xffffff;
    return ggc;
}


static GWindow _GGTKDraw_CreateWindow(GGTKDisplay *gdisp, GGTKWindow gw, GRect *pos,
                                      int (*eh)(GWindow, GEvent *), void *user_data, GWindowAttrs *wattrs) {

    GWindowAttrs temp = GWINDOWATTRS_EMPTY;
    GGTKWindow nw = (GGTKWindow)calloc(1, sizeof(struct ggtkwindow));
    GtkWindowType window_type = GTK_WINDOW_TOPLEVEL;
    char *window_title = NULL;

    if (nw == NULL) {
        Log(LOGWARN, "_GGTKDraw_CreateWindow: GGTKWindow calloc failed.");
        return NULL;
    }
    if (wattrs == NULL) {
        wattrs = &temp;
    }
    if (gw == NULL) {
        gw = gdisp->groot; // Creating a top-level window. Set parent as default root.
    }

    // Now check window type
    if ((wattrs->mask & wam_nodecor) && wattrs->nodecoration) {
        // Is a modeless dialogue
        nw->is_popup = true;
        nw->is_dlg = true;
        nw->not_restricted = true;
        window_type = GTK_WINDOW_POPUP;
    } else if ((wattrs->mask & wam_isdlg) && wattrs->is_dlg) {
        nw->is_dlg = true;
    }
    if ((wattrs->mask & wam_notrestricted) && wattrs->not_restricted) {
        nw->not_restricted = true;
    }
    nw->is_toplevel = (gw == gdisp->groot);

    // Drawing context
    nw->ggc = _GGTKDraw_NewGGC();
    if (nw->ggc == NULL) {
        Log(LOGWARN, "_GGTKDraw_NewGGC returned NULL");
        free(nw);
        return NULL;
    }

    // Base fields
    nw->display = gdisp;
    nw->eh = eh;
    nw->parent = gw;
    nw->pos = *pos;
    nw->user_data = user_data;

    // Window title, hints and event mask
    int event_mask = GDK_EXPOSURE_MASK | GDK_STRUCTURE_MASK;
    if (nw->is_toplevel) {
        // Default event mask for toplevel windows
        event_mask |= GDK_FOCUS_CHANGE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK;

        // Icon titles are ignored.
        if ((wattrs->mask & wam_utf8_wtitle) && (wattrs->utf8_window_title != NULL)) {
            window_title = copy(wattrs->utf8_window_title);
        } else if ((wattrs->mask & wam_wtitle) && (wattrs->window_title != NULL)) {
            window_title = u2utf8_copy(wattrs->window_title);
        }
    }

    // Further event mask flags
    if (wattrs->mask & wam_events) {
        if (wattrs->event_masks & (1 << et_char)) {
            event_mask |= GDK_KEY_PRESS_MASK;
        }
        if (wattrs->event_masks & (1 << et_charup)) {
            event_mask |= GDK_KEY_RELEASE_MASK;
        }
        if (wattrs->event_masks & (1 << et_mousemove)) {
            event_mask |= GDK_POINTER_MOTION_MASK;
        }
        if (wattrs->event_masks & (1 << et_mousedown)) {
            event_mask |= GDK_BUTTON_PRESS_MASK | GDK_SCROLL_MASK;
        }
        if (wattrs->event_masks & (1 << et_mouseup)) {
            event_mask |= GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK;
        }
        if (wattrs->event_masks & (1 << et_visibility)) {
            event_mask |= GDK_VISIBILITY_NOTIFY_MASK;
        }
    }

    if (wattrs->mask & wam_restrict) {
        nw->restrict_input_to_me = wattrs->restrict_input_to_me;
    }
    if (wattrs->mask & wam_redirect) {
        // FIXME: I don't actually do anything with this...
        nw->redirect_chars_to_me = wattrs->redirect_chars_to_me;
    }

    // Now create stuff
    if (nw->is_toplevel) {
        GtkWindow *window = GTK_WINDOW(gtk_window_new(window_type));
        if (window == NULL) {
            Log(LOGWARN, "Failed to create a GtkWindow");
            free(window_title);
            free(nw->ggc);
            free(nw);
            return NULL;
        }

        if (window_title) {
            gtk_window_set_title(window, window_title);
            free(window_title);
        }

        gtk_window_set_default_size(window, pos->width, pos->height);

        if (!(wattrs->mask & wam_positioned) || (wattrs->mask & wam_centered)) {
            // FIXME: This doesn't subtract off the taskbar area... oh well
            gtk_window_set_position (window, GTK_WIN_POS_CENTER);
        } else {
            gtk_window_move(window, nw->pos.x, nw->pos.y);
        }

        // Set icon
        GdkPixbuf* pb = NULL;
        //GGTKWindow icon = gdisp->default_icon;
        //if (((wattrs->mask & wam_icon) && wattrs->icon != NULL) && ((GGTKWindow)wattrs->icon)->is_pixmap) {
        //    icon = (GGTKWindow) wattrs->icon;
        //}
        //if (icon != NULL) {
        //    pb = gdk_pixbuf_get_from_surface(icon->cs, 0, 0, icon->pos.width, icon->pos.height);
        //}
        gtk_window_set_icon(window, pb);
        if (pb) {
            g_object_unref(pb);
        }

        GdkGeometry geom = {0};
        GdkWindowHints hints = 0;
        if (wattrs->mask & wam_palette) {
            hints |= GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE;
        }
        if ((wattrs->mask & wam_noresize) && wattrs->noresize) {
            hints |= GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE | GDK_HINT_BASE_SIZE;
        }

        // Hmm does this seem right?
        geom.base_width = geom.min_width = geom.max_width = pos->width;
        geom.base_height = geom.min_height = geom.max_height = pos->height;

        hints |= GDK_HINT_POS;
        nw->was_positioned = true;

        gtk_window_set_geometry_hints(window, NULL, &geom, hints);

        if ((wattrs->mask & wam_transient) && wattrs->transient != NULL) {
            // GGTKDrawSetTransientFor((GWindow)nw, wattrs->transient);
            nw->is_dlg = true;
        } else if (nw->restrict_input_to_me) {
            gtk_window_set_modal(window, true);
        }
        nw->isverytransient = (wattrs->mask & wam_verytransient) ? 1 : 0;

        // Finally make our instance...
        nw->w = GGTK_WINDOW(ggtk_window_new(nw));
        if (window == NULL) {
            Log(LOGWARN, "Failed to create a GGtkWindow (topleveL)");
            gtk_widget_destroy(GTK_WIDGET(window));
            free(nw->ggc);
            free(nw);
            return NULL;
        }

        gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(nw->w));
        // Am I meant to keep a ref to the window around? Am I meant to free it?
    } else {
        nw->w = GGTK_WINDOW(ggtk_window_new(nw));
        if (nw->w == NULL) {
            Log(LOGWARN, "Failed to create a GGtkWindow (child)");
            free(nw->ggc);
            free(nw);
            return NULL;
        }

        gtk_layout_put(GTK_LAYOUT(gw->w), GTK_WIDGET(nw->w), nw->pos.x, nw->pos.y);
        gtk_widget_set_size_request(GTK_WIDGET(nw->w), nw->pos.width, nw->pos.height);
    }

    // Set background
    if (!(wattrs->mask & wam_backcol) || wattrs->background_color == COLOR_DEFAULT) {
        wattrs->background_color = gdisp->def_background;
    }
    nw->ggc->bg = wattrs->background_color;

    GGTKDrawSetWindowBackground((GWindow)nw, wattrs->background_color);

    Log(LOGDEBUG, "Window created: %p[%p][%d]", nw, nw->w, nw->is_toplevel);
    return (GWindow)nw;
}

static void GGTKDrawInit(GDisplay *gdisp) {
    Log(LOGDEBUG, "");

    FState *fs = calloc(1, sizeof(FState));
    if (fs == NULL) {
        Log(LOGERR, "FState alloc failed!");
        assert(false);
    }

    // In inches, because that's how fonts are measured
    gdisp->fontstate = fs;
    fs->res = gdisp->res;
}

static void GGTKDrawSetDefaultIcon(GWindow icon) {
}

static GWindow GGTKDrawCreateTopWindow(GDisplay *gdisp, GRect *pos, int (*eh)(GWindow gw, GEvent *), void *user_data, GWindowAttrs *gattrs) {
    Log(LOGVERBOSE, " ");
    return _GGTKDraw_CreateWindow((GGTKDisplay *) gdisp, NULL, pos, eh, user_data, gattrs);
}

static GWindow GGTKDrawCreateSubWindow(GWindow gw, GRect *pos, int (*eh)(GWindow gw, GEvent *), void *user_data, GWindowAttrs *gattrs) {
    Log(LOGVERBOSE, " ");
    return _GGTKDraw_CreateWindow(((GGTKWindow) gw)->display, (GGTKWindow) gw, pos, eh, user_data, gattrs);
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

    NULL, // GGTKDrawPushClip TO IMPLEMENT
    NULL, // GGTKDrawPopClip TO IMPLEMENT

    NULL, // GGTKDrawSetDifferenceMode TO IMPLEMENT

    NULL, // GGTKDrawClear TO IMPLEMENT
    NULL, // GGTKDrawDrawLine TO IMPLEMENT
    NULL, // GGTKDrawDrawArrow TO IMPLEMENT
    NULL, // GGTKDrawDrawRect TO IMPLEMENT
    NULL, // GGTKDrawFillRect TO IMPLEMENT
    NULL, // GGTKDrawFillRoundRect TO IMPLEMENT
    NULL, // GGTKDrawDrawEllipse TO IMPLEMENT
    NULL, // GGTKDrawFillEllipse TO IMPLEMENT
    NULL, // GGTKDrawDrawArc TO IMPLEMENT
    NULL, // GGTKDrawDrawPoly TO IMPLEMENT
    NULL, // GGTKDrawFillPoly TO IMPLEMENT
    GGTKDrawScroll,

    NULL, // GGTKDrawDrawImage TO IMPLEMENT
    NULL, // GGTKDrawTileImage - Unused function
    NULL, // GGTKDrawDrawGlyph TO IMPLEMENT
    NULL, // GGTKDrawDrawImageMagnified TO IMPLEMENT
    NULL, // GGTKDrawCopyScreenToImage - Unused function
    NULL, // GGTKDrawDrawPixmap TO IMPLEMENT
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

    NULL, // GGTKDrawGetFontMetrics TO IMPLEMENT

    NULL, // GGTKDrawHasCairo TO IMPLEMENT
    NULL, // GGTKDrawPathStartNew TO IMPLEMENT
    NULL, // GGTKDrawPathClose TO IMPLEMENT
    NULL, // GGTKDrawPathMoveTo TO IMPLEMENT
    NULL, // GGTKDrawPathLineTo TO IMPLEMENT
    NULL, // GGTKDrawPathCurveTo TO IMPLEMENT
    NULL, // GGTKDrawPathStroke TO IMPLEMENT
    NULL, // GGTKDrawPathFill TO IMPLEMENT
    NULL, // GGTKDrawPathFillAndStroke TO IMPLEMENT

    NULL, // GGTKDrawLayoutInit TO IMPLEMENT
    NULL, // GGTKDrawLayoutDraw TO IMPLEMENT
    NULL, // GGTKDrawLayoutIndexToPos TO IMPLEMENT
    NULL, // GGTKDrawLayoutXYToIndex TO IMPLEMENT
    NULL, // GGTKDrawLayoutExtents TO IMPLEMENT
    NULL, // GGTKDrawLayoutSetWidth TO IMPLEMENT
    NULL, // GGTKDrawLayoutLineCount TO IMPLEMENT
    NULL, // GGTKDrawLayoutLineStart TO IMPLEMENT
    NULL, // GGTKDrawStartNewSubPath TO IMPLEMENT
    NULL, // GGTKDrawFillRuleSetWinding TO IMPLEMENT

    NULL, // GGTKDrawDoText8 TO IMPLEMENT

    NULL, // GGTKDrawPushClipOnly TO IMPLEMENT
    NULL, // GGTKDrawClipPreserve TO IMPLEMENT
};

// Protected member functions (package-level)

GDisplay *_GGTKDraw_CreateDisplay(char *displayname, char *UNUSED(programname)) {
    GGTKDisplay *gdisp;
    GdkDisplay *display;
    GGTKWindow groot;

    LogInit();

    if (displayname == NULL) {
        display = gdk_display_get_default();
    } else {
        display = gdk_display_open(displayname);
    }

    if (display == NULL) {
        Log(LOGERR, "Could not get display");
        return NULL;
    }

    // Yuck...
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    if (monitor == NULL) {
        monitor = gdk_display_get_monitor_at_window(display, gdk_display_get_default_group(display));
        if (monitor == NULL) {
            monitor = gdk_display_get_monitor(display, 0);
            if (monitor == NULL) {
                Log(LOGERR, "Could not get any monitor");
                return NULL;
            }
        }
    }

    gdisp = (GGTKDisplay *)calloc(1, sizeof(GGTKDisplay));
    if (gdisp == NULL) {
        Log(LOGERR, "Failed to alloc GGTKDisplay");
        return NULL;
    }

    // cursors.c creates ~41.
    gdisp->cursors = g_ptr_array_sized_new(50);

    gdisp->funcs = &gtkfuncs;
    gdisp->display = display;

    // sigh, this is terrible, really it should get this wrt. each window
    PangoContext* ctx = gdk_pango_context_get_for_display(display);
    gdisp->res = pango_cairo_context_get_resolution(ctx);
    if (gdisp->res <= 0) {
        Log(LOGWARN, "Failed to get default DPI, assuming 96");
        gdisp->res = 96;
    }
    g_object_unref(ctx);

    gdisp->scale_screen_by = 1; //Does nothing
    gdisp->bs.double_time = 200;
    gdisp->bs.double_wiggle = 3;

    bool tbf = false, mxc = false;
    GResStruct res[] = {
        {.resname = "MultiClickTime", .type = rt_int, .val = &gdisp->bs.double_time},
        {.resname = "MultiClickWiggle", .type = rt_int, .val = &gdisp->bs.double_wiggle},
        {.resname = "TwoButtonFixup", .type = rt_bool, .val = &tbf},
        {.resname = "MacOSXCmd", .type = rt_bool, .val = &mxc},
        {.resname = NULL},
    };
    GResourceFind(res, NULL);
    gdisp->twobmouse_win = tbf;
    gdisp->macosx_cmd = mxc;

    groot = (GGTKWindow)calloc(1, sizeof(struct ggtkwindow));
    if (groot == NULL) {
        Log(LOGERR, "Failedto alloc root GGTKWindow");
        free(gdisp);
        return NULL;
    }

    gdisp->groot = groot;
    groot->ggc = _GGTKDraw_NewGGC();
    groot->display = gdisp;
    groot->w = NULL; // Can't set this to anything meaningful...

    // More yuck...
    GdkRectangle geom;
    gdk_monitor_get_geometry(monitor, &geom);
    groot->pos.width = geom.width;
    groot->pos.height = geom.height;
    groot->is_toplevel = true;
    groot->is_visible = true;

    gdisp->def_background = GResourceFindColor("Background", COLOR_CREATE(0xf5, 0xff, 0xfa));
    gdisp->def_foreground = GResourceFindColor("Foreground", COLOR_CREATE(0x00, 0x00, 0x00));
    if (GResourceFindBool("Synchronize", false)) {
        gdk_display_sync(gdisp->display);
    }

    (gdisp->funcs->init)((GDisplay *) gdisp);

    // event handling?

    _GDraw_InitError((GDisplay *) gdisp);

    //DEBUG
    if (getenv("GGTK_DEBUG")) {
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
            gdk_window_set_debug_updates(true);
        G_GNUC_END_IGNORE_DEPRECATIONS
    }
    return (GDisplay *)gdisp;
}

void _GGTKDraw_DestroyDisplay(GDisplay *disp) {
    Log(LOGDEBUG, "");
    return;
}

// GGtkWindow definition
struct _GGtkWindow
{
    GtkLayout parent_instance;

    // private data
    GGTKWindow gw;
    GdkRGBA background_color;

    cairo_surface_t *offscreen_surface;
    cairo_t* offscreen_context;
    cairo_region_t* dirty_regions;
    int offscreen_width;
    int offscreen_height;

    bool disposed;
};

static void ggtk_window_dispose(GObject *gobject)
{
    GGtkWindow *ggw = GGTK_WINDOW(gobject);
    ggw->disposed = true;

    if (ggw->offscreen_context) {
        cairo_destroy(ggw->offscreen_context);
        ggw->offscreen_context = NULL;
    }
    if (ggw->dirty_regions) {
        cairo_region_destroy(ggw->dirty_regions);
        ggw->dirty_regions = NULL;
    }
    if (ggw->offscreen_surface) {
        cairo_surface_destroy(ggw->offscreen_surface);
        ggw->offscreen_surface = NULL;
    }
    ggw->offscreen_width = 0;
    ggw->offscreen_height = 0;

    // Invoke close event on window?
}

static void ggtk_window_class_init(GGtkWindowClass *ggwc)
{
    GObjectClass *object_class = G_OBJECT_CLASS(ggwc);
    object_class->dispose = ggtk_window_dispose;
}

static void ggtk_window_init(GGtkWindow *ggw)
{
    (void)ggw;
}

static gboolean ggtk_window_draw_callback(GtkWidget* widget, cairo_t* cr, G_GNUC_UNUSED gpointer data)
{
    GGtkWindow *ggw = GGTK_WINDOW(widget);

    bool repaint_all = false;
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    if (ggw->offscreen_context) {
        cairo_destroy(ggw->offscreen_context);
        ggw->offscreen_context = NULL;
    }

    if (!ggw->offscreen_surface || ggw->offscreen_width != width || ggw->offscreen_height != height) {
        GdkWindow* window = gtk_widget_get_window(widget);

        cairo_surface_destroy(ggw->offscreen_surface);
        if (ggw->dirty_regions) {
            cairo_region_destroy(ggw->dirty_regions);
            ggw->dirty_regions = NULL;
        }
        ggw->offscreen_surface = gdk_window_create_similar_surface(window, CAIRO_CONTENT_COLOR, width, height);
        ggw->offscreen_width = width;
        ggw->offscreen_height = height;
        repaint_all = true;
    }

    if (repaint_all || ggw->dirty_regions) {
        ggw->offscreen_context = cairo_create(ggw->offscreen_surface);
        if (ggw->dirty_regions) {
            cairo_rectangle_int_t area;
            int num_rectangles = cairo_region_num_rectangles(ggw->dirty_regions);

            for (int i = 0; i < num_rectangles; ++i) {
                cairo_region_get_rectangle(ggw->dirty_regions, i, &area);
                cairo_rectangle(ggw->offscreen_context, area.x, area.y, area.width, area.height);
                cairo_clip(ggw->offscreen_context);
            }
            cairo_region_destroy(ggw->dirty_regions);
            ggw->dirty_regions = NULL;
        }

        cairo_set_source_rgba(ggw->offscreen_context,
            ggw->background_color.red, ggw->background_color.green, ggw->background_color.red, ggw->background_color.alpha);
        cairo_rectangle(ggw->offscreen_context, 0, 0, width, height);
        cairo_fill(ggw->offscreen_context);

        // Now call the GDraw expose event handler here

        cairo_destroy(ggw->offscreen_context);
        ggw->offscreen_context = NULL;
    }

    // Now paint the offscreen surface
    cairo_set_source_surface(cr, ggw->offscreen_surface, 0, 0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_paint(cr);

    return false;
}

GtkWidget* ggtk_window_new(GGTKWindow gw)
{
    GGtkWindow *ggw = GGTK_WINDOW(g_object_new(GGTK_TYPE_WINDOW, NULL));
    g_return_val_if_fail(ggw != NULL, NULL);

    // I don't know if this is the correct thing to do, how is this meant to
    // get initialised in the _init method?!
    ggw->gw = gw;

    g_signal_connect(ggw, "draw", G_CALLBACK(ggtk_window_draw_callback), NULL);

    return GTK_WIDGET(ggw);
}

GGTKWindow ggtk_window_get_base(GGtkWindow *ggw)
{
    g_return_val_if_fail(ggw != NULL, NULL);
    return ggw->gw;
}

void ggtk_window_set_background(GGtkWindow *ggw, GdkRGBA col)
{
    ggw->background_color = col;
    gtk_widget_queue_draw(GTK_WIDGET(ggw));
}

cairo_t* ggtk_window_get_cairo_context(GGtkWindow *ggw)
{
    if (ggw->offscreen_context) {
        return ggw->offscreen_context;
    } else if (!ggw->offscreen_surface) {
        return NULL;
    }

    ggw->offscreen_context = cairo_create(ggw->offscreen_surface);
    gtk_widget_queue_draw(GTK_WIDGET(ggw));

    return ggw->offscreen_context;
}

void ggtk_window_request_expose(GGtkWindow *ggw, cairo_rectangle_int_t *area)
{
    if (area) {
        if (ggw->dirty_regions) {
            cairo_region_union_rectangle(ggw->dirty_regions, area);
        } else {
            ggw->dirty_regions = cairo_region_create_rectangle(area);
        }
        gtk_widget_queue_draw_area(GTK_WIDGET(ggw), area->x, area->y, area->width, area->height);
        return;
    }
    gtk_widget_queue_draw(GTK_WIDGET(ggw));
}


G_DEFINE_TYPE(GGtkWindow, ggtk_window, GTK_TYPE_LAYOUT)

// End GGtkWindow definition

#endif // FONTFORGE_CAN_USE_GTK
