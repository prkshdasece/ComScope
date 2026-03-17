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

#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "logger.h"

static int log_fd = -1;

int logger_start(const char *path)
{
    log_fd = open(path,
                  O_WRONLY | O_CREAT | O_APPEND,
                  0644);
    if (log_fd < 0)
        return -1;

    /* write timestamp header */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char header[64];
    strftime(header, sizeof(header),
             "\n--- Session started %Y-%m-%d %H:%M:%S ---\n", t);
    write(log_fd, header, strlen(header));
    return 0;
}

void logger_write(const char *buf, int n)
{
    if (log_fd < 0)
        return;

    ssize_t written = write(log_fd, buf, n);
    (void)written; /* Avoid unused variable warning */

    /* Flush data regularly to ensure it's written to disk */
    fsync(log_fd);
}

void logger_stop(void)
{
    if (log_fd < 0)
        return;

    /* write closing footer */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char footer[64];
    strftime(footer, sizeof(footer),
             "--- Session ended   %Y-%m-%d %H:%M:%S ---\n", t);
    write(log_fd, footer, strlen(footer));

    /* FIX: Ensure data is synced before closing */
    fsync(log_fd);
    close(log_fd);
    log_fd = -1;
}

int logger_active(void)
{
    return log_fd >= 0; /* FIX: Corrected logic */
}