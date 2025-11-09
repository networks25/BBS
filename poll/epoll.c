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

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    listen(listen_fd, 10);

    epfd = epoll_create1(0);
    if (epfd < 0) { perror("epoll_create1"); exit(1); }

    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    printf("epoll() echo server listening on port %d...\n", PORT);

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n < 0) { perror("epoll_wait"); break; }

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == listen_fd) {
                // New connection
                conn_fd = accept(listen_fd, (struct sockaddr*)&addr, &addrlen);
                if (conn_fd < 0) { perror("accept"); continue; }

                printf("New client connected: fd=%d\n", conn_fd);
                ev.events = EPOLLIN;
                ev.data.fd = conn_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);
            } else {
                // Data from client
                ssize_t len = read(events[i].data.fd, buffer, sizeof(buffer));
                if (len <= 0) {
                    printf("Client disconnected: fd=%d\n", events[i].data.fd);
                    close(events[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                } else {
                    write(events[i].data.fd, buffer, len);
                }
            }
        }
    }

    close(listen_fd);
    close(epfd);
    return 0;
}

