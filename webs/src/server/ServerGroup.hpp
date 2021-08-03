#ifndef SERVERSGROUP_HPP
# define SERVERSGROUP_HPP

#include "./../config_parser/Config.hpp"
#include "./ServerUnit.hpp"
#include <list>

class ServerGroup
{
private:
	int	serversCount;
	std::list<ServerUnit> servers;
	std::map<std::string, std::string> contentType;
public:
	ServerGroup(Config &config);
	~ServerGroup();
	void	startServers();
	std::list<ServerUnit> &getServers();
};



#endif
