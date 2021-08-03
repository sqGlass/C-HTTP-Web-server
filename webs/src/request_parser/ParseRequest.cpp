#include "ParseRequest.hpp"

void ParseRequest::Free(char **A, int length)
{
	if (length > 0)
	{
		for (int i = 0; i < length; i++)
			delete[] A[i];
		delete[] A;
	}
}

ParseRequest::~ParseRequest()
{
	Free(_cgiEnv, envs.size());
}

ParseRequest::ParseRequest(std::string req, Server &serv) : _server(serv)
{
	_autoindex = 0;
	_isCgi = false;
	_cgiEnv = NULL;
	_req = req;
	_flagErr = -1;
	tmpRoot = _server.getRoot();
	_responseCode = 200;
	_maxBodySize = serv.getClientMaxBodySize();
}

int ParseRequest::countToChar(std::string str, char ch)
{
	int i;

	i = 0;
	while (str.at(i) != ch)
		i++;
	return (i);
}

int ParseRequest::scipSpace(std::string str)
{
	int i;

	i = 0;
	while (str.at(i) == ' ')
		i++;
	return (i);
}

void ParseRequest::parseStartString()
{
	int i;

	i = 0;
	while (_req.at(i) == '\r' || _req.at(i) == '\n')
		i++;
	if (_req.compare(i, 3, "GET") == 0)
	{
		_method = _req.substr(i, 3);
		i = i + 3;
	}
	else if (_req.compare(i, 4, "POST") == 0)
	{
		_method = _req.substr(i, 4);
		i = i + 4;
	}
	else if (_req.compare(i, 6, "DELETE") == 0)
	{
		_method = _req.substr(i, 6);
		i = i + 6;
	}
	else if (_req.compare(i, 4, "HEAD") == 0)
	{
		_method = _req.substr(i, 4);
		i = i + 4;
	}
	else if (_req.compare(i, 3, "PUT") == 0)
	{
		_method = _req.substr(i, 3);
		i = i + 3;
	}

	i = i + scipSpace(&_req.at(i));
	_reqPath = _req.substr(i, countToChar(&_req.at(i), ' '));
	i = i + countToChar(&_req.at(i), ' ');
	i = i + scipSpace(&_req.at(i));
	_httpVersion = _req.substr(i, countToChar(&_req.at(i), '\r'));
	i = i + countToChar(&_req.at(i), '\r');
	i = i + 2;
	_req.erase(0, i);
	if (_reqPath.find("?") != std::string::npos)
	{
		_queryString = _reqPath.substr(_reqPath.find("?") + 1);
		_reqPath.erase(_reqPath.find("?"));
	}
}

std::string ParseRequest::cutReqReturnParam(std::string &req, int count)
{
	int countToDel;
	std::string ret;
	if (count != 0)
	{
		countToDel = scipSpace(&_req.at(count));
		req.erase(0, countToDel);
		ret = _req.substr(count, _req.find("\r\n", 0) - count);
	}
	req.erase(0, req.find("\r\n", 0) + 2);
	return (ret);
}

void ParseRequest::parseHeaders()
{
	int i;
	int countToDel;
	std::string tmp;

	i = 0;
	countToDel = 0;
	while (1)
	{
		if (_req.compare(0, 2, "\r\n") == 0)
			break;
		else if (_req.compare(0, 15, "Content-Length:") == 0)
			_contentLength = cutReqReturnParam(_req, 15);
		else if (_req.compare(0, 5, "Host:") == 0)
			_host = cutReqReturnParam(_req, 5);
		else if (_req.compare(0, 13, "Content-Type:") == 0)
			_contentType = cutReqReturnParam(_req, 13);
		else if (_req.compare(0, 8, "Referer:") == 0)
			_referer = cutReqReturnParam(_req, 8);
		else if (_req.compare(0, 18, "Transfer-Encoding:") == 0)
			_transferencoding = cutReqReturnParam(_req, 18);
		else if (_req.compare(0, 5, "Date:") == 0)
			_date = cutReqReturnParam(_req, 5);
		else
		{
			tmp = "HTTP_" + _req.substr(0, _req.find(":", 0));
			_req.erase(0, _req.find(":", 0) + 1);
			_req.erase(0, scipSpace(_req));
			tmp += "=";
			tmp += _req.substr(0, _req.find("\r\n"));
			_xParameters.push_back(tmp);
			cutReqReturnParam(_req, 0);
		}
	}
}

void ParseRequest::validStartString()
{
	if (_method.empty() || (_method.compare("POST") != 0 && _method.compare("DELETE") != 0 && _method.compare("GET") != 0 && _method.compare("PUT") != 0 && _method.compare("HEAD") != 0))
	{
		_responseCode = 405;
	}
	if (_httpVersion.empty() || _httpVersion.length() != 8 || _httpVersion.compare(0, 4, "HTTP") != 0 || _httpVersion.compare(5, 3, "1.1") != 0)
	{
		_responseCode = 405;
	}
}

