#ifndef TUI_H
#define TUI_H

#include <ncurses.h>
#include "config.h"

extern WINDOW *status_win;

void tui_init(TermConfig *cfg);
void tui_update_status(TermConfig *cfg, int connected);
void tui_destroy(void);

/* Phase 4 additions for formatting and scrollback */
void tui_write(const char *buf, int n);
void tui_scroll(int direction);
int  tui_get_char(void);

#endif