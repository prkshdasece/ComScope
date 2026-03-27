/*
 * ComScope - Configuration Header
 * Copyright (c) 2026 Prakash Das
 * Licensed under MIT License
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <termios.h>

typedef struct
{
    char port[270]; /*/dev/ttyXYZ*/
    speed_t baud;   /*baudrate*/
    int baud_index;  /*index for baud*/
    int databits;   /*5,6,7 or 8*/
    int parity;     /*0=none, 1=odd, 2=even*/
    int stopbits;

    /* Logging features */
    int log_enabled;
    char log_path[256];
} TermConfig;

#endif