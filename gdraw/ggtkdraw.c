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
static void GGTKDrawCancelTimer(GTimer *timer);
static void GGTKDrawDestroyWindow(GWindow w);
static void GGTKDrawSetCursor(GWindow w, GCursor gcursor);
static void GGTKDrawSetTransientFor(GWindow transient, GWindow owner);
static void GGTKDrawSetWindowBackground(GWindow w, Color gcol);
static void GGTKDrawPostEvent(GEvent *e);


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

static int16 _GGTKDraw_GdkModifierToKsm(GdkModifierType mask) {
    int16 state = 0;
    //Translate from mask to X11 state
    if (mask & GDK_SHIFT_MASK) {
        state |= ksm_shift;
    }
    if (mask & GDK_LOCK_MASK) {
        state |= ksm_capslock;
    }
    if (mask & GDK_CONTROL_MASK) {
        state |= ksm_control;
    }
    if (mask & GDK_MOD1_MASK) { //Guess
        state |= ksm_meta;
    }
    if (mask & GDK_MOD2_MASK) { //Guess
        state |= ksm_cmdmacosx;
    }
    if (mask & GDK_SUPER_MASK) {
        state |= ksm_super;
    }
    if (mask & GDK_HYPER_MASK) {
        state |= ksm_hyper;
    }
    //ksm_option?
    if (mask & GDK_BUTTON1_MASK) {
        state |= ksm_button1;
    }
    if (mask & GDK_BUTTON2_MASK) {
        state |= ksm_button2;
    }
    if (mask & GDK_BUTTON3_MASK) {
        state |= ksm_button3;
    }
    if (mask & GDK_BUTTON4_MASK) {
        state |= ksm_button4;
    }
    if (mask & GDK_BUTTON5_MASK) {
        state |= ksm_button5;
    }

    return state;
}

static int _GGTKDraw_WindowOrParentsDying(GGTKWindow gw) {
    while (gw != NULL) {
        if (gw->is_dying) {
            return true;
        }
        if (gw->is_toplevel) {
            return false;
        }
        gw = gw->parent;
    }
    return false;
}

static GdkDevice *_GGTKDraw_GetPointer(GdkDisplay *display) {
#ifdef GGTKDRAW_GDK_3_20
    GdkSeat *seat = gdk_display_get_default_seat(display);
    if (seat == NULL) {
        return NULL;
    }

    return gdk_seat_get_pointer(seat);
#else
    GdkDeviceManager *manager = gdk_display_get_device_manager(display);
    if (manager == NULL) {
        return NULL;
    }

    return gdk_device_manager_get_client_pointer(manager);
#endif
}

static gboolean _GGTKDraw_OnWindowDestroyed(gpointer data) {
    GGTKWindow gw = (GGTKWindow)data;
    Log(LOGINFO, "Destroying: %p", gw);
    if (gw->is_cleaning_up) {
        return false;
    }
    gw->is_cleaning_up = true; // We're in the process of destroying it.
    
    if (gw->is_pixmap) {
        if (gw->pixmap_layout) {
            g_object_unref(gw->pixmap_layout);
            gw->pixmap_layout = NULL;
        }
        if (gw->pixmap_context) {
            cairo_destroy(gw->pixmap_context);
            gw->pixmap_context = NULL;
        }
        if (gw->pixmap_surface) {
            cairo_surface_destroy(gw->pixmap_surface);
            gw->pixmap_surface = NULL;
        }
    } else {
        if (gw != gw->display->groot) {
            if (gw->is_toplevel) {
                // May need to manually disconnect the signal too?
                gtk_widget_destroy(GTK_WIDGET(ggtk_window_get_window(gw->w)));
            } else {
                gtk_widget_destroy(GTK_WIDGET(gw->w));
            }
        }

        // Signal that it has been destroyed - only if we're not cleaning up the display
        if (!gw->display->is_dying) {
            struct gevent die = {0};
            die.w = (GWindow)gw;
            die.native_window = gw->w;
            die.type = et_destroy;
            GGTKDRAW_ADDREF(gw); // temporarily increase it again, so performing this doesn't destroy us agian
            GGTKDrawPostEvent(&die);
            gw->reference_count--;
        }

        // Remove all relevant timers that haven't been cleaned up by the user
        // Note: We do not free the GTimer struct as the user may then call DestroyTimer themselves...
        GList_Glib *ent = gw->display->timers;
        while (ent != NULL) {
            GList_Glib *next = ent->next;
            GGTKTimer *timer = (GGTKTimer *)ent->data;
            if (timer->owner == (GWindow)gw) {
                //Since we update the timer list ourselves, don't all GGTKDrawCancelTimer.
                Log(LOGDEBUG, "Unstopped timer on window destroy!!! %p -> %p", gw, timer);
                timer->active = false;
                timer->stopped = true;
                g_source_remove(timer->glib_timeout_id);
                gw->display->timers = g_list_delete_link(gw->display->timers, ent);
            }
            ent = next;
        }

        // Decrement the toplevel window count
        if (gw->display->groot == gw->parent && !gw->is_dlg) {
            gw->display->top_window_count--;
        }

        Log(LOGDEBUG, "Window destroyed: %p[%p][%d]", gw, gw->w, gw->is_toplevel);
        if (gw != gw->display->groot) {
            // Unreference our reference to the window
            g_object_unref(G_OBJECT(gw->w));
        }
    }

    free(gw->ggc);
    free(gw);
    return false;
}

