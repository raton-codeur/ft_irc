#include "main.hpp"
#include "Server.hpp"
#include "Client.hpp"

int main()
{
	try
	{
		Server server;
		set_signal_handlers();
		while (g_stop_requested == 0)
			server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "exit success" << std::endl;
	return EXIT_SUCCESS;
}
