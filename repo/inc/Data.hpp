#pragma once
#include <main.hpp>

class Data
{
	private :
		Data(const Data& other);
		Data& operator=(const Data& other);

	public :
		static const int backlog = 5;

		int					server_fd;
		std::vector<int>	client_fds;

		Data();
		~Data();
};
