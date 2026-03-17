/*
 * ComScope - Logger Header
 * Copyright (c) 2026 Prakash Das
 * Licensed under MIT License
 */

#ifndef LOGGER_H
#define LOGGER_H

int logger_start(const char *path);
void logger_write(const char *buf, int n);
void logger_stop(void);
int logger_active(void);
const char *logger_get_log_dir(void);

#endif