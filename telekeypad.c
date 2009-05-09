#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmarshal.h>

#include "telekeypad.h"

#define PATH_CHAR_MAX 1025
#define PROCNAME "telekeypad"

void open_telekeypad(GtkButton *button, gpointer data);

static void close_window(GtkButton *button, G_GNUC_UNUSED gpointer user_data)
{
	GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
	gtk_widget_destroy(GTK_WIDGET(window));
}


void entry_activate(GtkEntry *entry, GtkButton *button)
{
	/*g_signal_emit_by_name(button, "clicked");*/
	gtk_widget_activate(GTK_WIDGET(button));
}

void send_delete_event(GtkWindow *window)
{
	/* Synthesize delete_event to close dialog. */
	GtkWidget *widget = GTK_WIDGET (window);
	GdkEvent *event;

	event = gdk_event_new (GDK_DELETE);

	event->any.window = g_object_ref (widget->window);
	event->any.send_event = TRUE;

	gtk_main_do_event (event);
	gdk_event_free (event);
}

static GtkWindow *prepare_window()
{
	GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window), PROCNAME);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	g_signal_connect(G_OBJECT(window), "destroy",
			G_CALLBACK(gtk_main_quit), NULL);

	/* DND */
	GtkWidget *widget = GTK_WIDGET(window);
	gtk_drag_dest_set(widget, GTK_DEST_DEFAULT_ALL, NULL, 0,
			GDK_ACTION_COPY | GDK_ACTION_MOVE);
	gtk_drag_dest_add_uri_targets(widget);

	/* Key binding */
	GtkWindowClass *class = GTK_WINDOW_GET_CLASS(window);
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	g_signal_new (("close_window"),
			G_OBJECT_CLASS_TYPE (gobject_class),
			G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			0,
			NULL, NULL,
			gtk_marshal_NONE__NONE,
			G_TYPE_NONE, 0);
	g_signal_connect(G_OBJECT(window), "close-window",
			G_CALLBACK(send_delete_event), NULL);

	GtkBindingSet *binding_set = gtk_binding_set_by_class(class);
	gtk_binding_entry_add_signal (binding_set,
			GDK_Escape, 0, "close-window", 0);

	return window;
}

void show_window_choose_project()
{
	GtkWidget *window, *button;
	char locale_dir[PATH_CHAR_MAX] = "./locale";	/* default */
	//char locale_dir[PATH_CHAR_MAX] = "/usr/share/locale";	/* default */
#if _WIN32
	char locale_env[65] = {0};
	char locale_name[65] = "";	/* default is empty value */
	GetLocaleFromRegistry(locale_name, 65);
	sprintf(locale_env, "LC_ALL=%s", locale_name);
	putenv(locale_env);
	GetFilePathInModuleDir("locale", locale_dir, PATH_CHAR_MAX - 1);
#else
	setlocale(LC_ALL, "");
#endif
	textdomain(PROCNAME);
	bindtextdomain(PROCNAME, locale_dir);

	window = GTK_WIDGET(prepare_window());
	button = gtk_button_new_with_label("telephone");
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(open_telekeypad), NULL);
	gtk_container_add(GTK_CONTAINER(window), button);
	gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	show_window_choose_project();
	gtk_main();
}


/* What you need */
GtkWidget *prepare_telekeypad(GtkWindow *parent_window)
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("telephone",
			parent_window, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	return dialog;
}

void open_telekeypad(GtkButton *button, gpointer data)
{
	GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
	GtkWidget *dialog = prepare_telekeypad(GTK_WINDOW(window));
	assert(dialog != NULL);
	//gtk_widget_show_all(dialog);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
	}
	gtk_widget_destroy (dialog);
}
