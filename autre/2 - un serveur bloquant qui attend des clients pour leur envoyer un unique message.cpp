#include <iostream>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
using namespace std;

int main()
{
	string		input;
	const int	backlog = 5;
	int			server_fd, client_fd;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		throw runtime_error("socket failed");
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    struct sockaddr_in addr_server;
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = INADDR_ANY;
    addr_server.sin_port = htons(6667);
    if (bind(server_fd, (struct sockaddr*)&addr_server, sizeof(addr_server)) < 0)
		throw runtime_error("bind failed");
    listen(server_fd, backlog);
    cout << "server listening on port 6667" << endl;

	while (true)
	{
		client_fd = accept(server_fd, NULL, NULL); // échoue si aucun client
		if (client_fd != -1)
		{
			string s = "message du serveur : test\n";
			if (send(client_fd, s.c_str(), s.size(), 0) < 0)
				throw runtime_error("send failed");
			close(client_fd);
		}
		else if (errno != EAGAIN && errno != EWOULDBLOCK)
        	throw runtime_error("accept failed");
	}

	return 0;
}
