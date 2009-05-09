#ifndef _TELEKEYPAD_H_
#define _TELEKEYPAD_H_

#include <assert.h>
#include <locale.h>

#if GTK_CHECK_VERSION(2,4,0)
#include <glib/gi18n.h>
#define __(str) ({ \
	assert(str);\
	gchar *t = g_locale_to_utf8(_(str), -1, NULL, NULL, NULL);\
	if (t == NULL) {\
		t = malloc(strlen(str)+1);\
		strcpy(t, (str));\
	}\
	t;\
})
#else
#define __(str) (str)
#endif

void show_window_choose_project();

#define  message_dialog(parent, type, buttons, title, message_format...) do {\
	GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,\
			type, buttons, message_format);\
	gtk_window_set_title(GTK_WINDOW(dialog), title);\
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);\
	gtk_dialog_run(GTK_DIALOG(dialog));\
	gtk_widget_destroy(dialog);\
} while (0)

#endif	/* _TELEKEYPAD_H_ */