// FF expects the destroy call to happen asynchronously to
// the actual GGTKDrawDestroyWindow call. So we add it to the queue...
static void _GGTKDraw_InitiateWindowDestroy(GGTKWindow gw) {
    Log(LOGINFO, "Initiating destroy for %p", gw);
    if (gw->is_pixmap) {
        _GGTKDraw_OnWindowDestroyed(gw);
    } else if (!gw->is_cleaning_up) { // Check for nested call - if we're already being destroyed.
        g_timeout_add(10, _GGTKDraw_OnWindowDestroyed, gw);
    }
}

static void _GGTKDraw_OnTimerDestroyed(GGTKTimer *timer) {
    GGTKDisplay *gdisp = ((GGTKWindow)(timer->owner))->display;

    // We may stop the timer without destroying it -
    // e.g. if the user doesn't cancel the timer before a window is destroyed.
    if (!timer->stopped) {
        g_source_remove(timer->glib_timeout_id);
    }
    gdisp->timers = g_list_remove(gdisp->timers, timer);
    free(timer);
}

static void _GGTKDraw_CallEHChecked(GGTKWindow gw, GEvent *event, int (*eh)(GWindow gw, GEvent *)) {
    if (eh) {
		// Increment reference counter
        GGTKDRAW_ADDREF(gw);
        (eh)((GWindow)gw, event);
        // Decrement reference counter
        GGTKDRAW_DECREF(gw, _GGTKDraw_InitiateWindowDestroy);
    }
}

static gboolean _GGTKDraw_ProcessTimerEvent(gpointer user_data) {
    GGTKTimer *timer = (GGTKTimer *)user_data;
    GEvent e = {0};
    bool ret = true;

    if (!timer->active || _GGTKDraw_WindowOrParentsDying((GGTKWindow)timer->owner)) {
        timer->active = false;
        timer->stopped = true;
        return false;
    }

    e.type = et_timer;
    e.w = timer->owner;
    e.native_window = timer->owner->native_window;
    e.u.timer.timer = (GTimer *)timer;
    e.u.timer.userdata = timer->userdata;

    GGTKDRAW_ADDREF(timer);
    _GGTKDraw_CallEHChecked((GGTKWindow)timer->owner, &e, timer->owner->eh);
    if (timer->active) {
        if (timer->repeat_time == 0) {
            timer->stopped = true; // Since we return false, this timer is no longer valid.
            GGTKDrawCancelTimer((GTimer *)timer);
            ret = false;
        } else if (timer->has_differing_repeat_time) {
            timer->has_differing_repeat_time = false;
            timer->glib_timeout_id = g_timeout_add(timer->repeat_time, _GGTKDraw_ProcessTimerEvent, timer);
            ret = false;
        }
    }

    GGTKDRAW_DECREF(timer, _GGTKDraw_OnTimerDestroyed);
    return ret;
}

