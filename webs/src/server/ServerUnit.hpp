#ifndef SERVERUNIT_HPP
# define SERVERUNIT_HPP

#include <string>
#include "./Client.hpp"
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include "./../config_parser/Server.hpp"

class ServerUnit
{
private:
	int					serverSocket;
	int					port;
	std::string			host;
	std::vector<Client>	clients;
	Server				config;

	void	initializeSocket();

public:
	ServerUnit(Server &serverConfig);
	~ServerUnit();
	void	startListening();
	int		acceptNewClient();
	void	deleteClient(std::vector<Client>::iterator clientIt);
	std::vector<Client>	&getClients();
	int		getSocket();
};

#endif
