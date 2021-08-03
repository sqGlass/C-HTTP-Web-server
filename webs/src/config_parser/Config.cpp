#include "Config.hpp"
#include "Location.hpp"
#include "Server.hpp"

static void error_exit(std::string message)
{
	std::cout << message << std::endl;
	exit(0);
}

Config::Config(void) { return; }

Config::Config(Config const &obj) { *this = obj; }

Config &Config::operator=(Config const &) { return *this; }

Config::~Config(void) { return; }

Config::Config(std::string path_to_config)
{
	_strings = readFile(path_to_config);
	checkValid();
	if (countServers(_strings) == 0)
		error_exit("Servers not found!");
	for (size_t i = 0; i < _strings.size(); i++)
	{
		if (_strings[i].compare("server") == 0)
			servers.push_back(Server(_strings, i));
	}
	excludeCopy();
	// for (size_t i = 0; i < servers.size(); i++)
	// {
	// 	servers[i].print();
	// }
}

void	Config::excludeCopy(void)
{
	int i = 0;
	int n;
	int len = servers.size();
	while (i < len)
	{
		n = i + 1;
		while (n < len)
		{
			if (servers[i].getHostPort() == servers[n].getHostPort())
			{
				servers.erase(servers.begin() + n);
				len = servers.size();
				continue;
			}
			n++;
		}
		i++;
	}
}

int	Config::countServers(strVector &strings)
{
	int	count = 0;
	for (size_t i = 0; i < strings.size(); i++)
	{
		if (strings[i].compare("server") == 0)
			count++;
	}
	return (count);
}

strVector	Config::split(std::string str, std::string charset)
{
	strVector	tokens;
	str += charset[0];
	std::string::size_type	start = str.find_first_not_of(charset, 0);
	std::string::size_type	end = 0;
	while (true) {
		end = str.find_first_of(charset, start);
		if (end == std::string::npos) {
			break;
		}
		std::string	s = str.substr(start, end - start);
		tokens.push_back(s);
		if ((start = str.find_first_not_of(charset, end)) == std::string::npos)
			break ;
	}
	return tokens;
}

strVector	Config::readFile(std::string path)
{
	std::string		line = "";
	strVector		file;
	std::ifstream is (path, std::ifstream::binary);

  	if (is)
	{
		is.seekg(0, is.end);
		int length = is.tellg();
		is.seekg(0, is.beg);
		char *buffer = new char[length];
		is.read(buffer, length);
		line += buffer;
		is.close();
		delete[] buffer;
	}
	else
		error_exit("Cannot open config file!");
	file = split(line, std::string(" \n\t"));
 	return file;
}

void	Config::checkValid(void)
{
	for (size_t i = 0; i < _strings.size(); i++)
	{
		if (_strings[i].compare(0, 6, "server") == 0 && i + 1 < _strings.size() && _strings[i + 1].compare("{") != 0)
		{
			if (_strings[i].compare("server_name") != 0)
				error_exit("Error syntax!");
		}
		if (_strings[i].compare("server") == 0)
		{
			i++;
			while (_strings[i].compare("}") != 0 && i < _strings.size())
			{
				if (_strings[i].compare(0, 8, "location") == 0 && i + 2 < _strings.size() && _strings[i + 2].compare("{") != 0)
					error_exit("Error syntax!");
				if (_strings[i].compare("location") == 0)
				{
					i++;
					while (_strings[i].compare("}") != 0 && i < _strings.size())
					{
						if (_strings[i].compare("server") == 0 || _strings[i].compare("location") == 0)
							error_exit("Error syntax!");
						i++;
					}
					if (i == _strings.size())
						error_exit("Error syntax!");
				}
				if (_strings[i].compare("server") == 0)
					error_exit("Error syntax!");
				i++;
			}
			if (_strings[i].compare("}") != 0)
				error_exit("Error syntax!");
		}
	}
}

std::vector<Server> &Config::getServers()
{
	return this->servers;
}

// int main()
// {
// 	Config test("../../configs/webserver.conf");
// 	return (0);
// }