static void _GGTKDraw_DispatchEvent(GGtkWindow *ggw, GdkEvent *event) {
    static int request_id = 0;
    struct gevent gevent = {0};

	GGTKWindow gw = ggtk_window_get_base(ggw);
	GGTKDisplay *gdisp = gw->display;
	assert(gw->w == ggw);

    Log(LOGVERBOSE, "[%d] Received event %d(%s) %p", request_id++, event->type, GdkEventName(event->type), ggw);
    //fflush(stderr);

    gevent.w = (GWindow)gw;
    gevent.native_window = (void *)gw->w;
    gevent.type = et_noevent;

    switch (event->type) {
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE: { // OK
            GdkEventKey *key = (GdkEventKey *)event;
            gevent.type = event->type == GDK_KEY_PRESS ? et_char : et_charup;
            gevent.u.chr.state = _GGTKDraw_GdkModifierToKsm(((GdkEventKey *)event)->state);
            gevent.u.chr.autorepeat =
                event->type    == GDK_KEY_PRESS &&
                gdisp->ks.type == GDK_KEY_PRESS &&
                key->keyval    == gdisp->ks.keyval &&
                key->state     == gdisp->ks.state;
            // Mumble mumble Mac
            //if ((event->xkey.state & ksm_option) && gdisp->macosx_cmd) {
            //    gevent.u.chr.state |= ksm_meta;
            //}

            if (gevent.u.chr.autorepeat && GKeysymIsModifier(key->keyval)) {
                gevent.type = et_noevent;
            } else {
                if (key->keyval == GDK_KEY_space) {
                    gw->display->is_space_pressed = event->type == GDK_KEY_PRESS;
                }

                gevent.u.chr.keysym = key->keyval;
                gevent.u.chr.chars[0] = gdk_keyval_to_unicode(key->keyval);
            }

            gdisp->ks.type   = key->type;
            gdisp->ks.keyval = key->keyval;
            gdisp->ks.state  = key->state;
        }
        break;
        case GDK_MOTION_NOTIFY: { // OK
            GdkEventMotion *evt = (GdkEventMotion *)event;
            gevent.type = et_mousemove;
            gevent.u.mouse.state = _GGTKDraw_GdkModifierToKsm(evt->state);
            gevent.u.mouse.x = evt->x;
            gevent.u.mouse.y = evt->y;
            //Log(LOGDEBUG, "Motion: [%f %f]", evt->x, evt->y);
        }
        break;
        case GDK_SCROLL: { //Synthesize a button press // OK
            GdkEventScroll *evt = (GdkEventScroll *)event;
            gevent.u.mouse.state = _GGTKDraw_GdkModifierToKsm(evt->state);
            gevent.u.mouse.x = evt->x;
            gevent.u.mouse.y = evt->y;
            gevent.u.mouse.clicks = gdisp->bs.cur_click = 1;
            gevent.type = et_mousedown;

            switch (evt->direction) {
                case GDK_SCROLL_UP:
                    gevent.u.mouse.button = 4;
                    break;
                case GDK_SCROLL_DOWN:
                    gevent.u.mouse.button = 5;
                    break;
                case GDK_SCROLL_LEFT:
                    gevent.u.mouse.button = 6;
                    break;
                case GDK_SCROLL_RIGHT:
                    gevent.u.mouse.button = 7;
                    break;
                case GDK_SCROLL_SMOOTH: {
                    // Under GTK, I seem to be only able to get these smooth scroll events...
                    double dx, dy;
                    if (gdk_event_get_scroll_deltas(event, &dx, &dy)) {
                        if (dy < 0) {
                            gevent.u.mouse.button = 4;
                        } else if (dy > 0) {
                            gevent.u.mouse.button = 5;
                        } else if (dx < 0) {
                            gevent.u.mouse.button = 6;
                        } else if (dx > 0) {
                            gevent.u.mouse.button = 7;
                        }
                    }
                } break;
                default:
                    break;
            }
            // We need to simulate two events... I think.
            if (gevent.u.mouse.button) {
                GGTKDrawPostEvent(&gevent);
                gevent.type = et_mouseup;
            }
        }
        break;
        case GDK_BUTTON_PRESS:
        case GDK_BUTTON_RELEASE: { // OK
            GdkEventButton *evt = (GdkEventButton *)event;
            gevent.u.mouse.state = _GGTKDraw_GdkModifierToKsm(evt->state);
            gevent.u.mouse.x = evt->x;
            gevent.u.mouse.y = evt->y;
            gevent.u.mouse.button = evt->button;
            gevent.u.mouse.time = evt->time;

            Log(LOGDEBUG, "Button %7s: [%f %f]", evt->type == GDK_BUTTON_PRESS ? "press" : "release", evt->x, evt->y);

#ifdef GDK_WINDOWING_QUARTZ
            // Quartz backend fails to give a button press event
            // https://bugzilla.gnome.org/show_bug.cgi?id=769961
            if (!evt->send_event && evt->type == GDK_BUTTON_RELEASE &&
                    gdisp->bs.release_w == gw &&
                    (evt->x < 0 || evt->x > gw->pos.width ||
                     evt->y < 0 || evt->y > gw->pos.height)) {
                evt->send_event = true;
                gdk_event_put(event);
                event->type = GDK_BUTTON_PRESS;
            }
#endif

            if (event->type == GDK_BUTTON_PRESS) {
                int xdiff, ydiff;
                gevent.type = et_mousedown;

                xdiff = abs(((int)evt->x) - gdisp->bs.release_x);
                ydiff = abs(((int)evt->y) - gdisp->bs.release_y);

                if (xdiff + ydiff < gdisp->bs.double_wiggle &&
                        gw == gdisp->bs.release_w &&
                        gevent.u.mouse.button == gdisp->bs.release_button &&
                        (int32_t)(gevent.u.mouse.time - gdisp->bs.last_press_time) < gdisp->bs.double_time &&
                        gevent.u.mouse.time >= gdisp->bs.last_press_time) {  // Time can wrap

                    gdisp->bs.cur_click++;
                } else {
                    gdisp->bs.cur_click = 1;
                }
                gdisp->bs.last_press_time = gevent.u.mouse.time;
				Log(LOGDEBUG, "LAST TIME %d click %d", gevent.u.mouse.time, gdisp->bs.cur_click);
            } else {
                gevent.type = et_mouseup;
                gdisp->bs.release_w = gw;
                gdisp->bs.release_x = evt->x;
                gdisp->bs.release_y = evt->y;
                gdisp->bs.release_button = evt->button;
            }
            gevent.u.mouse.clicks = gdisp->bs.cur_click;
        }
        break;
        case GDK_FOCUS_CHANGE:
            gevent.type = et_focus;
            gevent.u.focus.gained_focus = ((GdkEventFocus *)event)->in;
            gevent.u.focus.mnemonic_focus = false;
            break;
        case GDK_ENTER_NOTIFY:
        case GDK_LEAVE_NOTIFY: { // Should only get this on top level
            GdkEventCrossing *crossing = (GdkEventCrossing *)event;
            if (crossing->focus) { //Focus or inferior
                break;
            }
            if (gdisp->focusfollowsmouse && gw != NULL && gw->eh != NULL) {
                gevent.type = et_focus;
                gevent.u.focus.gained_focus = crossing->type == GDK_ENTER_NOTIFY;
                gevent.u.focus.mnemonic_focus = false;
                _GGTKDraw_CallEHChecked(gw, &gevent, gw->eh);
            }
            gevent.type = et_crossing;
            gevent.u.crossing.x = crossing->x;
            gevent.u.crossing.y = crossing->y;
            gevent.u.crossing.state = _GGTKDraw_GdkModifierToKsm(crossing->state);
            gevent.u.crossing.entered = crossing->type == GDK_ENTER_NOTIFY;
            gevent.u.crossing.device = NULL;
            gevent.u.crossing.time = crossing->time;

            // Always set to false when crossing boundary...
            gw->display->is_space_pressed = false;
        }
        break;
        // The events from here on are synthesised by GGtkWindow; they aren't true Gdk events
        case GDK_CONFIGURE: {
            GdkEventConfigure *configure = (GdkEventConfigure *)event;
            gevent.type = et_resize;
            gevent.u.resize.size.x      = configure->x;
            gevent.u.resize.size.y      = configure->y;
            gevent.u.resize.size.width  = configure->width;
            gevent.u.resize.size.height = configure->height;
            gevent.u.resize.dx          = configure->x - gw->pos.x;
            gevent.u.resize.dy          = configure->y - gw->pos.y;
            gevent.u.resize.dwidth      = configure->width - gw->pos.width;
            gevent.u.resize.dheight     = configure->height - gw->pos.height;
            gevent.u.resize.moved       = gevent.u.resize.sized = false;
            if (gevent.u.resize.dx != 0 || gevent.u.resize.dy != 0) {
                gevent.u.resize.moved = true;
            }
            if (gevent.u.resize.dwidth != 0 || gevent.u.resize.dheight != 0) {
                gevent.u.resize.sized = true;
            }

            // I could make this Windows specific... But it doesn't seem necessary on other platforms too.
            // On Windows, repeated configure messages are sent if we move the window around.
            // This causes CPU usage to go up because mouse handlers of this message just redraw the whole window.
            if (gw->is_toplevel && !gevent.u.resize.sized && gevent.u.resize.moved) {
                gevent.type = et_noevent;
                Log(LOGDEBUG, "Configure DISCARDED: %p:%s, %d %d %d %d", gw, ggtk_window_get_title(gw->w), gw->pos.x, gw->pos.y, gw->pos.width, gw->pos.height);
            } else {
                Log(LOGDEBUG, "CONFIGURED: %p:%s, %d %d %d %d", gw, ggtk_window_get_title(gw->w), gw->pos.x, gw->pos.y, gw->pos.width, gw->pos.height);
            }
            gw->pos = gevent.u.resize.size;
        }
        break;
        case GDK_EXPOSE: {
            GdkEventExpose *expose = (GdkEventExpose*)event;
            gevent.type = et_expose;
            gevent.u.expose.rect.x = expose->area.x;
            gevent.u.expose.rect.y = expose->area.y;
            gevent.u.expose.rect.width = expose->area.width;
            gevent.u.expose.rect.height = expose->area.height;
        };
        break;
        case GDK_MAP:
            gevent.type = et_map;
            gevent.u.map.is_visible = true;
            gw->is_visible = true;
            break;
        case GDK_UNMAP:
            gevent.type = et_map;
            gevent.u.map.is_visible = false;
            gw->is_visible = false;
            break;
        case GDK_DELETE:
            gevent.type = et_close;
            break;
        // end
        // I don't bother implementing GDK_VISIBILITY_NOTIFY; it's deprecated
        case GDK_SELECTION_CLEAR:
        break;
        case GDK_SELECTION_REQUEST:
        break;
        case GDK_SELECTION_NOTIFY: // paste
            break;
        case GDK_PROPERTY_NOTIFY:
            break;
        default:
            Log(LOGDEBUG, "UNPROCESSED GDK EVENT %d %s", event->type, GdkEventName(event->type));
            break;
    }

    if (gevent.type != et_noevent && gw != NULL && gw->eh != NULL) {
        _GGTKDraw_CallEHChecked(gw, &gevent, gw->eh);
    }
    Log(LOGVERBOSE, "[%d] Finished processing %d(%s)", request_id++, event->type, GdkEventName(event->type));
}

