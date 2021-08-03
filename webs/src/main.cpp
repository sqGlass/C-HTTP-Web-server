#include "server/ServerGroup.hpp"
#include "config_parser/Config.hpp"

int main()
{
	signal(SIGPIPE, SIG_IGN);
	Config conf("./configs/webserver.conf");
	ServerGroup	serverGroup(conf);
	serverGroup.startServers();
	
	return 0;
}
