#include "ServerUnit.hpp"

ServerUnit::ServerUnit(Server &serverConfig) : port(serverConfig.getPort()),
host(serverConfig.getHost()), config(serverConfig)
{
	initializeSocket();
    std::cout << "host: |" << host << "|\n";
    std::cout << "port: |" << port << "|\n";
}

ServerUnit::~ServerUnit()
{}

void	ServerUnit::initializeSocket()
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		std::cerr << "socket creation failed!\n";
	}
	this->serverSocket = serverSocket;
}

int	ServerUnit::acceptNewClient()
{
	struct sockaddr_in clientAddr;
	socklen_t sizeOfClientAddr = sizeof(clientAddr);
	int clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &sizeOfClientAddr);
	if (clientSocket == -1)
	{
		std::cerr << "client socket failed!\n";
		exit(1);
	}
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl error\n";
		exit(1);
	}
	clients.push_back(Client(clientSocket, clientAddr, config));
	return clientSocket;
}

void	ServerUnit::startListening()
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host.c_str());
	// addr.sin_addr.s_addr = INADDR_ANY;
	std::cout << "s.addr: " << addr.sin_addr.s_addr << "\n";
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &addr, sizeof(addr)) < 0)
	{
		std::cerr << "FAIL WHEN SETSOCKOPT\n";
		exit(1);
	}
	if (bind(serverSocket, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		std::cerr << "binding failed!\n";
		exit(1);
	}
	if (listen(serverSocket, SOMAXCONN) < 0)
	{
		std::cerr << "LISTEN RETURNED -1\n";
		exit(1);
	}
	std::cout << "Server is listening: " << serverSocket << " socket!\n";
}

std::vector<Client> &ServerUnit::getClients()
{
	return clients;
}

int	ServerUnit::getSocket()
{
	return serverSocket;
}

void	ServerUnit::deleteClient(std::vector<Client>::iterator clientIt)
{
	clients.erase(clientIt);
}