static gboolean _GGTKDraw_HandleDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    _GGTKDraw_DispatchEvent(GGTK_WINDOW(user_data), event);
    return true; // GDraw has to explicitly close windows    
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
    }

    if (wattrs->mask & wam_restrict) {
        nw->restrict_input_to_me = wattrs->restrict_input_to_me;
    }
    if (wattrs->mask & wam_redirect) {
        nw->restrict_input_to_me = nw->restrict_input_to_me || wattrs->redirect_chars_to_me;
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
        GGTKWindow icon = gdisp->default_icon;
        if (((wattrs->mask & wam_icon) && wattrs->icon != NULL) && ((GGTKWindow)wattrs->icon)->is_pixmap) {
            icon = (GGTKWindow) wattrs->icon;
        }
        if (icon != NULL) {
            pb = gdk_pixbuf_get_from_surface(icon->pixmap_surface, 0, 0, icon->pos.width, icon->pos.height);
        }
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

        gtk_window_set_geometry_hints(window, NULL, &geom, hints);

        if ((wattrs->mask & wam_transient) && wattrs->transient != NULL) {
            // GGTKDrawSetTransientFor((GWindow)nw, wattrs->transient);
            nw->is_dlg = true;
        } else if (!nw->is_dlg) {
            ++gdisp->top_window_count;
        } else if (nw->restrict_input_to_me) {
            gtk_window_set_modal(window, true);
        }
        nw->isverytransient = (wattrs->mask & wam_verytransient) ? 1 : 0;

        // Finally make our instance...
        nw->w = GGTK_WINDOW(ggtk_window_new(nw, _GGTKDraw_DispatchEvent));
        if (window == NULL) {
            Log(LOGWARN, "Failed to create a GGtkWindow (topleveL)");
            gtk_widget_destroy(GTK_WIDGET(window));
            free(nw->ggc);
            free(nw);
            return NULL;
        }

		gtk_widget_add_events(GTK_WIDGET(window), event_mask);
        gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(nw->w));
		gtk_window_set_default(window, GTK_WIDGET(nw->w));
        gtk_widget_show(GTK_WIDGET(nw->w)); // show/hide controlled on the GtkWindow
        
        g_signal_connect(window, "delete-event", G_CALLBACK(_GGTKDraw_HandleDeleteEvent), nw->w);
    } else {
        nw->w = GGTK_WINDOW(ggtk_window_new(nw, _GGTKDraw_DispatchEvent));
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
	
	// Set event mask, connect to the right signals
	gtk_widget_add_events(GTK_WIDGET(nw->w), event_mask);
    
    // Set cursor
    if ((wattrs->mask & wam_cursor) && wattrs->cursor != ct_default) {
        GGTKDrawSetCursor((GWindow)nw, wattrs->cursor);
    }

    // Add a reference to our own structure.
    GGTKDRAW_ADDREF(nw);
    // We need to make sure that our widget doesn't get finalised until we're ready, so add a ref
    g_object_ref_sink(G_OBJECT(nw->w));

    // Event handler
    if (eh != NULL) {
        GEvent e = {0};
        e.type = et_create;
        e.w = (GWindow) nw;
        e.native_window = nw->w;
        _GGTKDraw_CallEHChecked(nw, &e, eh);
    }

    Log(LOGINFO, "Window created: %p[%p][%d] has window: %d %d", nw, nw->w, nw->is_toplevel,
        gtk_widget_get_has_window(GTK_WIDGET(nw->w)),
        gtk_widget_has_focus(GTK_WIDGET(nw->w)));
    return (GWindow)nw;
}

