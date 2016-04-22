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

typedef struct ggdkwindow *GGDKWindow;

typedef struct ggdkdisplay /* :GDisplay */ {
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
    unsigned int expecting_core_event: 1;/* when we move an input extension device we generally get two events, one for the device, one later for the core device. eat the core event */
    unsigned int has_xkb: 1;        /* we were able to initialize the XKB extension */
    unsigned int supports_alpha_images: 1;
    unsigned int supports_alpha_windows: 1;
    int err_flag;
    char * err_report;
    // Inherit GDisplay end

    GdkDisplay *display;
    GdkScreen  *screen;
    GdkWindow  *root;

    PangoContext *pangoc_context;
} GGDKDisplay;

struct ggdkwindow /* :GWindow */ {
    // Inherit GWindow start
    GGC *ggc;
    GGDKDisplay *display;
    int (*eh)(GWindow,GEvent *);
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

#endif // _GGDKDRAWP_H