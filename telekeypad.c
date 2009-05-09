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


/* What you want */

enum {
	NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
	ASTERISK, NUM0,	HASH, DIALUP, SPACE, HANGUP,
	BACKSPACE, CLEAR
};
char *label[] = {"1", "2", "3", "4", "5", "6", \
	"7", "8", "9", "*", "0", "#", "dialup", "space", "hangup", \
	"backspace", "clear"};
GtkWidget *entry;

inline void dialup(const gchar *telenumber)
{
	g_printf("dialup with telephone number: %s\n", telenumber);
}

inline void hangup()
{
	g_printf("hangup\n");
}

void click_event(GtkButton *button, char *string)
{
	GtkEditable *editable = GTK_EDITABLE(entry);
	int textpos = gtk_editable_get_position(editable);

	/* parse some strings */
	if (strcmp(string, label[SPACE]) == 0)
		string = " ";
	else if (strcmp(string, label[BACKSPACE]) == 0) {
		gtk_editable_delete_text(editable, textpos-1, -1);
		goto skip_insert;
	} else if (strcmp(string, label[CLEAR]) == 0) {
		gtk_editable_delete_text(editable, 0, -1);
		goto skip_insert;
	} else if (strcmp(string, label[DIALUP]) == 0) {
		dialup(gtk_entry_get_text(GTK_ENTRY(entry)));
		return;
	} else if (strcmp(string, label[HANGUP]) == 0) {
		hangup();
		gtk_editable_delete_text(editable, 0, -1);
		return;
	}
	gtk_editable_insert_text(editable, string, strlen(string), &textpos);
	gtk_editable_set_position(editable, textpos);
skip_insert:
	return;
}

GtkWidget *prepare_telekeypad(GtkWindow *parent_window)
{
	/* New dialog */
	GtkWidget *dialog, *content_area;
	dialog = gtk_dialog_new_with_buttons("telephone",
			parent_window, GTK_DIALOG_DESTROY_WITH_PARENT,
			//GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	/* New vbox which will be added into dialog */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 5);

	/* Part1 of vbox: New buttons and layout */
	GtkWidget *table, *button[15];
	int i, j, idx = 0, rows = 5, cols = 3;
	table = gtk_table_new(rows, cols, TRUE);
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			/* new buttons and place them by table */
			button[idx] = gtk_button_new_with_label(label[idx]);
			g_signal_connect(G_OBJECT(button[idx]), "clicked",
					G_CALLBACK(click_event), label[idx]);
			gtk_table_attach_defaults(GTK_TABLE(table),
					button[idx], j, j+1, i, i+1);
			idx++;
		}
	}

	/* Part2 of vbox: New entry and two buttons */
	GtkWidget *align, *hbox;
	GtkButton *backspace, *clear;
	align = gtk_alignment_new(0, 0, 1, 1);
	hbox = gtk_hbox_new(FALSE, 3);

	entry = gtk_entry_new();
	backspace = GTK_BUTTON(gtk_button_new_with_label(label[BACKSPACE]));
	clear = GTK_BUTTON(gtk_button_new_with_label(label[CLEAR]));

	g_signal_connect(G_OBJECT(backspace), "clicked",
			G_CALLBACK(click_event), label[BACKSPACE]);
	g_signal_connect(G_OBJECT(clear), "clicked",
			G_CALLBACK(click_event), label[CLEAR]);
	g_signal_connect(G_OBJECT(entry), "activate",
			G_CALLBACK(entry_activate), button[DIALUP]);

	gtk_container_add(GTK_CONTAINER(hbox), entry);
	gtk_container_add(GTK_CONTAINER(hbox), GTK_WIDGET(backspace));
	gtk_container_add(GTK_CONTAINER(hbox), GTK_WIDGET(clear));
	gtk_container_add(GTK_CONTAINER(align), hbox);
	gtk_widget_show_all(align);

	/* Add Part2 into vbox first */
	gtk_container_add(GTK_CONTAINER(vbox), align);
	/* Then add Part1 into vbox */
	gtk_container_add(GTK_CONTAINER(vbox), table);
	/* Add vbox into content area of dialog */
	gtk_container_add(GTK_CONTAINER(content_area), vbox);
	gtk_widget_show_all(vbox);
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