static GWindow _GGTKDraw_NewPixmap(GDisplay *disp, GWindow similar, uint16 width, uint16 height, bool is_bitmap, unsigned char *data) {
    GGTKDisplay *gdisp = (GGTKDisplay *)disp;
    GGTKWindow gw = (GGTKWindow)calloc(1, sizeof(struct ggtkwindow));
    if (gw == NULL) {
        Log(LOGERR, "GGTKDRAW: GGTKWindow calloc failed!");
        return NULL;
    }

    gw->ggc = _GGTKDraw_NewGGC();
    if (gw->ggc == NULL) {
        Log(LOGERR, "GGTKDRAW: GGC alloc failed!");
        free(gw);
        return NULL;
    }
    gw->ggc->bg = gdisp->def_background;
    width &= 0x7fff; // We're always using a cairo surface...

    gw->display = gdisp;
    gw->is_pixmap = 1;
    gw->parent = NULL;
    gw->pos.x = gw->pos.y = 0;
    gw->pos.width = width;
    gw->pos.height = height;

    if (data == NULL) {
        if (similar == NULL) {
            gw->pixmap_surface = cairo_image_surface_create(is_bitmap ? CAIRO_FORMAT_A1 : CAIRO_FORMAT_ARGB32, width, height);
        } else {
			GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(((GGTKWindow)similar)->w));
            gw->pixmap_surface = gdk_window_create_similar_surface(window, CAIRO_CONTENT_COLOR, width, height);
        }
    } else {
        cairo_format_t format = is_bitmap ? CAIRO_FORMAT_A1 : CAIRO_FORMAT_ARGB32;
        gw->pixmap_surface = cairo_image_surface_create_for_data(data, format, width, height, cairo_format_stride_for_width(format, width));
    }
	
	if (gw->pixmap_surface != NULL) {
		gw->pixmap_context = cairo_create(gw->pixmap_surface);
		if (gw->pixmap_context != NULL) {
			PangoContext *context = similar == NULL ? gdisp->default_pango_context :
									gtk_widget_get_pango_context(GTK_WIDGET(((GGTKWindow)similar)->w));
			gw->pixmap_layout = pango_layout_new(context);
			if (gw->pixmap_layout != NULL) {
                // Add a reference to ourselves
                GGTKDRAW_ADDREF(gw);
				return (GWindow)gw;
			}
			Log(LOGWARN, "Failed to create pixmap layout, context=%p", context);
			cairo_destroy(gw->pixmap_context);
		} else {
			Log(LOGWARN, "Failed to create pixmap context, surface=%p", gw->pixmap_surface);
		}
		cairo_surface_destroy(gw->pixmap_surface);
	} else {
		Log(LOGWARN, "Failed to crate pixmap surface");
	}
	
	free(gw->ggc);
	free(gw);
	return NULL;
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
	GGTKWindow gicon = (GGTKWindow)icon;
    if (gicon->is_pixmap) {
        gicon->display->default_icon = gicon;
    }
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
    //TODO: Check format?
    return _GGTKDraw_NewPixmap(gdisp, similar, width, height, false, NULL);
}

static GWindow GGTKDrawCreateBitmap(GDisplay *gdisp, uint16 width, uint16 height, uint8 *data) {
    Log(LOGVERBOSE, " ");
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_A1, width);
    int actual = (width & 0x7fff) / 8;

    if (actual != stride) {
        GWindow ret = _GGTKDraw_NewPixmap(gdisp, NULL, width, height, true, NULL);
        if (ret == NULL) {
            return NULL;
        }

        cairo_surface_flush(((GGTKWindow)ret)->pixmap_surface);
        uint8 *buf = cairo_image_surface_get_data(((GGTKWindow)ret)->pixmap_surface);
        for (int j = 0; j < height; j++) {
            memcpy(buf + stride * j, data + actual * j, actual);
        }
        cairo_surface_mark_dirty(((GGTKWindow)ret)->pixmap_surface);
        return ret;
    }

    return _GGTKDraw_NewPixmap(gdisp, NULL, width, height, true, data);
}

static GCursor GGTKDrawCreateCursor(GWindow src, GWindow mask, Color fg, Color bg, int16 x, int16 y) {
    Log(LOGVERBOSE, " ");

	GGTKDisplay *gdisp = (GGTKDisplay *)(src->display);
    GdkCursor *cursor = NULL;
    if (mask == NULL) { // Use src directly
        assert(src != NULL);
        assert(src->is_pixmap);
        cursor = gdk_cursor_new_from_surface(gdisp->display, ((GGTKWindow)src)->pixmap_surface, x, y);
    } else { // Assume it's an X11-style cursor
        cairo_surface_t *cs = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, src->pos.width, src->pos.height);
        cairo_t *cc = cairo_create(cs);

        // Masking
        //Background
        cairo_set_source_rgb(cc, COLOR_RED(bg) / 255., COLOR_GREEN(bg) / 255., COLOR_BLUE(bg) / 255.);
        cairo_mask_surface(cc, ((GGTKWindow)mask)->pixmap_surface, 0, 0);
        //Foreground
        cairo_set_source_rgb(cc, COLOR_RED(fg) / 255., COLOR_GREEN(fg) / 255., COLOR_BLUE(fg) / 255.);
        cairo_mask_surface(cc, ((GGTKWindow)src)->pixmap_surface, 0, 0);

        cursor = gdk_cursor_new_from_surface(gdisp->display, cs, x, y);
        cairo_destroy(cc);
        cairo_surface_destroy(cs);
    }

    g_ptr_array_add(gdisp->cursors, cursor);
    return ct_user + (gdisp->cursors->len - 1);
}

