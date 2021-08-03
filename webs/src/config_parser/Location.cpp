#include "Location.hpp"

static void error_exit(std::string message)
{
	std::cout << message << std::endl;
	exit(0);
}

Location::Location(void) { return; }

Location::~Location(void)
{
	return;
}

Location::Location(strVector strings, size_t start)
{
	_strings = strings;
	_start = start;
	max_body_size = -1;
	autoindex = false;
	redirect = nullptr;
	setValue();
	return;
}

void	Location::setValue(void)
{
	_start++;
	if (_strings[_start].compare("{") == 0)
		error_exit("Location set incorrectly!");
	else
	{
		if (_strings[_start][0] != '/')
			error_exit("Location set incorrectly!");
		if (_strings[_start][_strings[_start].length() - 1] == '/' && _strings[_start].length() > 1)
			_strings[_start].erase(_strings[_start].length() - 1);
		uri = _strings[_start];
	}
	_start++;
	while (_strings[_start].compare("}") != 0 && _start < _strings.size())
	{
		if (_strings[_start].compare("allow_methods") == 0)
			setMethods();
		if (_strings[_start].compare("autoindex") == 0)
			setAutoindex();
		if (_strings[_start].compare("redirect") == 0)
			setRedirect();
		if (_strings[_start].compare("try_files") == 0)
			setTryFiles();
		if (_strings[_start].compare("root") == 0)
			setLocRoot();
		if (_strings[_start].compare("cgi_path") == 0)
			setCGIPath();
		if (_strings[_start].compare("max_body_size") == 0)
			setMaxBodySize();
		if (_strings[_start].compare("}") == 0)
			break;
		_start++;
	}
	if (methods.size() == 0)
		error_exit("Methods not set!");
}

void	Location::setLocRoot(void)
{
	_start++;
	if (_strings[_start][0] != '/')
		error_exit("Location root set incorrectly!");
	locRoot = _strings[_start];
}

void		Location::setMethods(void)
{
	_start++;
	if (_strings[_start].compare("GET") != 0 && _strings[_start].compare("POST") != 0 && _strings[_start].compare("DELETE") != 0 && _strings[_start].compare("PUT") != 0)
		error_exit("Methods not set!");
	for (size_t i = 0; i < _strings.size(); i++)
	{
		if (_strings[_start].compare("GET") != 0 && _strings[_start].compare("POST") != 0 && _strings[_start].compare("DELETE") != 0  && _strings[_start].compare("PUT") != 0)
			break;
		methods.push_back(_strings[_start]);
		_start++;
	}
	if (methods.size() > 4)
		error_exit("Too many methods!");
	checkMethods();
}

void		Location::setTryFiles(void)
{
	_start++;
	if (_strings[_start][0] != '/')
		error_exit("Try_files set incorrectly!");
	try_files = _strings[_start];
}

void		Location::setRedirect(void)
{
	_start++;
	for (size_t i = 0; i < _strings[_start].length(); i++)
	{
		if (_strings[_start][i] < '0' || _strings[_start][i] > '9' || _strings[_start].length() != 3)
			error_exit("Redirect set incorrectly!");
	}
	redirect = new std::string[2];
	redirect[0] = _strings[_start];
	_start++;
	redirect[1] = _strings[_start];
}

void		Location::setCGIPath(void)
{
	_start++;
	if (_strings[_start].compare(0, 7, "/Users/") != 0)
		error_exit("CGI path set incorrectly!");
	cgi_path = _strings[_start];
}

void		Location::setAutoindex(void)
{
	_start++;
	if (_strings[_start].compare("on") != 0 && _strings[_start].compare("off") != 0)
		error_exit("Autoindex not set!");
	if (_strings[_start].compare("on") == 0)
		autoindex = true;
}

void		Location::setMaxBodySize(void)
{
	_start++;
	for (size_t i = 0; i < _strings[_start].length(); i++)
	{
		if (_strings[_start][i] < '0' || _strings[_start][i] > '9')
			error_exit("Max_body_size set incorrectly!");
	}
	std::istringstream(_strings[_start]) >> max_body_size;
}

strVector	&Location::getMethods(void)
{
	return this->methods;
}

std::string		*Location::getRedirect(void)
{
	return this->redirect;
}

bool	Location::getAutoindex(void)
{
	return this->autoindex;
}

std::string		&Location::getUri(void)
{
	return this->uri;
}

std::string		&Location::getCGIPath(void)
{
	return this->cgi_path;
}

std::string		&Location::getLocRoot(void)
{
	return this->locRoot;
}

std::string		&Location::getTryFiles(void)
{
	return this->try_files;
}

int				Location::getMaxBodySize(void)
{
	return this->max_body_size;
}

void	Location::checkMethods(void)
{
	int i = 0;
	int n;
	int len = methods.size();
	if (len == 1)
		return;
	while (i < len)
	{
		n = i + 1;
		while (n < len)
		{
			if (methods[i].compare(methods[n]) == 0)
				error_exit("Some methods are the same!");
			n++;
		}
		i++;
	}
}
