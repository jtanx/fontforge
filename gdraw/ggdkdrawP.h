/**
 *  \file  ggdkdrawP.h
 *  \brief Private header file for the GDK backend.
 */

#ifndef _GGDKDRAWP_H
#define _GGDKDRAWP_H

#include "gdrawP.h"

// Le sigh
#define GTimer GTimer_GTK
#define GList  GList_Glib
#include <gdk/gdk.h>
#undef GTimer
#undef GList

#include "fontP.h"

#define GColorToGDK(col) COLOR_RED(col)/255., COLOR_GREEN(col)/255., COLOR_BLUE(col)/255.

typedef struct ggdkwindow *GGDKWindow;

typedef struct ggdkdisplay { /* :GDisplay */
    // Inherit GDisplay start
    struct displayfuncs *funcs;
    void *semaphore;
    struct font_state *fontstate;
    int16 res;
    int16 scale_screen_by;
    GGDKWindow groot;
    Color def_background, def_foreground;
    uint16 mykey_state;
    uint16 mykey_keysym;
    uint16 mykey_mask;
    fd_callback_t fd_callbacks[ gdisplay_fd_callbacks_size ];
    int fd_callbacks_last;
    unsigned int mykeybuild: 1;
    unsigned int default_visual: 1;
    unsigned int do_dithering: 1;
    unsigned int focusfollowsmouse: 1;
    unsigned int top_offsets_set: 1;
    unsigned int wm_breaks_raiseabove: 1;
    unsigned int wm_raiseabove_tested: 1;
    unsigned int endian_mismatch: 1;
    unsigned int macosx_cmd: 1;     /* if set then map state=0x20 to control */
    unsigned int twobmouse_win: 1;  /* if set then map state=0x40 to mouse button 2 */
    unsigned int devicesinit: 1;    /* the devices structure has been initialized. Else call XListInputDevices */
    unsigned int expecting_core_event: 1; /* when we move an input extension device we generally get two events, one for the device, one later for the core device. eat the core event */
    unsigned int has_xkb: 1;        /* we were able to initialize the XKB extension */
    unsigned int supports_alpha_images: 1;
    unsigned int supports_alpha_windows: 1;
    int err_flag;
    char *err_report;
    // Inherit GDisplay end

    GdkDisplay *display;
    GdkScreen  *screen;
    GdkWindow  *root;

    PangoContext *pangoc_context;
} GGDKDisplay;

struct ggdkwindow { /* :GWindow */
    // Inherit GWindow start
    GGC *ggc;
    GGDKDisplay *display;
    int (*eh)(GWindow, GEvent *);
    GRect pos;
    struct ggdkwindow *parent;
    void *user_data;
    void *widget_data;
    GdkWindow *w;
    unsigned int is_visible: 1;
    unsigned int is_pixmap: 1;
    unsigned int is_toplevel: 1;
    unsigned int visible_request: 1;
    unsigned int is_dying: 1;
    unsigned int is_popup: 1;
    unsigned int disable_expose_requests: 1;
    unsigned int usecairo: 1;
    // Inherit GWindow end
    unsigned int is_dlg: 1;
    unsigned int not_restricted: 1;
    unsigned int was_positioned: 1;

    char *window_title;

    cairo_surface_t *cs;
    cairo_t *cc;
    PangoLayout *pango_layout;
};

// Functions in ggdkcdraw.c

bool _GGDKDraw_InitPangoCairo(GGDKWindow gw);
PangoFontDescription *_GGDKDraw_configfont(GWindow w, GFont *font);
void _GGDKDraw_MyCairoRenderLayout(cairo_t *cc, Color fg, PangoLayout *layout, int x, int y);
void _GGDKDraw_PushClip(GGDKWindow gw);
void _GGDKDraw_PopClip(GGDKWindow gw);

void GGDKDrawPushClip(GWindow w, GRect *rct, GRect *old);
void GGDKDrawPopClip(GWindow gw, GRect *old);
void GGDKDrawSetDifferenceMode(GWindow gw);
void GGDKDrawClear(GWindow gw, GRect *rect);
void GGDKDrawDrawLine(GWindow w, int32 x, int32 y, int32 xend, int32 yend, Color col);
void GGDKDrawDrawArrow(GWindow gw, int32 x, int32 y, int32 xend, int32 yend, Color col);
void GGDKDrawDrawRect(GWindow gw, GRect *rect, Color col);
void GGDKDrawFillRect(GWindow gw, GRect *rect, Color col);
void GGDKDrawFillRoundRect(GWindow gw, GRect *rect, int radius, Color col);
void GGDKDrawDrawEllipse(GWindow gw, GRect *rect, Color col);
void GGDKDrawFillEllipse(GWindow gw, GRect *rect, Color col);
void GGDKDrawDrawArc(GWindow gw, GRect *rect, int32 sangle, int32 eangle, Color col);
void GGDKDrawDrawPoly(GWindow gw, GPoint *pts, int16 cnt, Color col);
void GGDKDrawFillPoly(GWindow gw, GPoint *pts, int16 cnt, Color col);
void GGDKDrawScroll(GWindow gw, GRect *rect, int32 hor, int32 vert);
void GGDKDrawDrawImage(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y);
void GGDKDrawTileImage(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y);
void GGDKDrawDrawGlyph(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y);
void GGDKDrawDrawImageMagnified(GWindow gw, GImage *gimg, GRect *src, int32 x, int32 y, int32 width, int32 height);

GImage *GGDKDrawCopyScreenToImage(GWindow gw, GRect *rect);
void    GGDKDrawDrawPixmap(GWindow gw1, GWindow gw2, GRect *src, int32 x, int32 y);
void    GGDKDrawTilePixmap(GWindow gw1, GWindow gw2, GRect *src, int32 x, int32 y);

enum gcairo_flags GGDKDrawHasCairo(GWindow w);

void GGDKDrawPathStartNew(GWindow w);
void GGDKDrawPathClose(GWindow w);
void GGDKDrawPathMoveTo(GWindow w, double x, double y);
void GGDKDrawPathLineTo(GWindow w, double x, double y);
void GGDKDrawPathCurveTo(GWindow w, double cx1, double cy1, double cx2, double cy2, double x, double y);
void GGDKDrawPathStroke(GWindow w, Color col);
void GGDKDrawPathFill(GWindow w, Color col);
void GGDKDrawPathFillAndStroke(GWindow w, Color fillcol, Color strokecol);
void GGDKDrawStartNewSubPath(GWindow w);
int  GGDKDrawFillRuleSetWinding(GWindow w);
int  GGDKDrawDoText8(GWindow w, int32 x, int32 y, const char *text, int32 cnt, Color col, enum text_funcs drawit, struct tf_arg *arg);

void GGDKDrawPushClipOnly(GWindow w);
void GGDKDrawClipPreserve(GWindow w);

void GGDKDrawGetFontMetrics(GWindow gw, GFont *fi, int *as, int *ds, int *ld);
void GGDKDrawLayoutInit(GWindow w, char *text, int cnt, GFont *fi);
void GGDKDrawLayoutDraw(GWindow w, int32 x, int32 y, Color fg);
void GGDKDrawLayoutIndexToPos(GWindow w, int index, GRect *pos);
int  GGDKDrawLayoutXYToIndex(GWindow w, int x, int y);
void GGDKDrawLayoutExtents(GWindow w, GRect *size);
void GGDKDrawLayoutSetWidth(GWindow w, int width);
int  GGDKDrawLayoutLineCount(GWindow w);
int  GGDKDrawLayoutLineStart(GWindow w, int l);

// END functions in ggdkdraw.c

#endif // _GGDKDRAWP_H