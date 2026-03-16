#pragma once

#include <ncurses.h>
#include "config.h"

/* windows used across modules */
extern WINDOW *pad_win;
extern WINDOW *status_win;

/* init / shutdown */
void tui_init(TermConfig *cfg);
void tui_destroy(void);

/* screen updates */
void tui_write(const char *buf, int len);
void tui_scroll(int direction);
void tui_update_status(TermConfig *cfg, int connected);
void tui_resize(TermConfig *cfg, int connected);

/* input */
int tui_get_char(void);