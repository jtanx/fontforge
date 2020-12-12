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
 *  \file  ggtkdrawP.h
 *  \brief Private header file for the GTK backend.
 */

#ifndef FONTFORGE_GGTKDRAWP_H
#define FONTFORGE_GGTKDRAWP_H

#include <fontforge-config.h>

#ifdef FONTFORGE_CAN_USE_GTK

#include "ffgtk.h"
#include "gdrawP.h"
#include "ggdkdrawloggerP.h"

#if (((GDK_MAJOR_VERSION == 3) && (GDK_MINOR_VERSION >= 22)) || (GDK_MAJOR_VERSION > 3))
#    define GGTKDRAW_GDK_3_22
#endif

#if (((GDK_MAJOR_VERSION == 3) && (GDK_MINOR_VERSION >= 20)) || (GDK_MAJOR_VERSION > 3))
#    define GGTKDRAW_GDK_3_20
#endif

#define GGTKDRAW_ADDREF(x) do { \
    assert((x)->reference_count >= 0); \
    (x)->reference_count++; \
} while(0)

#define GGTKDRAW_DECREF(x,y) do { \
    assert((x)->reference_count > 0); \
    (x)->reference_count--; \
    if ((x)->reference_count == 0) { \
        y(x); \
    } \
} while(0)


typedef struct ggtkwindow *GGTKWindow;

// GGtkWindow GObject declaration

#define GList  GList_Glib
#define GGTK_TYPE_WINDOW ggtk_window_get_type()
G_DECLARE_FINAL_TYPE(GGtkWindow, ggtk_window, GGTK, WINDOW, GtkLayout)
#undef GList

struct _GGtkWindowClass
{
	GtkLayoutClass parent_class;
	
	// virtual methods here, no pad because screw ABI compat, it's internal
};

typedef void (*GGtkWindowEventHandler) (GGtkWindow *ggw, GdkEvent *event);

GtkWidget* ggtk_window_new(GGTKWindow gw, GGtkWindowEventHandler eh);
GGTKWindow ggtk_window_get_base(GGtkWindow *ggw);
GtkWindow* ggtk_window_get_window(GGtkWindow *ggw);
void ggtk_window_set_background(GGtkWindow *ggw, GdkRGBA *col);
cairo_t* ggtk_window_get_cairo_context(GGtkWindow *ggw);
void ggtk_window_request_expose(GGtkWindow *ggw, cairo_rectangle_int_t *area);
const char *ggtk_window_get_title(GGtkWindow *ggw);
PangoLayout *ggtk_window_get_pango_layout(GGtkWindow *ggw);

// end GGtkWindow declaration

// Really GTimer should be opaque...
typedef struct ggtktimer { // :GTimer
    long time_sec;             // longs not int32s to match timeval
    long time_usec;
    int32 repeat_time;          // 0 == one shot (run once)
    GWindow owner;
    void *userdata;
    struct gtimer *next;       // Unused in favour of a GLib list
    unsigned int active: 1;
    // Extensions below
    unsigned int stopped: 1;
    unsigned int has_differing_repeat_time: 1;
    int reference_count;
    guint glib_timeout_id;
} GGTKTimer;

typedef struct ggtkbuttonstate {
    uint32 last_press_time;
    GGTKWindow release_w;
    int16 release_x, release_y;
    int16 release_button;
    int16 cur_click;
    int16 double_time;		// max milliseconds between release & click
    int16 double_wiggle;	// max pixel wiggle allowed between release&click
} GGTKButtonState;

typedef struct ggtkkeystate {
    GdkEventType type;
    guint state;
    guint keyval;
} GGTKKeyState;

typedef struct ggtkdisplay { /* :GDisplay */
    // Inherit GDisplay start
    struct displayfuncs *funcs;
    struct font_state *fontstate;
    int16 res;
    GGTKWindow groot;
    Color def_background, def_foreground;
    uint16 mykey_state;
    uint16 mykey_keysym;
    uint16 mykey_mask;
    unsigned int mykeybuild : 1;
    unsigned int default_visual : 1;
    unsigned int do_dithering : 1;
    unsigned int focusfollowsmouse : 1;
    unsigned int top_offsets_set : 1;
    unsigned int wm_breaks_raiseabove : 1;
    unsigned int wm_raiseabove_tested : 1;
    unsigned int endian_mismatch : 1;
    unsigned int macosx_cmd : 1;           /* if set then map state=0x20 to control */
    unsigned int twobmouse_win : 1;        /* if set then map state=0x40 to mouse button 2 */
    unsigned int devicesinit : 1;          /* the devices structure has been initialized. Else call XListInputDevices */
    unsigned int expecting_core_event : 1; /* when we move an input extension device we generally get two events, one for the device, one later for the core device. eat the core event */
    unsigned int has_xkb : 1;              /* we were able to initialize the XKB extension */
    unsigned int supports_alpha_images : 1;
    unsigned int supports_alpha_windows : 1;
    int err_flag;
    char *err_report;
    // Inherit GDisplay end
    
    unsigned int is_space_pressed: 1; // Used for GGTKDrawKeyState. We cheat!
    unsigned int is_dying: 1; // Flag to indicate if we're cleaning up the display.
    
    int top_window_count;
    GPtrArray *cursors; // List of cursors that the user made.
    GList_Glib *timers; //List of GGTKTimer's
    
    GGTKButtonState bs;
    GGTKKeyState ks;
    GGTKWindow default_icon;

    GdkDisplay *display;
    PangoContext *default_pango_context;
    PangoLayout *default_pango_layout;
} GGTKDisplay;

