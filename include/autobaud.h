#ifndef AUTOBAUD_H
#define AUTOBAUD_H

/* Returns the detected baud rate (e.g., 115200) or 0 if detection fails */
int detect_baudrate(const char *port);

#endif