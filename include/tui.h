/*
 * ComScope - Configuration Header
 * Copyright (c) 2026 Prakash Das
 * Licensed under MIT License
 */

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

/* input - with proper non-blocking support */
int tui_get_char(void);
void tui_set_timeout(int timeout_ms);