struct ggtkwindow { /* :GWindow */
    // Inherit GWindow start
    GGC *ggc;
    GGTKDisplay *display;
    int (*eh)(GWindow, GEvent *);
    GRect pos;
    struct ggtkwindow *parent;
    void *user_data;
    void *widget_data;
    GGtkWindow *w;
    unsigned int is_visible : 1;
    unsigned int is_pixmap : 1;
    unsigned int is_toplevel : 1;
    unsigned int visible_request : 1;
    unsigned int is_dying : 1;
    unsigned int is_popup : 1;
    unsigned int disable_expose_requests : 1;
    unsigned int usecairo : 1;
    char *window_type_name;
    // Inherit GWindow end

    unsigned int is_dlg: 1;
    unsigned int not_restricted: 1;
    unsigned int restrict_input_to_me: 1;/* for dialogs, no input outside of dlg */
    unsigned int istransient: 1;	/* has transient for hint set */
    unsigned int isverytransient: 1;
    unsigned int is_cleaning_up: 1; //Are we running cleanup?
    
    int reference_count; // Knowing when to destroy is tricky...
    GCursor current_cursor;

    cairo_surface_t *pixmap_surface;
    cairo_t *pixmap_context;
    PangoLayout *pixmap_layout;
};

// Functions in ggtkcdraw.c

bool _GGTKDraw_InitPangoCairo(GGTKWindow gw);
void _GGTKDraw_CleanupAutoPaint(GGTKDisplay *gdisp);

void GGTKDrawPushClip(GWindow w, GRect *rct, GRect *old);
void GGTKDrawPopClip(GWindow gw, GRect *old);
void GGTKDrawSetDifferenceMode(GWindow gw);
void GGTKDrawClear(GWindow gw, GRect *rect);
void GGTKDrawDrawLine(GWindow w, int32 x, int32 y, int32 xend, int32 yend, Color col);
void GGTKDrawDrawArrow(GWindow gw, int32 x, int32 y, int32 xend, int32 yend, int16 arrows, Color col);
void GGTKDrawDrawRect(GWindow gw, GRect *rect, Color col);
void GGTKDrawFillRect(GWindow gw, GRect *rect, Color col);
void GGTKDrawFillRoundRect(GWindow gw, GRect *rect, int radius, Color col);
void GGTKDrawDrawEllipse(GWindow gw, GRect *rect, Color col);
void GGTKDrawFillEllipse(GWindow gw, GRect *rect, Color col);
void GGTKDrawDrawArc(GWindow gw, GRect *rect, int32 sangle, int32 eangle, Color col);
void GGTKDrawDrawPoly(GWindow gw, GPoint *pts, int16 cnt, Color col);
void GGTKDrawFillPoly(GWindow gw, GPoint *pts, int16 cnt, Color col);
void GGTKDrawDrawImage(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y);
void GGTKDrawDrawGlyph(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y);
void GGTKDrawDrawImageMagnified(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y, int32 width, int32 height);

void GGTKDrawDrawPixmap(GWindow gw1, GWindow gw2, GRect *src, int32 x, int32 y);

enum gcairo_flags GGTKDrawHasCairo(GWindow w);

void GGTKDrawPathStartNew(GWindow w);
void GGTKDrawPathClose(GWindow w);
void GGTKDrawPathMoveTo(GWindow w, double x, double y);
void GGTKDrawPathLineTo(GWindow w, double x, double y);
void GGTKDrawPathCurveTo(GWindow w, double cx1, double cy1, double cx2, double cy2, double x, double y);
void GGTKDrawPathStroke(GWindow w, Color col);
void GGTKDrawPathFill(GWindow w, Color col);
void GGTKDrawPathFillAndStroke(GWindow w, Color fillcol, Color strokecol);
void GGTKDrawStartNewSubPath(GWindow w);
int GGTKDrawFillRuleSetWinding(GWindow w);
int GGTKDrawDoText8(GWindow w, int32 x, int32 y, const char *text, int32 cnt, Color col, enum text_funcs drawit, struct tf_arg *arg);

void GGTKDrawPushClipOnly(GWindow w);
void GGTKDrawClipPreserve(GWindow w);

void GGTKDrawGetFontMetrics(GWindow gw, GFont *fi, int *as, int *ds, int *ld);
void GGTKDrawLayoutInit(GWindow w, char *text, int cnt, GFont *fi);
void GGTKDrawLayoutDraw(GWindow w, int32 x, int32 y, Color fg);
void GGTKDrawLayoutIndexToPos(GWindow w, int index, GRect *pos);
int GGTKDrawLayoutXYToIndex(GWindow w, int x, int y);
void GGTKDrawLayoutExtents(GWindow w, GRect *size);
void GGTKDrawLayoutSetWidth(GWindow w, int width);
int GGTKDrawLayoutLineCount(GWindow w);
int GGTKDrawLayoutLineStart(GWindow w, int l);

// END functions in ggtkcdraw.c

#endif // FONTFORGE_CAN_USE_GTK

#endif /* FONTFORGE_GGTKDRAWP_H */
