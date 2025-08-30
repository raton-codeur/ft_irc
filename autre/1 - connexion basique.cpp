
#include <iostream>
#include <stdexcept>
#include <cstring> // memset
#include <unistd.h> // close
#include <arpa/inet.h> // tout ce qui est socket
using namespace std;

int main()
{
	int server_fd, client_fd;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		throw runtime_error("socket failed");

	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(6667);

	if (bind(server_fd, (struct sockaddr*)&address, addrlen) < 0)
		throw runtime_error("bind failed");

	if (listen(server_fd, 5) < 0)
		throw runtime_error("listen failed");

	cout << "Serveur en attente sur le port 6667..." << endl;

	client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_fd < 0)
		throw runtime_error("accept failed");

	cout << "client connecté" << endl;

	const char* msg = "Hello depuis le serveur IRC minimal !\n";
	send(client_fd, msg, strlen(msg), 0);

	close(client_fd);
    close(server_fd);

	return 0;
}
