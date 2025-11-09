// poll_echo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 12345
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 1024

int main(void) {
    int listen_fd, new_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buffer[BUFFER_SIZE];
    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    // Create server socket
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

    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    printf("poll() echo server listening on port %d...\n", PORT);

    while (1) {
        int ready = poll(fds, nfds, -1);
        if (ready < 0) { perror("poll"); break; }

        // Check all file descriptors
        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == listen_fd) {
                    // Accept new connection
                    new_fd = accept(listen_fd, (struct sockaddr*)&addr, &addrlen);
                    if (new_fd < 0) { perror("accept"); continue; }

                    printf("New client connected: fd=%d\n", new_fd);
                    fds[nfds].fd = new_fd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } else {
                    // Read from client
                    ssize_t n = read(fds[i].fd, buffer, sizeof(buffer));
                    if (n <= 0) {
                        printf("Client disconnected: fd=%d\n", fds[i].fd);
                        close(fds[i].fd);
                        fds[i] = fds[nfds-1];
                        nfds--;
                        i--;
                    } else {
                        write(fds[i].fd, buffer, n); // echo back
                    }
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}