static void GGTKDrawDestroyCursor(GDisplay *disp, GCursor gcursor) {
    Log(LOGVERBOSE, " ");
	
	GGTKDisplay *gdisp = (GGTKDisplay *)disp;
    gcursor -= ct_user;
    if ((int)gcursor >= 0 && gcursor < gdisp->cursors->len) {
        g_object_unref(gdisp->cursors->pdata[gcursor]);
        gdisp->cursors->pdata[gcursor] = NULL;
    }
}

static void GGTKDrawDestroyWindow(GWindow w) {
    Log(LOGVERBOSE, " ");
    GGTKWindow gw = (GGTKWindow) w;

    if (gw->is_dying) {
        return;
    }

    gw->is_dying = true;

    if (!gw->is_pixmap && gw != gw->display->groot) {
        GList_Glib *list = gtk_container_get_children(GTK_CONTAINER(gw->w));
        GList_Glib *child = list;
        while (child != NULL) {
            if (GGTK_IS_WINDOW(child->data)) {
                GGTKDrawDestroyWindow((GWindow)ggtk_window_get_base(GGTK_WINDOW(child->data)));
            }
            child = child->next;
        }
        g_list_free(list);
    }
    GGTKDRAW_DECREF(gw, _GGTKDraw_InitiateWindowDestroy);
}

static int GGTKDrawNativeWindowExists(GDisplay *UNUSED(gdisp), void *native_window) {
    Log(LOGVERBOSE, " ");
    
    // So if the window is dying, the gdk window is already gone.
    // But gcontainer.c expects this to return true on et_destroy...
    // This returns false when native_window is finalised
    // We make sure it only gets finalised after et_destroy has been sent
    return GGTK_IS_WINDOW(native_window);
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
	GGTKWindow gw = (GGTKWindow)w;
	GdkRGBA col = {
        .red = COLOR_RED(gcol) / 255.,
        .green = COLOR_GREEN(gcol) / 255.,
        .blue = COLOR_BLUE(gcol) / 255.,
        .alpha = 1.
    };
	ggtk_window_set_background(gw->w, &col);
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
	GGTKWindow gw = (GGTKWindow)w;
	GtkWidget *widget = GTK_WIDGET(gw->w);
	if (gw->is_toplevel) {
		widget = gtk_widget_get_toplevel(widget);
	}
    gtk_widget_set_visible(widget, show);
}

static void GGTKDrawMove(GWindow w, int32 x, int32 y) {
    //Log(LOGDEBUG, "%p:%s, %d %d", gw, ((GGTKWindow) gw)->window_title, x, y);
	GGTKWindow gw = (GGTKWindow)w;
	if (gw->is_toplevel) {
		gtk_window_move(ggtk_window_get_window(gw->w), x, y);
	} else {
		gtk_layout_move(GTK_LAYOUT(gw->parent->w), GTK_WIDGET(gw->w), x, y);
	}
}

static void GGTKDrawTrueMove(GWindow w, int32 x, int32 y) {
    Log(LOGVERBOSE, " ");
	GGTKDrawMove(w, x, y);
}

static void GGTKDrawResize(GWindow w, int32 width, int32 height) {
    //Log(LOGDEBUG, "%p:%s, %d %d", gw, ((GGTKWindow) gw)->window_title, w, h);
	GGTKWindow gw = (GGTKWindow)w;
	if (gw->is_toplevel) {
		gtk_window_resize(ggtk_window_get_window(gw->w), width, height);
	} else {
		gtk_widget_set_size_request(GTK_WIDGET(gw->w), width, height);
	}
}

static void GGTKDrawMoveResize(GWindow w, int32 x, int32 y, int32 width, int32 height) {
    //Log(LOGDEBUG, "%p:%s, %d %d %d %d", gw, ((GGTKWindow) gw)->window_title, x, y, w, h);
	// fixme
	GGTKDrawMove(w, x, y);
	GGTKDrawResize(w, width, height);
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
	GGTKWindow gw = (GGTKWindow)w;
	if (gw->is_toplevel) {
		gtk_window_set_title(ggtk_window_get_window(gw->w), title);
	}
}

static void GGTKDrawSetWindowTitles(GWindow w, const unichar_t *title, const unichar_t *UNUSED(icontitle)) {
    Log(LOGVERBOSE, " ");
	char *str = u2utf8_copy(title);
	GGTKDrawSetWindowTitles8(w, str, NULL);
	free(str);
}

static char *GGTKDrawGetWindowTitle8(GWindow w) {
    Log(LOGVERBOSE, " ");
    return copy(ggtk_window_get_title(((GGTKWindow)w)->w));
}

static unichar_t *GGTKDrawGetWindowTitle(GWindow w) {
    Log(LOGVERBOSE, " "); // assert(false);
	char *title = GGTKDrawGetWindowTitle8(w);
	unichar_t *ret = utf82u_copy(title);
	free(title);
	return ret;
}

static void GGTKDrawSetTransientFor(GWindow transient, GWindow owner) {
    Log(LOGVERBOSE, " ");
}

