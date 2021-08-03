#include "ServerGroup.hpp"
#include "./../utils/utils.hpp"
#include <time.h>

#define READY_FOR_REQUEST 1
#define CHUNKED_RECEIVE 2
#define BODYBUFF_RECEIVE 3
#define READY_TO_RESPONSE 4
#define CURRENTLY_RESPONDING 5
#define CHUNKED_SEND 6

ServerGroup::ServerGroup(Config &config)
{
	serversCount = config.getServers().size();
	for (int i = 0; i < serversCount; i++)
	{
		servers.push_back(ServerUnit(config.getServers().at(i)));
	}
	std::cout << "all server has been initialized\n";
	contentType = ft::initilizeContentTypes();
}

ServerGroup::~ServerGroup()
{
}

void ServerGroup::startServers()
{
	fd_set readSet;
	fd_set writeSet;
	int maxFd = 0;
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	std::list<ServerUnit>::iterator servIt = servers.begin();
	std::list<ServerUnit>::iterator servEnd = servers.end();
	while (servIt != servEnd)
	{
		servIt->startListening();
		int serverSocket = servIt->getSocket();
		if (serverSocket > maxFd)
		{
			maxFd = serverSocket;
		}
		FD_SET(serverSocket, &readSet);
		FD_SET(serverSocket, &writeSet);
		++servIt;
	}
	struct linger sl;
	sl.l_onoff = 1;
	sl.l_linger = 0;
	while (true)
	{
		fd_set copyReadSet;
		fd_set copyWriteSet;
		FD_ZERO(&copyReadSet);
		FD_ZERO(&copyWriteSet);
		FD_COPY(&readSet, &copyReadSet);
		FD_COPY(&writeSet, &copyWriteSet);
		int activity = select(maxFd + 1, &copyReadSet, &copyWriteSet, NULL, NULL);
		if (activity <= 0)
		{
			std::cerr << "select: << " << activity << "\n";
			continue;
		}
		servIt = servers.begin();
		while (servIt != servEnd)
		{
			if (FD_ISSET(servIt->getSocket(), &copyReadSet))
			{
				int clientSocket = servIt->acceptNewClient();
				FD_SET(clientSocket, &readSet);
				FD_SET(clientSocket, &writeSet);
				if (clientSocket > maxFd)
					maxFd = clientSocket;
				std::cout << "new connection: " << clientSocket << std::endl;
				std::cout << "total count: " << servIt->getClients().size() << std::endl;
			}
			std::vector<Client>::iterator clientIt = servIt->getClients().begin();
			while (clientIt != servIt->getClients().end())
			{
				if (FD_ISSET(clientIt->getSocket(), &copyReadSet) &&
					(clientIt->getStatus() == READY_FOR_REQUEST ||
					 clientIt->getStatus() == CHUNKED_RECEIVE))
				{
					if (clientIt->recieveRequest(contentType) <= 0)
					{
						FD_CLR(clientIt->getSocket(), &readSet);
						FD_CLR(clientIt->getSocket(), &writeSet);
						setsockopt(clientIt->getSocket(), SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));
						shutdown(clientIt->getSocket(), SHUT_RDWR);
						close(clientIt->getSocket());
						servIt->deleteClient(clientIt);
						clientIt = servIt->getClients().begin();
						continue;
					}
				}
				if (FD_ISSET(clientIt->getSocket(), &copyWriteSet) &&
					(clientIt->getStatus() == READY_TO_RESPONSE ||
					 clientIt->getStatus() == CHUNKED_SEND))
				{
					if (clientIt->sendResponse() <= 0)
					{
						FD_CLR(clientIt->getSocket(), &readSet);
						FD_CLR(clientIt->getSocket(), &writeSet);
						setsockopt(clientIt->getSocket(), SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));
						shutdown(clientIt->getSocket(), SHUT_RDWR);
						close(clientIt->getSocket());
						servIt->deleteClient(clientIt);
						clientIt = servIt->getClients().begin();
						continue;
					}
				}
				clientIt++;
			}
			servIt++;
		}
	}
}

std::list<ServerUnit> &ServerGroup::getServers()
{
	return servers;
}
