/* What you want */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmarshal.h>

#include "telekeypad.h"

//extern void telephone_related();

/* Internal implementation */
#define DIALUP_STR "dialup"
#define ACCEPT_STR "accept"
enum {
	NUM1, NUM2, NUM3,
	NUM4, NUM5, NUM6,
	NUM7, NUM8, NUM9,
	ASTERISK, NUM0,	HASH,
	DIALUP_ACCEPT, SPACE, HANGUP,
	BACKSPACE, CLEAR
};
static struct button_appearance {
	gchar *label;
	gchar *iconname;
} bappear[] = {
	{"1", NULL}, {"2", NULL}, {"3", NULL},
	{"4", NULL}, {"5", NULL}, {"6", NULL},
	{"7", NULL}, {"8", NULL}, {"9", NULL},
	{"*", NULL}, {"0", NULL}, {"#", NULL},
	{NULL, "dialup.png"}, {"space", NULL}, {"hangup", "hangup.png"},
	{"backspace", "backspace.png"}, {"clear", "clear.png"}
};
static GtkWidget *entry = NULL;
static int iscallin = 0;
static const char *callin_number = NULL;
static char icon_path[1025] = {0};
static enum { CONNECTING, CONNECTED, DISCONNECT } tele_state = DISCONNECT;

inline static void telekeypad_set_conn_state(int state)
{
	tele_state = state;
	if (GTK_IS_ENTRY(entry))
		gtk_editable_set_editable(GTK_EDITABLE(entry),
				tele_state != CONNECTING);
}

static inline void dialup_accept(const gchar *telenumber)
{
	if (iscallin) {	/* accept */
		g_printf("accept telephone number: %s\n", telenumber);
		telekeypad_set_conn_state(CONNECTED);
		iscallin = 0;
	} else {	/* dialup */
		telekeypad_set_conn_state(CONNECTING);
		g_printf("dialup with telephone number: %s\n", telenumber);
		telekeypad_set_conn_state(CONNECTED);
	}
}

static inline void hangup()
{
	if (tele_state != DISCONNECT) {
		g_printf("hangup\n");
	}
	telekeypad_set_conn_state(DISCONNECT);
	gtk_editable_delete_text(GTK_EDITABLE(entry), 0, -1);
	iscallin = 0;
}

/* Event callback handler
 * ================ */
static void entry_insert(GtkEditable *editable,
		gchar *new_text, gint new_text_length,
		gint *position, gpointer user_data)
{
	/* dialup string you input or paste if connected */
	if (tele_state == CONNECTED)
		dialup_accept(new_text);
}

static void entry_activate(GtkEntry *entry, GtkButton *button)
{
	/*g_signal_emit_by_name(button, "clicked");*/
	gtk_widget_activate(GTK_WIDGET(button));
}

static void click_event(GtkButton *button, char *string)
{
	GtkEditable *editable = GTK_EDITABLE(entry);

	/* parse some strings */
	if (strcmp(string, bappear[DIALUP_ACCEPT].label) == 0) {
		dialup_accept(gtk_entry_get_text(GTK_ENTRY(entry)));
		return;
	} else if (strcmp(string, bappear[HANGUP].label) == 0) {
		hangup();
		return;
	} else if (gtk_editable_get_editable(editable)) {
		int startpos, endpos;
		gtk_editable_get_selection_bounds(editable, &startpos, &endpos);
		gtk_editable_delete_selection(editable);
		int textpos = gtk_editable_get_position(editable);
		if (strcmp(string, bappear[SPACE].label) == 0)
			string = " ";
		else if (strcmp(string, bappear[BACKSPACE].label) == 0) {
			if (startpos == endpos) {
				gtk_editable_delete_text(editable,
						textpos-1, textpos);
				textpos--;
			}
			goto skip_insert;
		} else if (strcmp(string, bappear[CLEAR].label) == 0) {
			gtk_editable_delete_text(editable, textpos = 0, -1);
			goto skip_insert;
		}
		gtk_editable_insert_text(editable, string,
				strlen(string), &textpos);
skip_insert:
		gtk_widget_grab_focus(entry);
		gtk_editable_set_position(editable, textpos);
	}
}
/* ================ */

static GtkWidget *prepare_icon(int buttonid)
{
	int len = strlen(icon_path);
	GError *error = NULL;
	gchar *iconname = bappear[buttonid].iconname;
	if (iconname == NULL)
		return NULL;
	strncat(icon_path, iconname, sizeof(icon_path)-1);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(
			icon_path, 22, 22, &error);
	if (pixbuf == NULL)
		return NULL;
	icon_path[len] = 0;	/* resume icon path */
	return gtk_image_new_from_pixbuf(pixbuf);
}