void ParseRequest::makePath(Location loca)
{
	std::string tmp;
	std::string tmpReqPath;
	if (!loca.getLocRoot().empty())
	{
		tmpRoot = loca.getLocRoot();
	}
	tmpReqPath = _reqPath;
	if (!(loca.getUri().compare("/") == 0))
		tmp.append(tmpRoot).append(tmpReqPath.erase(0, loca.getUri().length()));
	else
		tmp.append(tmpRoot).append(tmpReqPath);

	struct stat stat_buf;
	int st = stat(tmp.c_str(), &stat_buf);
	if (_method.compare("GET") == 0 && st == 0 && (stat_buf.st_mode & S_IFDIR))
	{
		if (tmp[tmp.length() - 1] == '/')
			tmp.erase(tmp.length() - 1);
		_path.append(tmp).append(loca.getTryFiles());
		st = stat(_path.c_str(), &stat_buf);
		if (st == 0 && (stat_buf.st_mode & S_IFREG))
			return;
		else
		{
			if (loca.getAutoindex() == true)
			{
				_path = tmp;
				_autoindex = true;
			}

		}
		return;
	}
	_path.append(tmpRoot).append(tmpReqPath);
}

int ParseRequest::getStatus()
{
	return (_status);
}

int ParseRequest::getLocation(std::vector<Location> locs)
{
	for (size_t i = 0; i < locs.size(); i++)
	{
		if (_reqPath.find(locs.at(i).getUri()) == 0 && locs.at(i).getUri().find(".") == std::string::npos)
		{
			if (_reqPath.find(".") != std::string::npos)
				for (size_t j = 0; j < locs.size(); j++)
				{
					if (_reqPath.find(locs.at(j).getUri().substr(1)) != std::string::npos && locs.at(j).getUri().find(".") != std::string::npos)
					{
						_isCgi = true;
						_cgiPath = locs.at(j).getCGIPath();
						return (i);
					}
				}
			if (locs.at(i).getMaxBodySize() != -1)
				_maxBodySize = locs.at(i).getMaxBodySize();
			return (i);
		}
	}
	return (-1);
}

bool ParseRequest::isMethodAllowed(std::string method, std::vector<std::string> methods)
{
	for (size_t i = 0; i < methods.size(); i++)
		_allowMethods.append(methods.at(i) + " ");
	_allowMethods.erase(_allowMethods.size() - 1);
	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods.at(i).compare(method) == 0)
		{
			return (true);
		}
	}
	_responseCode = 405;
	_status = 4;
	return (false);
}

void ParseRequest::setStatusAndPrepareFd()
{
	std::vector<Location> locs = _server.getLocations();
	int ret = getLocation(locs);

	if (ret == -1)
	{
		_responseCode = 404;
		_status = 4;
		return;
	}
		
	if (locs[ret].getRedirect() != nullptr && _method.compare("GET") == 0) // ||  (_reqPath.compare(locs[ret].getUri()) == 0 && _reqPath.compare("/") != 0))
	{
		// if (_reqPath.compare(locs[ret].getUri()) == 0)
		// {
		// 	_path.append("http://");
		// 	_path.append(_host);
		// 	_path.append(_reqPath.append("/"));
		// 	_responseCode = 302;
		// 	_status = 4;
		// 	return ;
		// }
		std::istringstream(locs[ret].getRedirect()[0]) >> _responseCode;
		_path = locs[ret].getRedirect()[1];
		_status = 4;
		return;
	}
	makePath(locs[ret]);
	if (_method.compare("PUT") == 0)
		processingPutRequest(locs[ret]);
	if (_method.compare("POST") == 0)
		processingPostRequest(locs[ret]);
	if (_method.compare("GET") == 0)
		processingGetRequest(locs[ret]);
	if (_method.compare("HEAD") == 0 && isMethodAllowed(_method, locs[ret].getMethods()))
	{
		_status = 4;
	}
	if (_method.compare("DELETE") == 0 && isMethodAllowed(_method, locs[ret].getMethods()))
	{
		_status = 4;
	}
}

void ParseRequest::processingGetRequest(Location loca)
{
	if (_isCgi)
	{
		parseCGI();
		struct stat stat_buf;
		int st = stat(_path.c_str(), &stat_buf);
		if (st == 0 && stat_buf.st_mode & S_IFREG)
		{
			_fd = _path;
			_status = 7;
		}
		else
		{
			_responseCode = 404;
			isMethodAllowed(_method, loca.getMethods());
			_status = 4;
		}
	}
	else
	{
		isMethodAllowed(_method, loca.getMethods());
		_status = 4;
	}
}

void ParseRequest::processingPostRequest(Location loca)
{
	std::size_t pos;
	std::string fileName;
	if (_isCgi)
	{
		parseCGI();
	}
	else if (!isMethodAllowed(_method, loca.getMethods()))
	{
		_responseCode = 405;
		if (_transferencoding.compare("chunked") == 0)
			_status = 2;
		else
			_status = 3;
		return;
	}
	pos = _path.rfind('/');
	if (pos == std::string::npos || _path.length() == 0 || _path.length() - 1 == pos)
	{
		_responseCode = 400;
		if (_transferencoding.compare("chunked") == 0)
			_status = 2;
		else
			_status = 3;
		return;
	}
	if (_transferencoding.compare("chunked") == 0)
		_status = 2;
	else
		_status = 3;
	_fd = _path;
}

