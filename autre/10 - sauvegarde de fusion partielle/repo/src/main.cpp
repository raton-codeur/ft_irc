#include "main.hpp"
#include "Server.hpp"
#include "Client.hpp"

int main(int argc, char** argv)
{
	try
	{
		Server server(argc, argv);
		set_signal_handlers();
		while (g_stop_requested == 0)
			server.run();
	}
	catch (StopRequested&) {}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "exit success" << std::endl;
	return EXIT_SUCCESS;
}