static void GGTKDrawGetPointerPosition(GWindow w, GEvent *ret) {
    Log(LOGVERBOSE, " ");
    
    GGTKWindow gw = (GGTKWindow)w;
    GdkDisplay *display = gw->display->display;
    GtkWindow *window = NULL;
    if (gw->w) {
        window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(gw->w)));
        display = gdk_screen_get_display(gtk_window_get_screen(window));
    }
    
    GdkModifierType mask;
    int x, y;
    GdkDevice *pointer = _GGTKDraw_GetPointer(display);
    if (pointer == NULL) {
        ret->u.mouse.x = 0;
        ret->u.mouse.y = 0;
        ret->u.mouse.state = 0;
        return;
    }

    // FIXME: Root window?!
    gdk_window_get_device_position(gtk_widget_get_window(GTK_WIDGET(window)), pointer, &x, &y, &mask);
    gtk_widget_translate_coordinates(GTK_WIDGET(window), GTK_WIDGET(gw->w), x, y, &x, &y);
    ret->u.mouse.x = x;
    ret->u.mouse.y = y;
    ret->u.mouse.state = _GGTKDraw_GdkModifierToKsm(mask);
}

static GWindow GGTKDrawGetPointerWindow(GWindow gw) {
    Log(LOGVERBOSE, " ");
    return NULL;
}

static void GGTKDrawSetCursor(GWindow w, GCursor gcursor) {
    Log(LOGVERBOSE, " ");
    GGTKWindow gw = (GGTKWindow)w;
    GdkWindow *window = gtk_widget_get_window(gtk_widget_get_toplevel(GTK_WIDGET(gw->w)));
    GdkCursor *cursor = NULL;
    
    if (!window) {
        Log(LOGWARN, "Could not get the backing gdk window");
    }
    
    // FIXME: This applies to the whole window and not just the widget area

    switch (gcursor) {
        case ct_default:
        case ct_backpointer:
        case ct_pointer:
            cursor = gdk_cursor_new_from_name(gw->display->display, "default");
            break;
        case ct_hand:
            cursor = gdk_cursor_new_from_name(gw->display->display, "hand");
            break;
        case ct_question:
            cursor = gdk_cursor_new_from_name(gw->display->display, "help");
            break;
        case ct_cross:
            cursor = gdk_cursor_new_from_name(gw->display->display, "crosshair");
            break;
        case ct_4way:
            cursor = gdk_cursor_new_from_name(gw->display->display, "move");
            break;
        case ct_text:
            cursor = gdk_cursor_new_from_name(gw->display->display, "text");
            break;
        case ct_watch:
            cursor = gdk_cursor_new_from_name(gw->display->display, "wait");
            break;
        case ct_draganddrop:
            cursor = gdk_cursor_new_from_name(gw->display->display, "pointer");
            break;
        case ct_invisible:
            return; // There is no *good* reason to make the cursor invisible
            break;
        default:
            Log(LOGDEBUG, "CUSTOM CURSOR! %d", gcursor);
    }

    if (gcursor >= ct_user) {
        GGTKDisplay *gdisp = gw->display;
        gcursor -= ct_user;
        if (gcursor < gdisp->cursors->len && gdisp->cursors->pdata[gcursor] != NULL) {
            gdk_window_set_cursor(window, (GdkCursor *)gdisp->cursors->pdata[gcursor]);
            gw->current_cursor = gcursor;
        } else {
            Log(LOGWARN, "Invalid cursor value passed: %d", gcursor);
        }
    } else {
        gdk_window_set_cursor(window, cursor);
        gw->current_cursor = gcursor;
        if (cursor != NULL) {
            g_object_unref(cursor);
        }
    }
}

static GCursor GGTKDrawGetCursor(GWindow gw) {
    Log(LOGVERBOSE, " ");
    return ((GGTKWindow)gw)->current_cursor;
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
    GGTKWindow gw = (GGTKWindow) w;
    GRect temp;

    vert = -vert;
    if (rect == NULL) {
        temp.x = temp.y = 0;
        temp.width = gw->pos.width;
        temp.height = gw->pos.height;
        rect = &temp;
    }

    GDrawRequestExpose(w, rect, false);
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
    Log(LOGWARN, "%p [%s]", w, ggtk_window_get_title(((GGTKWindow) w)->w));
	assert(!w->is_pixmap);
	cairo_rectangle_int_t r;
	if (rect) {
		r.x = rect->x;
		r.y = rect->y;
		r.width = rect->width;
		r.height = rect->height;
	}
	ggtk_window_request_expose(GGTK_WINDOW(((GGTKWindow)w)->w), rect ? &r : NULL);
}

static void GGTKDrawForceUpdate(GWindow w) {
    //Log(LOGVERBOSE, " ");
	// fixme
	ggtk_window_request_expose(GGTK_WINDOW(((GGTKWindow)w)->w), NULL);
}

static void GGTKDrawSync(GDisplay *gdisp) {
    //Log(LOGVERBOSE, " ");
	gdk_display_sync(((GGTKDisplay *)gdisp)->display);
}

static void GGTKDrawSkipMouseMoveEvents(GWindow UNUSED(gw), GEvent *UNUSED(gevent)) {
    //Log(LOGVERBOSE, " ");
    // Not implemented, not needed.
}

static void GGTKDrawProcessPendingEvents(GDisplay *gdisp) {
    //Log(LOGVERBOSE, " ");
	while (gtk_events_pending())
		gtk_main_iteration_do(false);
}

static void GGTKDrawProcessWindowEvents(GWindow w) {
    Log(LOGWARN, "This function SHOULD NOT BE CALLED! Window: %p", w);
	
	if (w != NULL)  {
        GGTKDrawProcessPendingEvents(w->display);
    }
}

static void GGTKDrawProcessOneEvent(GDisplay *gdisp) {
    //Log(LOGVERBOSE, " ");
	gtk_main_iteration();
}

