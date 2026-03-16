#include <ncurses.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "portpicker.h"
#include "configpanel.h"
#include "config.h"
#include "port.h"
#include "engine.h"
#include "logger.h"

static void handle_resize(int sig)
{
    (void)sig;
    endwin();
    refresh();
    clear();
}

int main(void)
{
    signal(SIGWINCH, handle_resize);
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    /* 1 — pick port */
    char *port = pick_port();
    if (!port) { endwin(); printf("Goodbye.\n"); return 0; }

    /* 2 — configure */
    TermConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    strncpy(cfg.port, port, sizeof(cfg.port) - 1);
    
    /* Initialize Logger settings */
    strncpy(cfg.log_path, "session.txt", sizeof(cfg.log_path) - 1);
    cfg.log_enabled = 0;

    if (show_config(&cfg) < 0) { endwin(); printf("Cancelled.\n"); return 0; }

    /* 3 — open serial port */
    int fd = open_serial(&cfg);
    if (fd < 0) {
        endwin();
        printf("Failed to open %s\n", cfg.port);
        printf("Tip: sudo usermod -aG dialout $USER\n");
        return 1;
    }

    /* 4 — run the terminal */
    run_engine(fd, &cfg);

    /* 5 — clean up */
    close_serial(fd);
    
    /* Ensure log file is closed gracefully if left running */
    if (logger_active()) {
        logger_stop();
    }
    
    endwin();
    printf("ComScope closed. Goodbye.\n");
    return 0;
}