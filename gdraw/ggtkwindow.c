#include "ggtkdrawP.h"
#include "ggdkdrawloggerP.h"

#ifdef FONTFORGE_CAN_USE_GDK

struct _GGtkWindow
{
    GtkLayout parent_instance;

    // private data
    GGTKWindow gw;
    GGtkWindowEventHandler eh;
    GdkRGBA background_color;
	PangoLayout *pango_layout;

    cairo_surface_t *offscreen_surface;
    cairo_t* offscreen_context;
    cairo_region_t* dirty_regions;
    int offscreen_width;
    int offscreen_height;

    bool disposed;
    bool finalized;
};

G_DEFINE_TYPE(GGtkWindow, ggtk_window, GTK_TYPE_LAYOUT)

GGTKWindow ggtk_window_get_base(GGtkWindow *ggw)
{
    g_return_val_if_fail(ggw != NULL, NULL);
    return ggw->gw;
}

GtkWindow* ggtk_window_get_window(GGtkWindow *ggw)
{
    g_return_val_if_fail(ggw->gw->is_toplevel, NULL);
    return GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(ggw)));
}

void ggtk_window_set_background(GGtkWindow *ggw, GdkRGBA *col)
{
    ggw->background_color = *col;
    gtk_widget_queue_draw(GTK_WIDGET(ggw));
}

cairo_t* ggtk_window_get_cairo_context(GGtkWindow *ggw)
{
    if (ggw->offscreen_context) {
        return ggw->offscreen_context;
    } else if (!ggw->offscreen_surface) {
		// This should be rare - trying to draw before an initial expose event
		// Just return this, cairo_create never returns NULL, it just won't do anything
		Log(LOGWARN, "Tried to draw before any expose");
        return cairo_create(NULL);
    }

    ggw->offscreen_context = cairo_create(ggw->offscreen_surface);
    gtk_widget_queue_draw(GTK_WIDGET(ggw));

    return ggw->offscreen_context;
}

void ggtk_window_request_expose(GGtkWindow *ggw, cairo_rectangle_int_t *area)
{
    if (area) {
        Log(LOGDEBUG, "REQUEST EXPOSE %d %d %d %d", area->x, area->y, area->width, area->height);
        if (ggw->dirty_regions) {
            cairo_region_union_rectangle(ggw->dirty_regions, area);
        } else {
            ggw->dirty_regions = cairo_region_create_rectangle(area);
        }
        gtk_widget_queue_draw_area(GTK_WIDGET(ggw), area->x, area->y, area->width, area->height);
        return;
    } else {
        Log(LOGDEBUG, "REQUEST EXPOSE ALL");
    }
    gtk_widget_queue_draw(GTK_WIDGET(ggw));
}

const char *ggtk_window_get_title(GGtkWindow *ggw)
{
    if (ggw->gw->is_toplevel) {
        return gtk_window_get_title(ggtk_window_get_window(ggw));
    }
	return NULL;
}

PangoLayout *ggtk_window_get_pango_layout(GGtkWindow *ggw)
{
	if (ggw->pango_layout) {
		return ggw->pango_layout;
	}
	ggw->pango_layout = pango_layout_new(gtk_widget_get_pango_context(GTK_WIDGET(ggw)));
	return ggw->pango_layout;
}

GtkWidget* ggtk_window_new(GGTKWindow gw, GGtkWindowEventHandler eh)
{
    GGtkWindow *ggw = GGTK_WINDOW(g_object_new(GGTK_TYPE_WINDOW, NULL));
    g_return_val_if_fail(ggw != NULL, NULL);
    g_return_val_if_fail(eh != NULL, NULL);

    ggw->eh = eh;
    ggw->gw = gw;

    return GTK_WIDGET(ggw);
}

static void ggtk_window_size_allocate(GtkWidget* widget, GtkAllocation *alloc)
{
    Log(LOGINFO, "SIZE ALLOCATE: %d %d %d %d", alloc->x, alloc->y, alloc->width, alloc->height);
    GTK_WIDGET_CLASS(ggtk_window_parent_class)->size_allocate(widget, alloc);
}

static void ggtk_window_screen_changed(GtkWidget *widget, G_GNUC_UNUSED GdkScreen *previous_screen)
{
	GGtkWindow *ggw = GGTK_WINDOW(widget);
	if (ggw->pango_layout) {
		g_object_unref(ggw->pango_layout);
		ggw->pango_layout = NULL;
	}
}

