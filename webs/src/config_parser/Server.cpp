#include "Server.hpp"

static void error_exit(std::string message)
{
	std::cout << message << std::endl;
	exit(0);
}

Server::Server(void) { return; }

Server::Server(strVector &strings, size_t start)
{
	_strings = strings;
	_start = start;
	port = -1;
	client_max_body_size = -1;
	setValue();
	return;
}

Server::~Server(void) { return; }

void	Server::setValue(void)
{
	while (_strings[_start].compare("}") != 0 && _start < _strings.size())
	{
		if (_strings[_start].compare("location") == 0)
			setLocations();
		if (_strings[_start].compare("server_name") == 0)
			setServerName();
		if (_strings[_start].compare("host") == 0)
			setHost();
		if (_strings[_start].compare("listen") == 0)
			setPort();
		if (_strings[_start].compare("client_max_body_size") == 0)
			setClientMaxBodySize();
		if (_strings[_start].compare("error_page") == 0)
			setErrors();
		if (_strings[_start].compare("root") == 0)
			setRoot();
		_start++;
	}
	if (serverName.empty())
		serverName = "default";
	if (host.empty() || port == -1 || client_max_body_size == -1 || errors.size() == 0 || root.empty())
		error_exit("One or more fields are not specified: \nhost\nport\nclient_max_body_size\nerror_page\nroot");
	std::stringstream ss;
	ss << port;
	host_port = host + ":" + ss.str();
	sortLocations();
}

void	Server::setLocations(void)
{
	locations.push_back(Location(_strings, _start));
	while (_strings[_start].compare("}") != 0 && _start < _strings.size())
		_start++;
}

void	Server::setRoot(void)
{
	_start++;
	if (_strings[_start][0] != '/')
		error_exit("Root set incorrectly!");
	root = _strings[_start];
}

void	Server::setServerName(void)
{
	_start++;
	if (_strings[_start].compare("location") != 0 && _strings[_start].compare("host") != 0
	&& _strings[_start].compare("listen") != 0 && _strings[_start].compare("client_max_body_size") != 0
	&& _strings[_start].compare("error_page") != 0)
		serverName = _strings[_start];
}

void	Server::setClientMaxBodySize(void)
{
	_start++;
	for (size_t i = 0; i < _strings[_start].length(); i++)
	{
		if (_strings[_start][i] < '0' || _strings[_start][i] > '9')
			error_exit("Client_max_body_size set incorrectly!");
	}
	std::istringstream(_strings[_start]) >> client_max_body_size;
}

void	Server::setPort(void)
{
	_start++;
	for (size_t i = 0; i < _strings[_start].length(); i++)
	{
		if (_strings[_start][i] < '0' || _strings[_start][i] > '9' || _strings[_start].length() > 5)
			error_exit("Port set incorrectly!");
	}
	std::istringstream(_strings[_start]) >> port;
	if (port > 65536 || port < 1025)
		error_exit("Port set incorrectly!");
}

void	Server::setErrors(void)
{
	_start++;
	for (size_t i = 0; i < _strings[_start].length(); i++)
	{
		if (_strings[_start][i] < '0' || _strings[_start][i] > '9' || _strings[_start].length() != 3)
			error_exit("Error_page set incorrectly!");
	}
	_start++;
	errors[_strings[_start - 1]] = _strings[_start];
}

void	Server::setHost(void)
{
	_start++;
	int countPoint = 0;
	for (size_t i = 0; i < _strings[_start].length(); i++)
	{
		if ((_strings[_start][i] < '0' || _strings[_start][i] > '9') && _strings[_start][i] != '.')
			error_exit("Host set incorrectly!");
		if (_strings[_start][i] == '.')
			countPoint++;
	}
	if (countPoint != 3)
		error_exit("Host set incorrectly!");
	host = _strings[_start];
}

std::string		&Server::getServerName(void)
{
	return this->serverName;
}

std::string		&Server::getHost(void)
{
	return this->host;
}

int				Server::getPort(void)
{
	return this->port;
}

int				Server::getClientMaxBodySize(void)
{
	return this->client_max_body_size;
}

std::string		&Server::getHostPort(void)
{
	return this->host_port;
}

std::string	&Server::getRoot(void)
{
	return this->root;
}

std::vector<Location>		&Server::getLocations(void)
{
	return this->locations;
}

std::map<std::string, std::string>		&Server::getErrors(void)
{
	return this->errors;
}

void	Server::sortLocations(void)
{
	size_t i = 0;
	Location tmp;
	while (i + 1 < locations.size())
	{
		if (locations.at(i + 1).getUri().length() > locations.at(i).getUri().length())
		{
			tmp = locations.at(i + 1);
			locations.at(i + 1) = locations.at(i);
			locations.at(i) = tmp;
			i = 0;
			continue;
		}
		i++;
	}
}

void Server::print(void)
{
	std::cout << "serverName: " + serverName + " host:port: " + host_port + " size: " << client_max_body_size << std::endl;
	std::cout << root << std::endl;
	std::map<std::string, std::string>::iterator it = errors.begin();
	for (int i = 0; it != errors.end(); it++, i++)
	{
		std::cout << i << ") Ключ " << it->first << ", значение " << it->second << std::endl;
	}
}
