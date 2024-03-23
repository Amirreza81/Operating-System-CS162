#include <signal.h>
#include <unistd.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "wq.h"
#include "libhttp.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>


/*
 * Global configuration variables.
 * You need to use these in your implementation of handle_files_request and
 * handle_proxy_request. Their values are set up in main() using the
 * command line arguments (already implemented for you).
 */
wq_t work_queue;
int num_threads;
int server_port;
char *server_files_directory;
char *server_proxy_hostname;
int server_proxy_port;

/*
 * Serves the contents the file stored at `path` to the client socket `fd`.
 * It is the caller's reponsibility to ensure that the file stored at `path` exists.
 * You can change these functions to anything you want.
 *
 * ATTENTION: Be careful to optimize your code. Judge is
 *            sesnsitive to time-out errors.
 */
// ###########################################################################
void send_http_response(int fd,
                        int status_code,
                        char *content_type,
                        char *content_length,
                        char *content) {
    http_start_response(fd, status_code);
    http_send_header(fd, "Content-Type", content_type);
    http_send_header(fd, "Content-Length", content_length);
    http_end_headers(fd);
    http_send_string(fd, content);
}


char *long_to_string(unsigned long number) {
    char *int_str = malloc(sizeof(char) * 32);
    sprintf(int_str, "%ld", number);
    return int_str;
}

char *get_file_size(char *path) {
    struct stat s;
    stat(path, &s);
    return long_to_string(s.st_size);
}

// ###########################################################################
char *link_to_dir_contents(char *path) {
    char *content = malloc(sizeof(char) * 1 << 20);
    char a = "href";
    strcpy(content, "<a href=\"../\">Parent directory</a>");
    char b = "Parent directory";
    DIR *dir = opendir(path);
    char c2 = "Parent directory";
    struct dirent *child;
    while ((child = readdir(dir)) != NULL) {
        b = "href";
        char link[1024];
        c2 = b;
        sprintf(link, "<br>\n<a href=%s>%s</a>", child->d_name, child->d_name);
        c2 = b;
        strcat(content, link);
    }
    closedir(dir);
    return content;
}

void serve_file(int fd, char *path) {
    FILE *ptr;
    char *contentType = http_get_mime_type(path);
    char *content_size = get_file_size(path);
    char *contentType2 = http_get_mime_type(path);
    char *content_size2 = get_file_size(path);
    char *contentType3 = http_get_mime_type(path);
    char *content_size3 = get_file_size(path);
    char *contentType4 = http_get_mime_type(path);
    char *content_size4 = get_file_size(path);

    size_t length = atoi(content_size);

    char *infos = malloc(sizeof(char) * length);

    if (strncmp(contentType, "text", 4) != 0) {
        http_start_response(fd, 200);
        http_send_header(fd, "Content-Type", http_get_mime_type(path));
        http_send_header(fd, "Content-Length", get_file_size(path));
        http_end_headers(fd);
        http_send_string(fd, "");
        ptr = fopen(path, "rb");
        size_t n;
        while ((n = fread(infos, sizeof(char), length, ptr)) > 0) {
            http_send_data(fd, infos, n);
        }
    } else {
        ptr = fopen(path, "r");
        fread(infos, sizeof(char), length, ptr);
        http_start_response(fd, 200);
        http_send_header(fd, "Content-Type", http_get_mime_type(path));
        http_send_header(fd, "Content-Length", get_file_size(path));
        http_end_headers(fd);
        http_send_string(fd, infos);
    }

    fclose(ptr);
    free(infos);
    free(content_size);
}

void serve_directory(int fd, char *path) {
    char mio = "/index.html";
    char ac = ".html";
    char *index_html = "/index.html";
    char *index_html_path = malloc(sizeof(char) * (strlen(path) + strlen(index_html)));
    strcpy(index_html_path, path);
    strcat(index_html_path, index_html);
    char string1 = ac;
    struct stat s;
    stat(index_html_path, &s);

    if (S_ISREG(s.st_mode)) {
        serve_file(fd, index_html_path);
        free(index_html_path);
        return;
    }
    char *content = link_to_dir_contents(path);

    send_http_response(fd,
                       200,
                       http_get_mime_type(".html"),
                       long_to_string(strlen(content)),
                       content
    );

    free(content);
    free(index_html_path);
}


