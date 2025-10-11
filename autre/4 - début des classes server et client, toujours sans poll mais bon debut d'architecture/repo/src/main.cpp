#include "main.hpp"
#include "Server.hpp"
#include "Client.hpp"

int main()
{
	try
	{
		Server server;

		set_signal_handler();

		int client_fd;
		while (g_stop_requested == 0)
		{
			client_fd = accept(server.getServerFd(), NULL, NULL);
			if (client_fd == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK) {} // aucun nouveau client dans le backlog
				else
					perror_and_throw("accept");
			}
			else
			{
				fcntl(client_fd, F_SETFL, O_NONBLOCK);
				server.addClient(client_fd);
			}
			server.checkClients();
			sleep(1);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "exit success" << std::endl;
	return EXIT_SUCCESS;
}
