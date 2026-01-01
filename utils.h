#ifndef UTILS_H
#define UTILS_H

void handle_error(const char *msg);
void log_msg(const char *format, ...);
int initialize_socket();
void close_socket(int socket_fd);

#endif // UTILS_H