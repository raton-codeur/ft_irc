#include "main.hpp"

int main()
{
	try
	{
		Data data;
		set_handler();

		int client_fd;
		std::string s = "message du serveur : test\n";
		std::vector<int>::iterator it;
		int n;
		char buffer[1024];
		while (g_stop_requested == 0)
		{
			client_fd = accept(data.server_fd, NULL, NULL);
			if (client_fd == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK) {} // aucun nouveau client dans le backlog
				else
					perror_and_throw("accept");
			}
			else
			{
				std::cout << "nouveau client" << std::endl;
				fcntl(client_fd, F_SETFL, O_NONBLOCK);
				data.client_fds.push_back(client_fd);
			}
			for (it = data.client_fds.begin(); it != data.client_fds.end(); )
			{
				n = recv(*it, buffer, sizeof(buffer), 0);
				if (n == 0)
				{
					std::cout << "client déconnecté" << std::endl;
					close(*it);
					it = data.client_fds.erase(it);
					continue;
				}
				else if (n == -1)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK) {} // pas de données envoyées mais la connexion est toujours là
					else
						perror_and_throw("recv");
				}
				else
				{
					std::cout << "message du client : ";
					std::cout.write(buffer, n);
				}
				if (send(*it, s.c_str(), s.size(), 0) == -1)
					perror_and_throw("send");
				++it;
			}
			sleep(1);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "\nexit success\n";
	return EXIT_SUCCESS;
}