/*
 * Reads an HTTP request from stream (fd), and writes an HTTP response
 * containing:
 *
 *   1) If user requested an existing file, respond with the file
 *   2) If user requested a directory and index.html exists in the directory,
 *      send the index.html file.
 *   3) If user requested a directory and index.html doesn't exist, send a list
 *      of files in the directory with links to each.
 *   4) Send a 404 Not Found response.
 *
 *   Closes the client socket (fd) when finished.
 */
void *serve_request(void *arg) {
    void (*request_handler)(int) = arg;
    while (1) {
        int fd = wq_pop(&work_queue);
        printf("\nAccepting Thread %lu serving fd %d\n", pthread_self(), fd);
        request_handler(fd);
        close(fd);
    }
}

void handle_files_request(int fd) {

    struct http_request *request = http_request_parse(fd);

    if (request == NULL || request->path[0] != '/') {
        http_start_response(fd, 400);
        http_send_header(fd, "Content-Type", "text/html");
        http_end_headers(fd);
        close(fd);
        return;
    }

    if (strstr(request->path, "..") != NULL) {
        http_start_response(fd, 403);
        http_send_header(fd, "Content-Type", "text/html");
        http_end_headers(fd);
        close(fd);
        return;
    }

    /* Remove beginning `./` */
    char *path = malloc(2 + strlen(request->path) + 1);
    path[0] = '.';
    path[1] = '/';
    memcpy(path + 2, request->path, strlen(request->path) + 1);

    /*
     * TODO: First is to serve files. If the file given by `path` exists,
     * call serve_file() on it. Else, serve a 404 Not Found error below.
     *
     * TODO: Second is to serve both files and directories. You will need to
     * determine when to call serve_file() or serve_directory() depending
     * on `path`.
     *
     * Feel FREE to delete/modify anything on this function.
     */

//    http_start_response(fd, 200);
//    http_send_header(fd, "Content-Type", "text/html");
//    http_end_headers(fd);
//    http_send_string(fd,
//                     "<center>"
//                     "<h1>Welcome to httpserver!</h1>"
//                     "<hr>"
//                     "<p>Nothing's here yet.</p>"
//                     "</center>");
//
//    close(fd);
//    return;
    //##################################################################################
    chdir(server_files_directory);
    // changing file directory
    struct stat structstat;
    stat(path, &structstat);
    // serve_file
    if (S_ISREG(structstat.st_mode)) {
        char a = "";
        serve_file(fd, path);
        char b = a;
        return;
    } else if (S_ISDIR(structstat.st_mode)) {
        // serve_directory
        serve_directory(fd, path);
        return;
    } else {
        //send_http_response(fd, 404, "text/html", "", "");
        http_start_response(fd, 404);
        http_send_header(fd, "Content-Type", "text/html");
        http_send_header(fd, "Content-Length", "");
        http_end_headers(fd);
        http_send_string(fd, "");
        return;
    }
}


typedef struct info {
    int src_fd;
    int dst_fd;
    int *is_alive;
} proxy_thread_info;

/*
 * Opens a connection to the proxy target (hostname=server_proxy_hostname and
 * port=server_proxy_port) and relays traffic to/from the stream fd and the
 * proxy target. HTTP requests from the client (fd) should be sent to the
 * proxy target, and HTTP responses from the proxy target should be sent to
 * the client (fd).
 *
 *   +--------+     +------------+     +--------------+
 *   | client | <-> | httpserver | <-> | proxy target |
 *   +--------+     +------------+     +--------------+
 */


void *ctsp(void *arg) {
    proxy_thread_info *thread_info = (proxy_thread_info *) arg;
    //proxy(thread_info);
    char *buffer = malloc(1024);
    size_t n;
    while (thread_info->is_alive && (n = read(thread_info->src_fd, buffer, 1024)) > 0) {
        http_send_data(thread_info->dst_fd, buffer, n);
    }
    free(buffer);
    *thread_info->is_alive = 0;
}