static gboolean ggtk_window_draw(GtkWidget* widget, cairo_t* cr)
{
    Log(LOGWARN, "DRAWING NOW");
    GGtkWindow *ggw = GGTK_WINDOW(widget);

    bool repaint_all = false;
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);

    if (!ggw->offscreen_surface || ggw->offscreen_width != alloc.width || ggw->offscreen_height != alloc.height) {
        GdkWindow* window = gtk_widget_get_window(widget);
        
        if (ggw->offscreen_context) {
            cairo_destroy(ggw->offscreen_context);
            ggw->offscreen_context = NULL;
        }

        cairo_surface_destroy(ggw->offscreen_surface);
        if (ggw->dirty_regions) {
            cairo_region_destroy(ggw->dirty_regions);
            ggw->dirty_regions = NULL;
        }
        ggw->offscreen_surface = gdk_window_create_similar_surface(window, CAIRO_CONTENT_COLOR, alloc.width, alloc.height);
        ggw->offscreen_width = alloc.width;
        ggw->offscreen_height = alloc.height;
        repaint_all = true;
    }

    if (repaint_all || ggw->dirty_regions) {
		cairo_rectangle_int_t extents = {.width = alloc.width, .height = alloc.height};
        ggw->offscreen_context = cairo_create(ggw->offscreen_surface);
        if (ggw->dirty_regions) {
            cairo_rectangle_int_t area;
            int num_rectangles = cairo_region_num_rectangles(ggw->dirty_regions);

            for (int i = 0; i < num_rectangles; ++i) {
                cairo_region_get_rectangle(ggw->dirty_regions, i, &area);
                cairo_rectangle(ggw->offscreen_context, area.x, area.y, area.width, area.height);
            }
            cairo_clip(ggw->offscreen_context);
			cairo_region_get_extents(ggw->dirty_regions, &extents);
            cairo_region_destroy(ggw->dirty_regions);
            ggw->dirty_regions = NULL;
        }

		cairo_set_operator(ggw->offscreen_context, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgba(ggw->offscreen_context,
            ggw->background_color.red, ggw->background_color.green, ggw->background_color.blue, ggw->background_color.alpha);
        cairo_paint(ggw->offscreen_context);
		cairo_set_operator(ggw->offscreen_context, CAIRO_OPERATOR_OVER);
        
        if (repaint_all) {
            if (ggw->gw->is_toplevel) {
                // hmmmm
                gtk_window_get_position(ggtk_window_get_window(ggw), &alloc.x, &alloc.y);
            }
            
            GdkEventConfigure configure = {
                .type = GDK_CONFIGURE,
                .send_event = TRUE,
                .x = alloc.x,
                .y = alloc.y,
                .width = alloc.width,
                .height = alloc.height,
            };
            
            ggw->eh(ggw, (GdkEvent*)&configure);
        }

        GdkEventExpose event = {
            .type = GDK_EXPOSE,
            .send_event = true,
        };
        event.area.x = extents.x;
        event.area.y = extents.y;
        event.area.width = extents.width;
        event.area.height = extents.height;
        
        Log(LOGWARN, "REFRESH ON [%d,%d,%d,%d]", extents.x, extents.y, extents.width, extents.height);
        ggw->eh(ggw, (GdkEvent*)&event);
        
        cairo_destroy(ggw->offscreen_context);
        ggw->offscreen_context = NULL;
    }

    // Now paint the offscreen surface
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE); // can't do this if it's not opaque
    cairo_set_source_surface(cr, ggw->offscreen_surface, 0, 0);
    cairo_paint(cr);

    return GTK_WIDGET_CLASS(ggtk_window_parent_class)->draw(widget, cr);
}

static gboolean ggtk_window_event(GtkWidget *widget, GdkEvent *event)
{
    Log(LOGINFO, "Received event type %s on %p", GdkEventName(event->type), widget);
    GGtkWindow *ggw = GGTK_WINDOW(widget);
    switch (event->type) {
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
        case GDK_MOTION_NOTIFY:
        case GDK_SCROLL:
        case GDK_BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
        case GDK_FOCUS_CHANGE:
        case GDK_ENTER_NOTIFY:
        case GDK_LEAVE_NOTIFY:
            ggw->eh(ggw, event);
            return true; // don't propagate any further
        break;
        default:
            Log(LOGINFO, "Discarded event %s on %p", GdkEventName(event->type), widget);
    }
    
    return false;
}

static void ggtk_window_map(GtkWidget *widget)
{
    Log(LOGINFO, "Received map on %p", widget);
    GGtkWindow *ggw = GGTK_WINDOW(widget);
    GdkEventAny map = {
        .type = GDK_MAP,
        .send_event = true,
    };
    ggw->eh(ggw, (GdkEvent*)&map);

    GTK_WIDGET_CLASS(ggtk_window_parent_class)->map(widget);
}

static void ggtk_window_unmap(GtkWidget *widget)
{
    Log(LOGINFO, "Received unmap on %p", widget);
    GGtkWindow *ggw = GGTK_WINDOW(widget);
    GdkEventAny map = {
        .type = GDK_UNMAP,
        .send_event = true,
    };
    ggw->eh(ggw, (GdkEvent*)&map);
    
    GTK_WIDGET_CLASS(ggtk_window_parent_class)->unmap(widget);
}

static void ggtk_window_dispose(GObject *gobject)
{
    GGtkWindow *ggw = GGTK_WINDOW(gobject);
    ggw->disposed = true;
	
	G_OBJECT_CLASS(ggtk_window_parent_class)->dispose(gobject);
}

static void ggtk_window_finalize(GObject *gobject)
{
    GGtkWindow *ggw = GGTK_WINDOW(gobject);
    ggw->finalized = true;

	if (ggw->pango_layout) {
		g_object_unref(ggw->pango_layout);
		ggw->pango_layout = NULL;
	}
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

    ggw->gw = NULL;
    G_OBJECT_CLASS(ggtk_window_parent_class)->finalize(gobject);
}

static void ggtk_window_class_init(GGtkWindowClass *ggwc)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(ggwc);
    GObjectClass *object_class = G_OBJECT_CLASS(ggwc);
    
    widget_class->draw = ggtk_window_draw;
    widget_class->event = ggtk_window_event;
    widget_class->map = ggtk_window_map;
    widget_class->screen_changed = ggtk_window_screen_changed;
    widget_class->size_allocate = ggtk_window_size_allocate;
    widget_class->unmap = ggtk_window_unmap;

    object_class->dispose = ggtk_window_dispose;
    object_class->finalize = ggtk_window_finalize;
}

static void ggtk_window_init(GGtkWindow *ggw)
{
    GtkWidget *widget = GTK_WIDGET(ggw);
    gtk_widget_set_can_default(widget, true);
    gtk_widget_set_can_focus(widget, true);
    gtk_widget_set_focus_on_click(widget, true);
}

#endif // FONTFORGE_CAN_USE_GDK
