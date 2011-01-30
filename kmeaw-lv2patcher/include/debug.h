#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"

int debug_sock;
char debug_buffer[1024];

ssize_t debug_print_to_socket(int sockd, const void *vptr);
void debug_wait_for_client();

#if DEBUG
#define PRINTF(fmt, args...) sprintf(debug_buffer, fmt, ## args); debug_print_to_socket(debug_sock, debug_buffer);
#else
#define PRINTF(fmt, args...)
#endif

#endif