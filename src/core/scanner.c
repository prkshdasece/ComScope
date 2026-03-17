/*
 * ComScope - Serial Port Terminal for Embedded Development
 * Copyright (c) 2026 Prakash Das
 * 
 * This file is part of ComScope.
 * ComScope is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Prakash Das
 * GitHub: https://github.com/prkshdas/ComScope
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "scanner.h"

int scan_ports(char ports[][270], int max_ports)
{
    DIR *dir = opendir("/dev");
    if (!dir)
        return 0;

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < max_ports)
    {
        char *name = entry->d_name;

        /* scan only ttyUSB, ttyACM, ttyS*/
        if (strncmp(name, "ttyUSB", 6) != 0 &&
            strncmp(name, "ttyACM", 6) != 0 &&
            strncmp(name, "ttyS", 4) != 0)
            continue;

        char path[270];
        snprintf(path, sizeof(path), "/dev/%s", name);
        int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd < 0)
            continue;
        close(fd);

        /* Safely copy path to ports array with explicit size */
        size_t path_len = strlen(path);
        if (path_len < sizeof(ports[count])) {
            strcpy(ports[count], path);  /* Safe because we checked length */
        } else {
            /* Path too long, skip this port */
            continue;
        }
        count++;
    }
    closedir(dir);
    return count; /*return number of the ports found*/
}