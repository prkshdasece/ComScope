#ifndef ENGINE_H
#define ENGINE_H

#include "config.h"

/* Runs the main I/O loop until user quits.
   serial_fd is the open file descriptor from open_serial(). */
void run_engine(int serial_fd, TermConfig *cfg);

#endif