static GtkWidget *prepare_telekeypad(GtkWindow *parent_window)
{
	/* New dialog */
	GtkWidget *dialog, *content_area;
	gchar title[1025] = "call out";
	if (iscallin)
		snprintf(title, sizeof(title)-1,
				"%s is calling you", callin_number);
	dialog = gtk_dialog_new_with_buttons(title,
			parent_window, GTK_DIALOG_DESTROY_WITH_PARENT,
			//GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	/* New vbox which will be added into dialog */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 5);

#define ROWS 5
#define COLS 3
#define TOTAL (ROWS*COLS)
	/* prepare image first */
	GtkWidget *imagep, *image[TOTAL+2] = {0};
	image[DIALUP_ACCEPT] = prepare_icon(DIALUP_ACCEPT);
	image[HANGUP] = prepare_icon(HANGUP);
	image[BACKSPACE] = prepare_icon(BACKSPACE);
	image[CLEAR] = prepare_icon(CLEAR);

	/* Part1 of vbox: New buttons and layout */
	GtkWidget *table, *button[TOTAL];
	int i, j, idx = 0, rows = ROWS, cols = COLS;
	table = gtk_table_new(rows, cols, TRUE);
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			gchar *strval = NULL;
			/* new buttons and place them by table */
			if (image[idx] == NULL)
				button[idx] =
				gtk_button_new_with_label(bappear[idx].label);
			else {
				button[idx] = gtk_button_new();
				gtk_button_set_image(GTK_BUTTON(button[idx]),
					image[idx]);
			}
			g_signal_connect(G_OBJECT(button[idx]), "clicked",
					G_CALLBACK(click_event),
					bappear[idx].label);
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
	if (iscallin) {
		gtk_entry_set_text(GTK_ENTRY(entry), callin_number);
		gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	}
	/* new button backspace */
	imagep = image[BACKSPACE];
	if (imagep == NULL)
		backspace = GTK_BUTTON(gtk_button_new_with_label(
					bappear[BACKSPACE].label));
	else {
		backspace = GTK_BUTTON(gtk_button_new());
		gtk_button_set_image(backspace, imagep);
	}
	/* new button clear */
	imagep = image[CLEAR];
	if (imagep == NULL)
		clear = GTK_BUTTON(gtk_button_new_with_label(
					bappear[CLEAR].label));
	else {
		clear = GTK_BUTTON(gtk_button_new());
		gtk_button_set_image(clear, imagep);
	}

	g_signal_connect(G_OBJECT(backspace), "clicked",
			G_CALLBACK(click_event), bappear[BACKSPACE].label);
	g_signal_connect(G_OBJECT(clear), "clicked",
			G_CALLBACK(click_event), bappear[CLEAR].label);
	g_signal_connect(G_OBJECT(entry), "activate",
			G_CALLBACK(entry_activate), button[DIALUP_ACCEPT]);
	/* The insert-at-cursor callback of GtkEntry is not
	 * called when performing insert, I don't know why.
	 * Use insert-text signal of GtkEditable instead. */
	g_signal_connect(G_OBJECT(entry), "insert-text",
			G_CALLBACK(entry_insert), NULL);

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

static void open_telekeypad(GtkWindow *window, gpointer data)
{
	GtkWidget *dialog = prepare_telekeypad(GTK_WINDOW(window));
	assert(dialog != NULL);
	//gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));

	/* Here dialog is closed. Don't change order of cleanup routines. */
	hangup();
	gtk_widget_destroy (dialog);
	entry = NULL;
}

/* External Interface */
void telekeypad_click(GtkButton *button, gpointer data)
{
	iscallin = 0;
	bappear[DIALUP_ACCEPT].label = DIALUP_STR;
	GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
	open_telekeypad(GTK_WINDOW(window), data);
}

void telekeypad_callin(const char *telenumber)
{
	iscallin = 1;	/* indicate telekeypad will be open due to callin */
	callin_number = telenumber;
	assert(callin_number != NULL);
	bappear[DIALUP_ACCEPT].label = ACCEPT_STR;
	open_telekeypad(NULL, NULL);
}

void telekeypad_set_icon_path(const char *path)
{
	strncpy(icon_path, path, sizeof(icon_path)-1);
}

void telekeypad_set_connecting()
{
	telekeypad_set_conn_state(CONNECTING);
}
void telekeypad_set_connected()
{
	telekeypad_set_conn_state(CONNECTED);
}
void telekeypad_set_disconnect()
{
	telekeypad_set_conn_state(DISCONNECT);
}

