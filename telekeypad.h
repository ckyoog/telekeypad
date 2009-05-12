#ifndef _TELEKEYPAD_H_
#define _TELEKEYPAD_H_

/* Parameters explaination of two main functions below:
 * w: the main window which telekeypad dialog belongs to, can be NULL.
 * data: anything you want to pass to telekeypad dialog, but currently
 * 	 is useless, can be NULL.
 */
/* call this if you want to call out to somebody */
void telekeypad_callout(GtkWindow *w, gpointer data);
/* call this when call is incomming */
void telekeypad_callin(const gchar *telenumber, GtkWindow *w, gpointer data);


/* routines used for telling connect state to telekeypad dialog */
void telekeypad_set_connecting();
void telekeypad_set_connected();
void telekeypad_set_disconnect();

/* helper for load icon which is on button */
void telekeypad_set_icon_path(const char *path);

#endif	/* _TELEKEYPAD_H_ */
