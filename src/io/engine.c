#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include "engine.h"
#include "tui.h"
#include "config.h"
#include "logger.h"

#define BUF_SIZE 512
#define CMD_ESCAPE 0x01   /* Ctrl+A */

static void handle_command_mode(TermConfig *cfg, int serial_fd)
{
    /* Show menu in status via tui_update_status + a short prompt */
    tui_update_status(cfg, 1);

    /* draw a small inline prompt on status line */
    mvwprintw(status_win, 0, 1, "CMD: l=toggle log   q=quit   Esc=cancel");
    wrefresh(status_win);

    /* FIX: Set longer timeout for command mode */
    tui_set_timeout(1000);  /* 1 second timeout for menu */
    
    int ch = tui_get_char();

    /* Restore normal timeout */
    tui_set_timeout(50);

    if (ch == 'l' || ch == 'L') {
        if (!logger_active()) {
            if (logger_start(cfg->log_path) == 0)
                cfg->log_enabled = 1;
        } else {
            logger_stop();
            cfg->log_enabled = 0;
        }
    }
    else if (ch == 'q' || ch == 'Q' || ch == 27) {  /* ESC or q to exit */
        ungetch('q');
        return;
    }

    /* refresh status */
    tui_update_status(cfg, 1);
}

void run_engine(int serial_fd, TermConfig *cfg)
{
    tui_init(cfg);
    tui_set_timeout(50);  /* FIX: Set initial timeout */
    tui_update_status(cfg, 1);

    struct pollfd fds[2];
    char buf[BUF_SIZE];

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = serial_fd;
    fds[1].events = POLLIN | POLLERR | POLLHUP;

    int in_command_mode = 0;  /* Track if we're in command mode */

    while (1) {
        /* Use very small timeout for keyboard responsiveness */
        int ret = poll(fds, 2, 30);  /* 30ms - ultra responsive */
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        if (ret == 0) {
            /* timeout — do nothing, just loop again */
            continue;
        }

        /* keyboard input - HIGHEST PRIORITY */
        if (fds[0].revents & POLLIN) {
            int ch = tui_get_char();

            /* FIX: Proper Ctrl+A detection (0x01 in raw mode) */
            if (ch == CMD_ESCAPE) {
                in_command_mode = 1;
                handle_command_mode(cfg, serial_fd);
                in_command_mode = 0;
                
                /* After command mode, check for 'q' request */
                int next = tui_get_char();
                if (next == 'q' || next == 'Q') break;
                if (next != ERR) ungetch(next);
            }
            else if (ch == KEY_PPAGE) {
                tui_scroll(1);
            }
            else if (ch == KEY_NPAGE) {
                tui_scroll(-1);
            }
            else if (ch == 'q' || ch == 'Q') {
                break;
            }
            else if (ch > 0 && ch != ERR) {
                /* FIX: Send valid printable characters to serial */
                if (isprint(ch) || ch == '\n' || ch == '\r' || ch == '\t' || ch == 0x08) {
                    char c = (char)ch;
                    ssize_t w = write(serial_fd, &c, 1);
                    if (w < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            perror("serial write failed");
                        }
                    }
                }
            }
        }

        /* serial input - SECONDARY PRIORITY */
        if (fds[1].revents & POLLIN) {
            ssize_t n = read(serial_fd, buf, sizeof(buf));
            if (n > 0) {
                /* send to UI and logger */
                tui_write(buf, (int)n);
                if (cfg->log_enabled) logger_write(buf, (int)n);
            } else if (n == 0) {
                /* 0 bytes = hangup/closed */
                tui_update_status(cfg, 0);
                break;
            } else {
                /* read error */
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) 
                    continue;
                perror("serial read failed");
                break;
            }
        }

        if (fds[1].revents & (POLLHUP | POLLERR)) {
            tui_update_status(cfg, 0);
            break;
        }
    }

    tui_destroy();
}