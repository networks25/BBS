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
    struct pollfd fds[MAX_CLIENTS];  // List of sockets to monitor
    int nfds = 1;                    // How many fds are active

    // === Create listening socket ===
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); exit(1); }

    // Allow reusing the same port immediately after restart
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket to port
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }

    // Start listening for incoming connections
    listen(listen_fd, 10);

    // Add the listening socket to the poll list
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;  // Interested in "readable" events (new connections)

    printf("poll() echo server listening on port %d...\n", PORT);

    while (1) {
        // Wait for activity on any of the sockets (-1 means no timeout)
        int ready = poll(fds, nfds, -1);
        if (ready < 0) { perror("poll"); break; }

        // Iterate through all file descriptors
        for (int i = 0; i < nfds; i++) {
            // Check if this fd is ready to read
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == listen_fd) {
                    // === New incoming connection ===
                    new_fd = accept(listen_fd, (struct sockaddr*)&addr, &addrlen);
                    if (new_fd < 0) { perror("accept"); continue; }

                    printf("New client connected: fd=%d\n", new_fd);

                    // Add new client to list
                    fds[nfds].fd = new_fd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } else {
                    // === Existing client sent data ===
                    ssize_t n = read(fds[i].fd, buffer, sizeof(buffer));
                    if (n <= 0) {
                        // n==0 means client closed connection
                        printf("Client disconnected: fd=%d\n", fds[i].fd);
                        close(fds[i].fd);

                        // Remove this fd by moving the last one in its place
                        fds[i] = fds[nfds-1];
                        nfds--;
                        i--;  // recheck this position
                    } else {
                        // Echo back to client
                        write(fds[i].fd, buffer, n);
                    }
                }
            }
        }
    }

    close(listen_fd);
    return 0;
}

