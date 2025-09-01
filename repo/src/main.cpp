#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
using namespace std;

int main()
{
    int fd_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_listen < 0)
		throw runtime_error("socket failed");

	fcntl(fd_listen, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr_server;
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(6667);
    addr_server.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd_listen, (struct sockaddr*)&addr_server, sizeof(addr_server)) < 0)
		throw runtime_error("bind failed");
    if (listen(fd_listen, 5) < 0)
		throw runtime_error("listen failed");

	struct pollfd fds[2];
	fds[0].fd = fd_listen;
	fds[0].events = POLLIN;
	fds[1].fd = -1;

    printf("server running on port 6667...\n");

    while (true)
	{
		int n = poll(fds, 2, -1); // attend qu'un fd soit prêt
		if (n < 0)
			{perror("poll"); break;}

		if (fds[0].revents & POLLIN)
		{
			int fd_client = accept(fd_listen, NULL, NULL);
			if (fd_client >= 0)
			{
				fds[1].fd = fd_client;
				fds[1].events = POLLIN;
				printf("client accepté\n");
			}
		}

		if (fds[1].fd >= 0 && (fds[1].revents & POLLIN))
		{
            char buf[1024];
            int n = recv(fds[1].fd, buf, sizeof(buf), 0);
            if (n > 0)
			{
                printf("reçu: %.*s\n", n, buf);
				send(fds[1].fd, buf, n, 0);
            }
			else
			{
				close(fds[1].fd);
				fds[1].fd = -1;
				printf("client déconnecté\n");
			}
        }
    }

    close(fd_listen);
    return 0;
}
