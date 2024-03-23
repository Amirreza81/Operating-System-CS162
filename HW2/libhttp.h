#ifndef LIBHTTP_H
#define LIBHTTP_H

struct http_request {
    char *method;
    char *path;
};

struct http_request *http_request_parse(int fd);

void http_start_response(int fd, int status_code);

void http_send_header(int fd, char *key, char *value);

void http_end_headers(int fd);

void http_send_string(int fd, char *data);

void http_send_data(int fd, char *data, size_t size);

char *http_get_mime_type(char *file_name);

#endif

