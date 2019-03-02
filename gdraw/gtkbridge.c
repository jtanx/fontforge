#include "basics.h"

#ifdef FONTFORGE_CAN_USE_GTK_BRIDGE
#include <gtk/gtk.h>

static void print_hello(GtkWidget *widget, gpointer data) {
	printf ("Hello World\n");
}

static void modal_win(GtkWidget *widget, gpointer data) {
	GtkWidget *dialog, *label, *content_area;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_dialog_new_with_buttons("My dialog", GTK_WINDOW(widget), flags,
	         "_OK", GTK_RESPONSE_ACCEPT, "_Cancel", GTK_RESPONSE_REJECT, NULL);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	label = gtk_label_new("This is a message");
	g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_container_add(GTK_CONTAINER(content_area), label);
	gtk_widget_show_all(dialog);
}

static GtkWidget *make_window() {
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *button_box;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

	button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_container_add(GTK_CONTAINER (window), button_box);

	button = gtk_button_new_with_label ("Hello World");
	//g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
	g_signal_connect (button, "clicked", G_CALLBACK (modal_win), NULL);
	// g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
	gtk_container_add (GTK_CONTAINER (button_box), button);

	gtk_widget_show_all (window);
}

static GtkWidget *the_window;

void gtkb_addWindow() {
	if (the_window == NULL) {
		//gtk_init(0,NULL);
		the_window = make_window();
	}
}

void gtkb_do_event(GdkEvent *event) {
	if (the_window != NULL)
		gtk_main_do_event(event);
}

#endif // FONTFORGE_CAN_USE_GTK_BRIDGE

