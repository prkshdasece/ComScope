#include <ncurses.h>
#include <stdio.h>
#include "tui.h"
#include "config.h"

#define SCROLLBACK_LINES 10000

static WINDOW *output_pad = NULL;
WINDOW *status_win = NULL;

static int current_pad_y = 0;
static int scroll_offset = 0;

static void draw_pad(void)
{
    int view_h = LINES - 1;
    int max_visible_y = current_pad_y;
    
    int top_y = max_visible_y - view_h + 1 - scroll_offset;
    if (top_y < 0) top_y = 0;

    int pad_cols = getmaxx(output_pad);
    int draw_cols = COLS - 1;
    if (draw_cols >= pad_cols) draw_cols = pad_cols - 1;

    prefresh(output_pad, top_y, 0, 0, 0, view_h - 1, draw_cols);
}

void tui_init(TermConfig *cfg)
{
    output_pad = newpad(SCROLLBACK_LINES, COLS);
    scrollok(output_pad, TRUE);
    idlok(output_pad, TRUE);

    status_win = newwin(1, COLS, LINES - 1, 0);
    keypad(status_win, TRUE);

    tui_update_status(cfg, 0);
}

void tui_update_status(TermConfig *cfg, int connected)
{
    werase(status_win);
    wattron(status_win, A_REVERSE);

    /* FIX: Draw up to COLS - 1. Never touch the bottom-right corner! */
    for (int i = 0; i < COLS - 1; i++) waddch(status_win, ' ');

    mvwprintw(status_win, 0, 1, "%s  |  %d baud  |  %s%s  |  Ctrl+A = menu",
        cfg->port,
        (int)cfg->baud,
        connected ? "[CONNECTED]" : "[DISCONNECTED]",
        cfg->log_enabled ? "  |  [LOG]" : ""
    );

    wattroff(status_win, A_REVERSE);
    wrefresh(status_win);
    
    touchwin(output_pad);
    draw_pad(); 
}

void tui_write(const char *buf, int n)
{
    for (int i = 0; i < n; i++) {
        char c = buf[i];
        
        if (c == '\r') {
            /* Just snap cursor to the left edge, don't drop a line */
            int y, x;
            getyx(output_pad, y, x);
            wmove(output_pad, y, 0);
        } else if (c == '\n') {
            /* Standard newline */
            waddch(output_pad, '\n');
        } else {
            /* Normal character */
            waddch(output_pad, (unsigned char)c);
        }
    }
    
    int x;
    getyx(output_pad, current_pad_y, x);

    scroll_offset = 0; 
    draw_pad();
}

void tui_scroll(int direction)
{
    int view_h = LINES - 1;
    int max_scroll = current_pad_y - view_h + 1;
    if (max_scroll < 0) max_scroll = 0;

    if (direction > 0) {
        scroll_offset += (view_h / 2);
    } else {
        scroll_offset -= (view_h / 2);
    }

    if (scroll_offset > max_scroll) scroll_offset = max_scroll;
    if (scroll_offset < 0) scroll_offset = 0;

    touchwin(output_pad);
    draw_pad();
}

int tui_get_char(void)
{
    return wgetch(status_win);
}

void tui_destroy(void)
{
    if (output_pad) { delwin(output_pad); output_pad = NULL; }
    if (status_win) { delwin(status_win); status_win = NULL; }
}