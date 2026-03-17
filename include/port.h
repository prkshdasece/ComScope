/*
 * ComScope - Configuration Header
 * Copyright (c) 2026 Prakash Das
 * Licensed under MIT License
 */

#ifndef PORT_H
#define PORT_H

#include "config.h"

/*Open serial port*/
int open_serial(TermConfig *cfg);

/*Close serial port*/
void close_serial(int fd);

#endif