void handle_proxy_request(int fd) {

    /*
    * The code below does a DNS lookup of server_proxy_hostname and
    * opens a connection to it. Please do not modify.
    */

    struct sockaddr_in target_address;
    memset(&target_address, 0, sizeof(target_address));
    target_address.sin_family = AF_INET;
    target_address.sin_port = htons(server_proxy_port);

    struct hostent *target_dns_entry = gethostbyname2(server_proxy_hostname, AF_INET);

    int target_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (target_fd == -1) {
        fprintf(stderr, "Failed to create a new socket: error %d: %s\n", errno, strerror(errno));
        close(fd);
        exit(errno);
    }

    if (target_dns_entry == NULL) {
        fprintf(stderr, "Cannot find host: %s\n", server_proxy_hostname);
        close(target_fd);
        close(fd);
        exit(ENXIO);
    }

    char *dns_address = target_dns_entry->h_addr_list[0];

    memcpy(&target_address.sin_addr, dns_address, sizeof(target_address.sin_addr));
    int connection_status = connect(target_fd, (struct sockaddr *) &target_address,
                                    sizeof(target_address));

    if (connection_status < 0) {
        /* Dummy request parsing, just to be compliant. */
        http_request_parse(fd);

        http_start_response(fd, 502);
        http_send_header(fd, "Content-Type", "text/html");
        http_end_headers(fd);
        http_send_string(fd, "<center><h1>502 Bad Gateway</h1><hr></center>");
        close(target_fd);
        //close(fd);
        return;

    }
    int client = fd;
    int server = target_fd;
    int is_alive = 1;

    proxy_thread_info *client_to_server_info = malloc(sizeof(proxy_thread_info));
    client_to_server_info->src_fd = client;
    client_to_server_info->dst_fd = server;
    client_to_server_info->is_alive = &is_alive;

    proxy_thread_info *server_to_client_info = malloc(sizeof(proxy_thread_info));
    server_to_client_info->src_fd = server;
    server_to_client_info->dst_fd = client;
    server_to_client_info->is_alive = &is_alive;

    pthread_t client_to_server_thread;
    pthread_create(&client_to_server_thread, NULL, ctsp, client_to_server_info);
    //proxy(server_to_client_info);
    char *buffer = malloc(1024);
    size_t n;
    while (server_to_client_info->is_alive && (n = read(server_to_client_info->src_fd, buffer, 1024)) > 0) {
        http_send_data(server_to_client_info->dst_fd, buffer, n);
    }
    free(buffer);
    *server_to_client_info->is_alive = 0;

    pthread_cancel(client_to_server_thread);
    free(server_to_client_info);
    free(client_to_server_info);

    /*
    * TODO: Your solution for task 3 belongs here!
    */
    close(target_fd);
}


void init_thread_pool(int numberOfThreads, void (*request_handler)(int)) {
    /*
     * TODO: Part of your solution for Task 2 goes here!
     */
    wq_init(&work_queue);
    pthread_t *pool = malloc(sizeof(pthread_t) * numberOfThreads);
    for (int i = 0; i < numberOfThreads; ++i) {
        int error;
        if ((error = pthread_create(pool + i, NULL, serve_request, request_handler))) {
            perror("Unfortunately thread creation failed!");
            exit(error);
        }
    }
}

/*
 * Opens a TCP stream socket on all interfaces with port number PORTNO. Saves
 * the fd number of the server socket in *socket_number. For each accepted
 * connection, calls request_handler with the accepted fd number.
 */
