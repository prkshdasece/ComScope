#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>
#include "engine.h"
#include "tui.h"
#include "config.h"
#include "logger.h"

#define BUF_SIZE 256
#define CMD_ESCAPE 0x01   /* Ctrl+A */

static void handle_command_mode(TermConfig *cfg, int serial_fd)
{
    (void)serial_fd;

    werase(status_win);
    wattron(status_win, A_REVERSE);
    mvwprintw(status_win, 0, 1,
        "CMD: l=toggle log   q=quit   Esc=cancel");
    wattroff(status_win, A_REVERSE);
    wrefresh(status_win);

    int ch = tui_get_char();

    if (ch == 'l' || ch == 'L') {
        if (!logger_active()) {
            if (logger_start(cfg->log_path) == 0) cfg->log_enabled = 1;
        } else {
            logger_stop();
            cfg->log_enabled = 0;
        }
    } else if (ch == 'q' || ch == 'Q') {
        ungetch('q');
        return;
    }

    tui_update_status(cfg, 1);
}

void run_engine(int serial_fd, TermConfig *cfg)
{
    tui_init(cfg);
    tui_update_status(cfg, 1);

    struct pollfd fds[2];
    fds[0].fd     = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd     = serial_fd;
    fds[1].events = POLLIN;

    char buf[BUF_SIZE];

    while (1) {
        int ret = poll(fds, 2, -1);
        if (ret < 0) break;

        /* ── keyboard ready ── */
        if (fds[0].revents & POLLIN) {
            int ch = tui_get_char();

            if (ch == CMD_ESCAPE) {
                handle_command_mode(cfg, serial_fd);
                int next = tui_get_char();
                if (next == 'q' || next == 'Q') break;
                ungetch(next);
            } else if (ch == KEY_PPAGE) {
                tui_scroll(1); /* Page Up key */
            } else if (ch == KEY_NPAGE) {
                tui_scroll(-1); /* Page Down key */
            } else if (ch == 'q') {
                break;
            } else {
                char c = (char)ch;
                write(serial_fd, &c, 1);
            }
        }

        /* ── serial port ready ── */
        if (fds[1].revents & POLLIN) {
            int n = read(serial_fd, buf, BUF_SIZE);
            if (n > 0) {
                /* Write to TUI with auto-formatting */
                tui_write(buf, n);

                /* Write to log file */
                if (cfg->log_enabled)
                    logger_write(buf, n);
            } else if (n == 0) {
                tui_update_status(cfg, 0);
                break;
            }
        }

        if (fds[1].revents & POLLHUP) {
            tui_update_status(cfg, 0);
            break;
        }
    }

    tui_destroy();
}