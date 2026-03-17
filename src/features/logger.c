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
#include <stdlib.h>
#include <sys/stat.h>
#include "logger.h"

static int log_fd = -1;

/* Create logs directory if it doesn't exist */
static int ensure_log_directory(void)
{
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Error: HOME environment variable not set\n");
        return -1;
    }

    /* Build path: ~/Documents/ComScope */
    char log_dir[1024];
    snprintf(log_dir, sizeof(log_dir), "%s/Documents/ComScope", home);

    /* Create Documents directory if needed */
    char docs_dir[1024];
    snprintf(docs_dir, sizeof(docs_dir), "%s/Documents", home);
    mkdir(docs_dir, 0755);  /* Create if doesn't exist, ignore if already exists */

    /* Create ComScope directory */
    if (mkdir(log_dir, 0755) < 0) {
        /* Directory might already exist, which is fine */
        /* Check if it's actually a directory */
        struct stat st;
        if (stat(log_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Error: Cannot create log directory: %s\n", log_dir);
            return -1;
        }
    }

    return 0;
}

/* Generate timestamped log filename */
static int generate_log_filename(char *filename, size_t size)
{
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Error: HOME environment variable not set\n");
        return -1;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    /* Format: ~/Documents/ComScope/ComScope_2026-03-17_14-23-45.txt */
    int ret = snprintf(filename, size, 
             "%s/Documents/ComScope/ComScope_%04d-%02d-%02d_%02d-%02d-%02d.txt",
             home,
             t->tm_year + 1900,
             t->tm_mon + 1,
             t->tm_mday,
             t->tm_hour,
             t->tm_min,
             t->tm_sec);

    if (ret < 0 || (size_t)ret >= size) {
        fprintf(stderr, "Error: Log filename too long\n");
        return -1;
    }

    return 0;
}

int logger_start(const char *path)
{
    (void)path;  /* Ignore the passed path, use auto-generated one */

    /* Ensure log directory exists */
    if (ensure_log_directory() < 0) {
        fprintf(stderr, "Warning: Could not create log directory\n");
        return -1;
    }

    /* Generate timestamped filename */
    char log_filename[1024];
    if (generate_log_filename(log_filename, sizeof(log_filename)) < 0) {
        fprintf(stderr, "Warning: Could not generate log filename\n");
        return -1;
    }

    /* Open log file (create if doesn't exist, append if it does) */
    log_fd = open(log_filename,
                  O_WRONLY | O_CREAT | O_APPEND,
                  0644);
    if (log_fd < 0) {
        fprintf(stderr, "Warning: Could not open log file: %s\n", log_filename);
        return -1;
    }

    /* Write session header with full path info */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char header[512];
    int header_len = snprintf(header, sizeof(header),
             "\n========================================\n"
             "ComScope Session Started\n"
             "Date: %04d-%02d-%02d\n"
             "Time: %02d:%02d:%02d\n"
             "========================================\n\n",
             t->tm_year + 1900,
             t->tm_mon + 1,
             t->tm_mday,
             t->tm_hour,
             t->tm_min,
             t->tm_sec);
    
    if (header_len > 0 && (size_t)header_len < sizeof(header)) {
        ssize_t written = write(log_fd, header, (size_t)header_len);
        (void)written;
    }

    /* Flush to ensure it's written */
    fsync(log_fd);

    return 0;
}

void logger_write(const char *buf, int n)
{
    if (log_fd < 0) return;
    
    /* Write data to log file */
    ssize_t written = write(log_fd, buf, (size_t)n);
    (void)written;
    
    /* Flush periodically to ensure data is saved */
    fsync(log_fd);
}

void logger_stop(void)
{
    if (log_fd < 0) return;

    /* Write session footer */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char footer[512];
    int footer_len = snprintf(footer, sizeof(footer),
             "\n========================================\n"
             "ComScope Session Ended\n"
             "Date: %04d-%02d-%02d\n"
             "Time: %02d:%02d:%02d\n"
             "========================================\n\n",
             t->tm_year + 1900,
             t->tm_mon + 1,
             t->tm_mday,
             t->tm_hour,
             t->tm_min,
             t->tm_sec);
    
    if (footer_len > 0 && (size_t)footer_len < sizeof(footer)) {
        ssize_t written = write(log_fd, footer, (size_t)footer_len);
        (void)written;
    }
    
    /* Ensure all data is written before closing */
    fsync(log_fd);
    close(log_fd);
    log_fd = -1;
}

int logger_active(void)
{
    return log_fd >= 0;
}

/* Get the current log directory path */
const char *logger_get_log_dir(void)
{
    const char *home = getenv("HOME");
    if (!home) return NULL;
    
    static char log_dir[1024];
    snprintf(log_dir, sizeof(log_dir), "%s/Documents/ComScope", home);
    return log_dir;
}