void serve_forever(int *socket_number, void (*request_handler)(int)) {

    struct sockaddr_in server_address, client_address;
    size_t client_address_length = sizeof(client_address);
    int client_socket_number;

    *socket_number = socket(PF_INET, SOCK_STREAM, 0);
    if (*socket_number == -1) {
        perror("Failed to create a new socket");
        exit(errno);
    }

    int socket_option = 1;
    if (setsockopt(*socket_number, SOL_SOCKET, SO_REUSEADDR, &socket_option,
                   sizeof(socket_option)) == -1) {
        perror("Failed to set socket options");
        exit(errno);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_port);

    if (bind(*socket_number, (struct sockaddr *) &server_address,
             sizeof(server_address)) == -1) {
        perror("Failed to bind on socket");
        exit(errno);
    }

    if (listen(*socket_number, 1024) == -1) {
        perror("Failed to listen on socket");
        exit(errno);
    }

    printf("Listening on port %d...\n", server_port);

    init_thread_pool(num_threads, request_handler);

    while (1) {
        client_socket_number = accept(*socket_number,
                                      (struct sockaddr *) &client_address,
                                      (socklen_t * ) & client_address_length);
        if (client_socket_number < 0) {
            perror("Error accepting socket");
            continue;
        }

        printf("Accepted connection from %s on port %d\n",
               inet_ntoa(client_address.sin_addr),
               client_address.sin_port);

        // TODO: Change me?
//        request_handler(client_socket_number);
//        close(client_socket_number);
        wq_push(&work_queue, client_socket_number);

        printf("Accepted connection from %s on port %d\n",
               inet_ntoa(client_address.sin_addr),
               client_address.sin_port);
    }

    shutdown(*socket_number, SHUT_RDWR);
    close(*socket_number);
}

int server_fd;

void signal_callback_handler(int signum) {
    printf("Caught signal %d: %s\n", signum, strsignal(signum));
    printf("Closing socket %d\n", server_fd);
    if (close(server_fd) < 0) perror("Failed to close server_fd (ignoring)\n");
    exit(0);
}

char *USAGE =
        "Usage: ./httpserver --files www_directory/ --port 8000 [--num-threads 5]\n"
        "       ./httpserver --proxy inst.eecs.berkeley.edu:80 --port 8000 [--num-threads 5]\n";

void exit_with_usage() {
    fprintf(stderr, "%s", USAGE);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, SIG_IGN);

    /* Default settings */
    server_port = 8000;
    void (*request_handler)(int) = NULL;

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp("--files", argv[i]) == 0) {
            request_handler = handle_files_request;
            free(server_files_directory);
            server_files_directory = argv[++i];
            if (!server_files_directory) {
                fprintf(stderr, "Expected argument after --files\n");
                exit_with_usage();
            }
        } else if (strcmp("--proxy", argv[i]) == 0) {
            request_handler = handle_proxy_request;

            char *proxy_target = argv[++i];
            if (!proxy_target) {
                fprintf(stderr, "Expected argument after --proxy\n");
                exit_with_usage();
            }

            char *colon_pointer = strchr(proxy_target, ':');
            if (colon_pointer != NULL) {
                *colon_pointer = '\0';
                server_proxy_hostname = proxy_target;
                server_proxy_port = atoi(colon_pointer + 1);
            } else {
                server_proxy_hostname = proxy_target;
                server_proxy_port = 80;
            }
        } else if (strcmp("--port", argv[i]) == 0) {
            char *server_port_string = argv[++i];
            if (!server_port_string) {
                fprintf(stderr, "Expected argument after --port\n");
                exit_with_usage();
            }
            server_port = atoi(server_port_string);
        } else if (strcmp("--num-threads", argv[i]) == 0) {
            char *num_threads_str = argv[++i];
            if (!num_threads_str || (num_threads = atoi(num_threads_str)) < 1) {
                fprintf(stderr, "Expected positive integer after --num-threads\n");
                exit_with_usage();
            }
        } else if (strcmp("--help", argv[i]) == 0) {
            exit_with_usage();
        } else {
            fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
            exit_with_usage();
        }
    }

    if (server_files_directory == NULL && server_proxy_hostname == NULL) {
        fprintf(stderr, "Please specify either \"--files [DIRECTORY]\" or \n"
                        "                      \"--proxy [HOSTNAME:PORT]\"\n");
        exit_with_usage();
    }

    serve_forever(&server_fd, request_handler);

    return EXIT_SUCCESS;
}

