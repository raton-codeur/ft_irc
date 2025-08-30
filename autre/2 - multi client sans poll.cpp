#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
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
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0)
		throw runtime_error("socket failed");

    int flags = fcntl(lfd, F_GETFL, 0);
    fcntl(lfd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6667);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(lfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw runtime_error("bind failed");
    listen(lfd, 5);

    printf("server running on port 6667...\n");

    while (true)
	{
        int cfd = accept(lfd, NULL, NULL); // échoue tant qu'aucun client n'arrive
        if (cfd >= 0)
		{
            printf("client accepté\n");
            char buf[1024];
            int n = recv(cfd, buf, sizeof(buf), 0); // échoue tant qu'aucune donnée n'est reçue
            if (n > 0)
			{
                printf("reçu: %.*s\n", n, buf);
                send(cfd, buf, n, 0); // échoue si le buffer est plein
            }
            close(cfd);
        }
		else
		{
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                perror("accept");
        }
    }
    close(lfd);
    return 0;
}
