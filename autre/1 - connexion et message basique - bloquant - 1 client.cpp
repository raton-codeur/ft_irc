
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;

int main()
{
	int server_fd, client_fd;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		throw runtime_error("socket failed");

	struct sockaddr_in address_server;
	address_server.sin_family = AF_INET;
    address_server.sin_addr.s_addr = INADDR_ANY;
    address_server.sin_port = htons(6667);

	if (bind(server_fd, (struct sockaddr*)&address_server, sizeof(address_server)) < 0)
		throw runtime_error("bind failed");

	if (listen(server_fd, 5) < 0)
		throw runtime_error("listen failed");

	cout << "serveur en attente sur le port 6667" << endl;

	client_fd = accept(server_fd, NULL, NULL);
	if (client_fd < 0)
		throw runtime_error("accept failed");

	cout << "client connecté" << endl;

	string s = "message du serveur\n";
	if (send(client_fd, s.c_str(), s.size(), 0) < 0)
		throw runtime_error("send failed");

	close(client_fd);
    close(server_fd);

	cout << "message transmis" << endl;

	return 0;
}
