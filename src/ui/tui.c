#include <ncurses.h>
#include <string.h>
#include "tui.h"

#define PAD_ROWS 20000
#define PAD_COLS 1024

WINDOW *pad_win;
WINDOW *status_win;

static int pad_row = 0;     /* last written pad row (0-based) */
static int pad_pos = 0;     /* top row shown in viewport */
static int auto_scroll = 1; /* if 1, follow new incoming data */

static int visible_rows(void)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    /* status window occupies 1 row at bottom, pad uses rows-1 */
    return (rows > 1) ? (rows - 1) : 1;
}

void tui_init(TermConfig *cfg)
{
    (void)cfg;

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors())
        start_color();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    pad_win = newpad(PAD_ROWS, PAD_COLS);
    scrollok(pad_win, TRUE);

    status_win = newwin(1, cols, rows - 1, 0);
    wrefresh(status_win);
}

void tui_destroy(void)
{
    if (status_win) delwin(status_win);
    if (pad_win) delwin(pad_win);
    endwin();
}

int tui_get_char(void)
{
    return getch();
}

/* write incoming bytes into pad and refresh viewport */
void tui_write(const char *buf, int len)
{
    if (!pad_win) return;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int vis = visible_rows();

    /* write chunk using waddch (handles control chars) */
    for (int i = 0; i < len; ++i) {
        char c = buf[i];
        /* Convert CRLF patterns so we don't create empty extra lines */
        if (c == '\r') {
            /* ignore standalone CR */
            continue;
        }
        waddch(pad_win, c);
    }

    /* compute current pad cursor row */
    int y, x;
    getyx(pad_win, y, x);
    if (y < 0) y = 0;
    pad_row = y;

    /* auto-scroll if user is at bottom (auto_scroll == 1) */
    if (auto_scroll) {
        if (pad_row >= vis) {
            pad_pos = pad_row - vis + 1;
            if (pad_pos < 0) pad_pos = 0;
        } else {
            pad_pos = 0;
        }
    } else {
        /* if user had scrolled up, ensure pad_pos not beyond content */
        if (pad_pos > pad_row) pad_pos = pad_row;
    }

    /* refresh pad viewport (top pad_pos -> show through rows-2,
       status window occupies bottom row (rows-1) so last pad row to show is rows-2) */
    int last_display_row = (rows >= 2) ? (rows - 2) : 0;
    prefresh(pad_win, pad_pos, 0, 0, 0, last_display_row, cols - 1);
}

void tui_scroll(int direction)
{
    if (!pad_win) return;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int vis = visible_rows();

    /* user scrolled => disable auto-scroll */
    auto_scroll = 0;

    /* direction positive => scroll up (page up), negative => scroll down (page down) */
    pad_pos += direction * 3;

    if (pad_pos < 0) pad_pos = 0;
    if (pad_pos > pad_row) pad_pos = pad_row;

    int last_display_row = (rows >= 2) ? (rows - 2) : 0;
    prefresh(pad_win, pad_pos, 0, 0, 0, last_display_row, cols - 1);
}

/* Update the reversed status bar at the bottom */
void tui_update_status(TermConfig *cfg, int connected)
{
    if (!status_win) return;

    werase(status_win);
    wattron(status_win, A_REVERSE);

    int cols = 80;
    getmaxyx(status_win, cols, cols); /* harmless attempt to get width */

    mvwprintw(status_win, 0, 0,
              "%s | %d baud | [%s] | Ctrl+A = menu",
              cfg->port,
              cfg->baud,
              connected ? "CONNECTED" : "DISCONNECTED");

    wattroff(status_win, A_REVERSE);
    wrefresh(status_win);
}

/* Called by main when the terminal resizes */
void tui_resize(TermConfig *cfg, int connected)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    if (status_win) {
        wresize(status_win, 1, cols);
        mvwin(status_win, rows - 1, 0);
    }

    /* Ensure visible area recalculated and pad repainted */
    tui_update_status(cfg, connected);

    /* Recompute last_display_row and repaint pad using existing pad_pos */
    int last_display_row = (rows >= 2) ? (rows - 2) : 0;
    prefresh(pad_win, pad_pos, 0, 0, 0, last_display_row, cols - 1);
}