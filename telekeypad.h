#ifndef _TELEKEYPAD_H_
#define _TELEKEYPAD_H_

/* button click event callback */
void telekeypad_click(GtkButton *button, gpointer data);

/* call this when telephone call is incomming */
void telekeypad_callin(const gchar *telenumber);

/* helper for load icon which is on button */
void telekeypad_set_icon_path(const char *path);

/* routines for set connect state */
void telekeypad_set_connecting();
void telekeypad_set_connected();
void telekeypad_set_disconnect();

#endif	/* _TELEKEYPAD_H_ */