static void GGTKDrawEventLoop(GDisplay *gdisp) {
    Log(LOGVERBOSE, " ");
    do {
        while (((GGTKDisplay *)gdisp)->top_window_count > 0) {
            gtk_main_iteration();
            if ((gdisp->err_flag) && (gdisp->err_report)) {
                GDrawIErrorRun("%s", gdisp->err_report);
            }
            if (gdisp->err_report) {
                free(gdisp->err_report);
                gdisp->err_report = NULL;
            }
        }
        GGTKDrawProcessPendingEvents(gdisp);
    } while (((GGTKDisplay *)gdisp)->top_window_count > 0 || gtk_events_pending());
}

static void GGTKDrawPostEvent(GEvent *e) {
    //Log(LOGVERBOSE, " ");
	GGTKWindow gw = (GGTKWindow)(e->w);
    e->native_window = gw->w;
    _GGTKDraw_CallEHChecked(gw, e, gw->eh);
}

static void GGTKDrawPostDragEvent(GWindow w, GEvent *mouse, enum event_type et) {
    Log(LOGVERBOSE, " ");
}

static int GGTKDrawRequestDeviceEvents(GWindow w, int devcnt, struct gdeveventmask *de) {
    Log(LOGVERBOSE, " ");
    return 0; //Not sure how to handle... For tablets...
}

static GTimer *GGTKDrawRequestTimer(GWindow w, int32 time_from_now, int32 frequency, void *userdata) {
    Log(LOGVERBOSE, " ");
    GGTKTimer *timer = calloc(1, sizeof(GGTKTimer));
    GGTKWindow gw = (GGTKWindow)w;
    if (timer == NULL)  {
        return NULL;
    }

    gw->display->timers = g_list_append(gw->display->timers, timer);

    timer->owner = w;
    timer->repeat_time = frequency;
    timer->userdata = userdata;
    timer->active = true;
    timer->has_differing_repeat_time = (time_from_now != frequency);
    timer->glib_timeout_id = g_timeout_add(time_from_now, _GGTKDraw_ProcessTimerEvent, timer);

    GGTKDRAW_ADDREF(timer);
    return (GTimer *)timer;
}

static void GGTKDrawCancelTimer(GTimer *timer) {
    Log(LOGVERBOSE, " ");
    GGTKTimer *gtimer = (GGTKTimer *)timer;
    gtimer->active = false;
    GGTKDRAW_DECREF(gtimer, _GGTKDraw_OnTimerDestroyed);
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
	GGTKDrawClipPreserve,
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
    GdkRectangle geom;
#ifdef GGTKDRAW_GDK_3_22
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
    gdk_monitor_get_geometry(monitor, &geom);
#else
    geom.x = geom.y = 0;
    geom.width = gdk_screen_get_width(gdk_display_get_default_screen(display));
    geom.height = gdk_screen_get_height(gdk_display_get_default_screen(display));
#endif

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
    gdisp->default_pango_context = gdk_pango_context_get_for_screen(gdk_display_get_default_screen(display));
    gdisp->default_pango_layout = pango_layout_new(gdisp->default_pango_context);
    gdisp->res = pango_cairo_context_get_resolution(gdisp->default_pango_context);
    if (gdisp->res <= 0) {
        Log(LOGWARN, "Failed to get default DPI, assuming 96");
        gdisp->res = 96;
    }

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
    GGTKDisplay *gdisp = (GGTKDisplay *)(disp);

    // Indicate we're dying...
    gdisp->is_dying = true;

    // Destroy remaining windows
    GList_Glib *toplevels = gtk_window_list_toplevels();
    GList_Glib *iter = toplevels;
    while (iter) {
        GtkWidget *widget = gtk_bin_get_child(GTK_BIN(iter->data));
        if (GGTK_IS_WINDOW(widget)) {
            GGtkWindow *ggw = GGTK_WINDOW(widget);
            GGTKWindow gw = ggtk_window_get_base(ggw);
            if (gw) {
                Log(LOGINFO, "Forcibly destroying window (%p:%s)", gw, ggtk_window_get_title(ggw));
                gw->reference_count = 2;
                GGTKDrawDestroyWindow((GWindow)gw);
                _GGTKDraw_OnWindowDestroyed(gw);                
            }
        }
        iter = iter->next;
    }
    g_list_free(toplevels);

    // Destroy root window
    _GGTKDraw_OnWindowDestroyed(gdisp->groot);
    gdisp->groot = NULL;

    // Destroy cursors
    for (guint i = 0; i < gdisp->cursors->len; i++) {
        if (gdisp->cursors->pdata[i] != NULL) {
            GGTKDrawDestroyCursor((GDisplay *)gdisp, (GCursor)(ct_user + i));
        }
    }
    g_ptr_array_free(gdisp->cursors, true);
    gdisp->cursors = NULL;

    // Destroy any orphaned timers (???)
    if (gdisp->timers != NULL) {
        Log(LOGWARN, "Orphaned timers present - forcibly freeing!");
        while (gdisp->timers != NULL) {
            GGTKTimer *timer = (GGTKTimer *)gdisp->timers->data;
            timer->reference_count = 1;
            GGTKDrawCancelTimer((GTimer *)timer);
        }
    }

    // Get rid of our default pango layout/context
    g_object_unref(gdisp->default_pango_layout);
    g_object_unref(gdisp->default_pango_context);
    gdisp->default_pango_layout = NULL;
    gdisp->default_pango_context = NULL;

    // Destroy the fontstate
    free(gdisp->fontstate);
    gdisp->fontstate = NULL;

    // Close the display
    if (gdisp->display != NULL) {
        gdisp->display = NULL;
    }

    // Free the data structure
    free(gdisp);
}

#endif // FONTFORGE_CAN_USE_GTK
