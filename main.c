#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmarshal.h>

#include "helper.header"
#include "telekeypad.h"

#define PATH_CHAR_MAX 1025
#define PROCNAME "telekeypad"

static void close_window(GtkButton *button, G_GNUC_UNUSED gpointer user_data)
{
	GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
	gtk_widget_destroy(GTK_WIDGET(window));
}

#ifndef FOR_MY_OWN_USE
static GtkWidget *status_button = NULL;
void set_status_button_sensitive()
{
	if (GTK_IS_WIDGET(status_button))
		gtk_widget_set_sensitive(status_button, TRUE);
}
void set_status_button_insensitive()
{
	if (GTK_IS_WIDGET(status_button))
		gtk_widget_set_sensitive(status_button, FALSE);
}
#endif
void telecall(GtkButton *button, const gchar *label)
{
	GtkWindow *window;
#ifndef FOR_MY_OWN_USE
	status_button = GTK_WIDGET(button);
#endif
	window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)));
	if (strcmp(label, "callout") == 0)
		telekeypad_callout(window, NULL);
	else if (strcmp(label, "callin") == 0)
		telekeypad_callin("13811091234", window, NULL);
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
#if 0
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
#endif

	GtkWidget *window, *callout, *callin, *hbox;
	window = GTK_WIDGET(prepare_window());
	hbox = gtk_hbox_new(TRUE, 5);
	gtk_container_add(GTK_CONTAINER(window), hbox);

#define BUTTON_NEW_SIGNAL_AND_ADD(buttonname) \
	buttonname = gtk_button_new_with_label(#buttonname);\
	g_signal_connect(G_OBJECT(buttonname), "clicked",\
			G_CALLBACK(telecall), #buttonname);\
	gtk_container_add(GTK_CONTAINER(hbox), buttonname);
	BUTTON_NEW_SIGNAL_AND_ADD(callout);
	BUTTON_NEW_SIGNAL_AND_ADD(callin);
#undef BUTTON_NEW_SIGNAL_AND_ADD

	gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	show_window_choose_project();
	gtk_main();
}

