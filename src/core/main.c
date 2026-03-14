#include <ncurses.h>
#include <unistd.h>
#include <signal.h>
#include "scanner.h"

static void handle_resize(int sig)
{
    (void)sig; /*suppress unused warning*/
    endwin();  /*let nucurses release the screen*/
    refresh(); /*reinitialise with new dimensions*/
    clear();   /*clear stale content*/
}

int main(void)
{
    signal(SIGWINCH, handle_resize); /*register handler before initscr*/
    initscr();                       /*start ncurses*/
    cbreak();                        /*pass kewpress immediately*/
    noecho();                        /*don't echo typed characters*/
    keypad(stdscr, TRUE);            /*enable arrow keys*/
    curs_set(0);                     /*hide cursor*/

    /* --- temporary scanner test --- */
    char ports[MAX_PORTS][64];
    int count = scan_ports(ports, MAX_PORTS);

    mvprintw(0, 0, "ComScope v0.1 -- Serial Port Scanner");
    mvprintw(1, 0, "------------------------------------");

    if (count == 0)
    {
        mvprintw(3, 0, "No serial ports found.");
        mvprintw(4, 0, "Tip: run  socat -d -d pty,raw,echo=0 pty,raw,echo=0");
        mvprintw(5, 0, "     in another terminal to create virtual ports.");
    }
    else
    {
        mvprintw(3, 0, "Found %d port(s):", count);
        for (int i = 0; i < count; i++)
            mvprintw(4 + i, 2, "%s", ports[i]);
    }

    mvprintw(LINES - 1, 0, "Press any key to exit...");
    refresh();

    getch();  /*wait for any keypress*/
    endwin(); /*restore terminal*/
    return 0;
}