void ParseRequest::processingPutRequest(Location loca)
{
	std::size_t pos;
	std::string fileName;
	if (_isCgi)
	{
		parseCGI();
	}
	else if (!isMethodAllowed(_method, loca.getMethods()))
	{
		_responseCode = 405;
		if (_transferencoding.compare("chunked") == 0)
			_status = 2;
		else
			_status = 3;
		return;
	}
	pos = _path.rfind('/');
	if (pos == std::string::npos || _path.length() == 0 || _path.length() - 1 == pos)
	{
		_responseCode = 400;
		if (_transferencoding.compare("chunked") == 0)
			_status = 2;
		else
			_status = 3;
		return;
	}
	if (_transferencoding.compare("chunked") == 0)
		_status = 2;
	else
		_status = 3;
	_fd = _path;
}

void ParseRequest::parseCGI()
{
	if (!_contentLength.empty())
		envs.push_back("CONTENT_LENGTH=" + _contentLength);
	envs.push_back("CONTENT_TYPE=" + _contentType);
	envs.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envs.push_back("SERVER_SOFTWARE=*NIX");
	envs.push_back("REMOTE_ADDR=" + _remoteAddr);
	envs.push_back("REMOTE_PORT=" + _remotePort);
	envs.push_back("SERVER_NAME=" + _server.getServerName());
	envs.push_back("SERVER_PORT=" + std::to_string(_server.getPort()));
	envs.push_back("SERVER_ADDR=" + _server.getHost());
	// envs.push_back("SCRIPT_NAME=" + tmpRoot + _reqPath);
	// envs.push_back("REQUEST_URI=" + _reqPath);
	// envs.push_back("REQUEST_URI=localhost:8080/wordpress");
	envs.push_back("SCRIPT_FILENAME=" + tmpRoot + _reqPath); // nado 4tob on bil etot path
	// envs.push_back("SCRIPT_FILENAME=setup-config.php"); // nado 4tob on bil etot path
	envs.push_back("QUERY_STRING=" + _queryString);
	envs.push_back("PATH_INFO=" + _reqPath);
	envs.push_back("SERVER_PROTOCOL=" + _httpVersion);
	envs.push_back("PATH_TRANSLATED=" + _path);
	envs.push_back("REQUEST_METHOD=" + _method);
	envs.push_back("REDIRECT_STATUS=200");
	envs.push_back("HTTP_HOST=" + getHost());
	for (size_t i = 0; i < _xParameters.size(); i++)
		envs.push_back(_xParameters.at(i));
	if (!envs.empty())
	{
		_cgiEnv = new char *[envs.size() + 1];
		for (size_t i = 0; i < envs.size(); i++)
		{
			_cgiEnv[i] = strdup(envs.at(i).c_str());
		}
		_cgiEnv[envs.size()] = 0;
	}
}

std::vector<std::string> ParseRequest::getCgiEnvsInVector()
{
	return (envs);
}

char **ParseRequest::getCgiEnvs()
{
	return (_cgiEnv);
}
bool ParseRequest::getIsCgi()
{
	return (_isCgi);
}
void ParseRequest::setResponseCode(int responseCode)
{
	_responseCode = responseCode;
}

std::string ParseRequest::getPath()
{
	return (_path);
}

std::string ParseRequest::getRequest()
{
	return (_req);
}

std::string ParseRequest::getCgiPath()
{
	return (_cgiPath);
}

int ParseRequest::getFlagErr()
{
	return (_flagErr);
}
std::string ParseRequest::getMethod()
{
	return (_method);
}
std::string ParseRequest::getReqPath()
{
	return (_reqPath);
}
std::string ParseRequest::getHttpVersion()
{
	return (_httpVersion);
}
std::string ParseRequest::getHost()
{
	return (_host);
}

int ParseRequest::getResponseCode()
{
	return (_responseCode);
}

std::string ParseRequest::getAllowMethods()
{
	return (_allowMethods);
}

std::string ParseRequest::getFd()
{
	return (_fd);
}

int ParseRequest::getMaxBodySize()
{
	return (_maxBodySize);
}

void ParseRequest::setRemotePort(std::string port)
{
	_remotePort = port;
}
void ParseRequest::setRemoteAddr(std::string addr)
{
	_remoteAddr = addr;
}
std::string ParseRequest::getRemotePort()
{
	return (_remotePort);
}
std::string ParseRequest::getRemoteAddr()
{
	return (_remoteAddr);
}
 Server &ParseRequest::getServer()
 {
	 return (_server);
 }
 bool ParseRequest::getAutoindex()
 {
	 return (_autoindex);
 }