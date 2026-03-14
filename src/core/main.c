#include <ncurses.h>
#include <unistd.h>

int main(void)
{
    initscr();            /*start ncurses*/
    cbreak();             /*pass kewpress immediately*/
    noecho();             /*don't echo typed characters*/
    keypad(stdscr, TRUE); /*enable arrow keys*/
    curs_set(0);          /*hide cursor*/

    mvprintw(0, 0, "ComScope v0.1 -- press any key to exit");
    mvprintw(2, 0, "LINES=%d COLS=%d", LINES, COLS);
    refresh();

    getch();  /*wait for any keypress*/
    endwin(); /*restore terminal*/
    return 0;
}