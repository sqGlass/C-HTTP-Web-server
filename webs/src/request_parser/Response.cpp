#include "Response.hpp"

Response::Response(ParseRequest &pars) : _pars(pars)
{
	_fileSize = 0;
}

Response::~Response()
{
}

void Response::makeResponseHead(std::map<std::string, std::string> &contentType)
{
	if (_pars.getResponseCode() >= 300 &&  _pars.getResponseCode() <= 307)
	{
		_response.append("HTTP/1.1").append(" ").append(std::to_string(_pars.getResponseCode())).append("\r\n");
		_response.append("Version: ").append("HTTP/1.1").append("\r\n");
		_response.append("Allow: ").append(_pars.getAllowMethods()).append("\r\n");
		_response.append("Location: ").append(_pars.getPath()).append("\r\n");
		_response.append("\r\n");
	}
	else
	{
		_response.append("HTTP/1.1").append(" ").append(std::to_string(_pars.getResponseCode())).append("\r\n");
		_response.append("Version: ").append("HTTP/1.1").append("\r\n");
		// _response.append("Allow: ").append(_pars.getAllowMethods()).append("\r\n");
		_response.append("Content-Type: ").append(findContentType(_pars.getPath(), contentType)).append("\r\n");
		if (_pars.getResponseCode() == 200)
			_response.append("Content-Length: ").append(std::to_string(_fileSize).append("\r\n"));
		else
			_response.append("Content-Length: 0").append("\r\n");
		_response.append("Allow: ").append(_pars.getAllowMethods());
	_response.append("\r\n\r\n");
	}
}

void Response::makeResponseBody()
{
	if (_pars.getResponseCode() == 200 && !_pars.getIsCgi())
	{
		if (_pars.getMethod().compare("GET") == 0)
		{
			_response.append(_contentFile);
			if (_pars.getAutoindex())
			{
				std::string tmp = "Content-Type: text/html";
    			int start = _response.find("Content-Type:");
    			int finish = _response.find("\r\n", start);
				_response.replace(start, finish - start, tmp);
			}
		}
	}
	else if (_pars.getMethod().compare("HEAD") != 0 && _pars.getResponseCode() != 204 && !_pars.getIsCgi() && (_pars.getResponseCode() < 300 || _pars.getResponseCode() > 307))
	{
		returnErrorResponse(_pars.getServer(), _response, _pars.getResponseCode());
	}
}

std::string& Response::getResponse()
{
	return(_response);
}


std::string Response::getFileExtension(std::string path)
{
	std::string fileExtension;
	size_t i;
	int last;

	i = 0;
	last = -1;
	while(i < path.length())
	{
		if (path.at(i) == '/')
			last = i;
		i++;
	}
	if (path.length() == 0 || (int)path.length() - 1 == last)
		return (std::string());
	fileExtension = path.substr(last + 1, path.length() - last - 1);
	int res = fileExtension.find(".", 0);
	if (res == -1)
		return (std::string());
	fileExtension = fileExtension.substr(res + 1, fileExtension.length() - res - 1);
	return (fileExtension);
}


std::string Response::findContentType(std::string path, std::map<std::string, std::string> &content)
{
    std::string contentType;
    std::map<std::string, std::string>::iterator find;
    std::string key = getFileExtension(path);
    find = content.find(key);
    if (find != content.end())
        contentType = find->second;
    else
        contentType = "";
    return (contentType);
}

void Response::validRequest()
{
	if (_pars.getMethod().compare("GET") == 0 && (_pars.getResponseCode() < 300 || _pars.getResponseCode() > 307) && _pars.getResponseCode() != 405 && !_pars.getIsCgi() && _pars.getAutoindex())
	{
		std::string currentDir = _pars.getPath();
		currentDir.append("/");
		std::string requestUrl = _pars.getReqPath();
		if (requestUrl.compare("/") != 0)
			requestUrl.append("/");
		std::list<std::string> listingDirs = listDirs(currentDir, requestUrl);
		std::string autoindexBody = makeAutoindexBody(listingDirs);
		_contentFile.append(autoindexBody);
		_fileSize = autoindexBody.size();
		return ;
	}
	if (_pars.getMethod().compare("GET") == 0 && (_pars.getResponseCode() < 300 || _pars.getResponseCode() > 307) && _pars.getResponseCode() != 405 && !_pars.getIsCgi()) // dobav !issgi
	{
		struct stat stat_buf;
		_fd = open(_pars.getPath().c_str(), O_RDWR, 666);
		if (_fd < 0)
		{
			_pars.setResponseCode(404);
			return ;
		}
    	int rc = fstat(_fd, &stat_buf);
		if (rc == -1)
		{
			close(_fd);
			_pars.setResponseCode(1);
			return;
		}
		_fileSize = stat_buf.st_size;
		_contentFile.reserve(_fileSize);
		char buf[128000];
		int res;
		while ((res = read(_fd, buf, 128000)) != 0)
		{
			if (res < 0)
			{
				std::cout << "error while reading\n";
				exit(1);
			}
			_contentFile.append(buf, res);
		}
		close(_fd);
	}
	if (_pars.getMethod().compare("DELETE") == 0)
	{
		struct stat stat_buf;
		int ret;
		stat(_pars.getPath().c_str(), &stat_buf);
		if (stat_buf.st_mode & 0100000)
		{
			ret = std::remove(_pars.getPath().c_str());
			if (ret == 0)
				_pars.setResponseCode(204);
			else
				_pars.setResponseCode(404);
		}
		else
			_pars.setResponseCode(404);
	}
}

int Response::getFd()
{
	return(_fd);
}

int Response::getFileSize()
{
	return(_fileSize);
}

std::string &Response::getContentFile()
{
	return (_contentFile);
}

std::list<std::string> Response::listDirs(std::string &currentDirectory, std::string &requestUrl)
{
	std::list<std::string>	listingDirs;
	DIR *dir = opendir(currentDirectory.c_str());
	if (dir)
	{
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_type == DT_DIR && *ent->d_name != '.')
				listingDirs.push_back(requestUrl + ent->d_name);
		}
		closedir(dir);
	}
	else
	{
		std::cerr << "Error opening directory\n";
	}
	return listingDirs;
}

std::string	Response::makeAutoindexBody(std::list<std::string> &listingDirs)
{
	std::string autoindexBody;
	for (std::list<std::string>::iterator begin = listingDirs.begin(); begin != listingDirs.end(); ++begin)
		autoindexBody.append("<p><a href=\"" + *begin + "\">" + begin->substr(begin->find_last_of("/") + 1) + "</a><p>" + "\n");
	return autoindexBody;	
}

