// epoll_echo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_EVENTS 1024
#define BUFFER_SIZE 1024

int main(void) {
    int listen_fd, conn_fd, epfd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buffer[BUFFER_SIZE];
    struct epoll_event ev, events[MAX_EVENTS];

    // === Create listening socket ===
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to port
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }

    // Start listening
    listen(listen_fd, 10);

    // === Create epoll instance ===
    epfd = epoll_create1(0);  // Argument must be 0 for modern Linux
    if (epfd < 0) { perror("epoll_create1"); exit(1); }

    // Register the listening socket with epoll
    ev.events = EPOLLIN;      // Notify us when it's readable (new connections)
    ev.data.fd = listen_fd;   // Store fd in event data
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    printf("epoll() echo server listening on port %d...\n", PORT);

    // === Main event loop ===
    while (1) {
        // Wait for any ready events (-1 = no timeout)
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n < 0) { perror("epoll_wait"); break; }

        // Process each event returned
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == listen_fd) {
                // === Accept new client ===
                conn_fd = accept(listen_fd, (struct sockaddr*)&addr, &addrlen);
                if (conn_fd < 0) { perror("accept"); continue; }

                printf("New client connected: fd=%d\n", conn_fd);

                // Register this new connection with epoll
                ev.events = EPOLLIN;    // Notify when readable
                ev.data.fd = conn_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);
            } else {
                // === Data available from a client ===
                ssize_t len = read(events[i].data.fd, buffer, sizeof(buffer));
                if (len <= 0) {
                    // Client closed connection or error
                    printf("Client disconnected: fd=%d\n", events[i].data.fd);
                    close(events[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                } else {
                    // Echo the data back to the same socket
                    write(events[i].data.fd, buffer, len);
                }
            }
        }
    }

    close(listen_fd);
    close(epfd);
    return 0;
}

