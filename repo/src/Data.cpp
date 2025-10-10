#include "Data.hpp"

Data::Data()
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		perror_and_throw("socket");

	fcntl(server_fd, F_SETFL, O_NONBLOCK);

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		perror_and_throw("setsockopt(SO_REUSEADDR)");

	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = INADDR_ANY;
	addr_server.sin_port = htons(6667);

	if (bind(server_fd, (struct sockaddr*)&addr_server, sizeof(addr_server)) == -1)
		perror_and_throw("bind");

	listen(server_fd, backlog);
}

Data::~Data()
{
	if (server_fd != -1)
		close(server_fd);
	for (size_t i = 0; i < client_fds.size(); ++i)
		close(client_fds[i]);
	std::cout << "\nall data has been freed\n";
}
