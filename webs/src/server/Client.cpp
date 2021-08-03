#include "Client.hpp"

#define READY_FOR_REQUEST 1
#define CHUNKED_RECEIVE 2
#define BODYBUFF_RECEIVE 3
#define READY_TO_RESPONSE 4
#define CURRENTLY_RESPONDING 5
#define CHUNKED_SEND 6
#define CGI_GET 7
#define LENGTH_RECEIVING 10
#define BUF_RECEIVING 11
#define BUFFER_SIZE 32768

Client::Client(int socket, struct sockaddr_in &sockAddr, Server &config)
: clientSocket(socket), status(READY_FOR_REQUEST),
chunkedStatus(LENGTH_RECEIVING), chunkTotalReceive(0),
serverConfig(config), clientAddr(sockAddr)
{
	isCgi = false;
	isLastChunk = false;
}

Client::~Client()
{}

int	Client::getSocket()
{
	return clientSocket;
}

int Client::getStatus()
{
	return status;
}

size_t	Client::getReponseLength()
{
	return response.length();
}

int Client::sendResponse()
{
	long sent = send(clientSocket, response.c_str(), response.length(), 0);
	if (sent <= 0)
		return sent;
	if (sent - response.length() == 0)
	{
		status = READY_FOR_REQUEST;
		response.clear();
		chunkedRequestLength.clear();
		chunkedLineLength = 0;
		chunkedSaveLine.clear();
		filePath.clear();
		chunkTotalReceive = 0;
	}
	else
		response = response.substr(sent);
	return 1;
}

std::string	Client::ipToString(unsigned long ip)
{
	std::string result;

	result += std::to_string(ip & 0xFF) + '.';
	result += std::to_string(ip >> 8 & 0xFF) + '.';
	result += std::to_string(ip >> 16 & 0xFF) + '.';
	result += std::to_string(ip >> 24 & 0xFF);
	return result;
}

void Client::parseRequestHeader(std::map<std::string, std::string> &contentType, std::string &requestHeader)
{
	ParseRequest pars(requestHeader, serverConfig);
	pars.setRemoteAddr(ipToString(clientAddr.sin_addr.s_addr));
	pars.setRemotePort(std::to_string(clientAddr.sin_port));
	pars.parseStartString();
    pars.parseHeaders();
	pars.validStartString();
	pars.setStatusAndPrepareFd();
	status = pars.getStatus();
	cgiPath = pars.getCgiPath();
	if (status == CHUNKED_RECEIVE || status == CGI_GET)
	{
		filePath = pars.getFd();
		std::cout << "file path: " << filePath << std::endl;
	}
	if (pars.getIsCgi() == true && !filePath.empty())
	{
		isCgi = true;
		cgiEnv = new char*[pars.getCgiEnvsInVector().size() + 1];
		cgiEnv[pars.getCgiEnvsInVector().size()] = 0;
		for (int i = 0; pars.getCgiEnvs()[i] != 0; i++)
			cgiEnv[i] = strdup(pars.getCgiEnvs()[i]);
	}
	maxBody = pars.getMaxBodySize();
	Response resp(pars);
	resp.validRequest();
    resp.makeResponseHead(contentType);
    resp.makeResponseBody();
	this->response = resp.getResponse();
}

int	Client::recieveRequest(std::map<std::string, std::string> &contentType)
{
	char buf[BUFFER_SIZE];
	memset(buf, '\0', BUFFER_SIZE);
	long received = recv(clientSocket, buf, BUFFER_SIZE, 0);
	if (received == -1)
	{
		std::cerr << "error with recv\n";
		return -1;
	}
	if (received == 0)
	{
		std::cout << "connection closed while receiving\n";
		return 0;
	}
	requestBuf.append(buf, received);
	if (status == READY_FOR_REQUEST)
	{
		size_t headerAndBodySplitter;
		headerAndBodySplitter = requestBuf.find("\r\n\r\n");
		if (headerAndBodySplitter != std::string::npos)
		{
			std::string requestHeader = requestBuf.substr(0, headerAndBodySplitter + 4);
			requestBuf.erase(0, headerAndBodySplitter + 4);
			parseRequestHeader(contentType, requestHeader);
		}
	}
	if (status == CGI_GET)
	{
		cgiMagic();
		for (int i = 0; cgiEnv[i] != 0; i++)
			delete cgiEnv[i];
		delete[] cgiEnv;
		isCgi = false;
		status = READY_TO_RESPONSE;
		return 1;
	}
	if (status == CHUNKED_RECEIVE && !requestBuf.empty())
	{
		if (!filePath.empty())
		{
			size_t splitter;
			while ((splitter = requestBuf.find("\r\n")) != std::string::npos)
			{
				if (chunkedStatus == LENGTH_RECEIVING)
				{
					chunkedRequestLength = requestBuf.substr(0, splitter);
					chunkedLineLength = std::strtol(chunkedRequestLength.c_str(), NULL, 16);
					requestBuf.erase(0, splitter + 2);
					if (chunkedLineLength == 0)
					{
						isLastChunk = true;
					}
					chunkedStatus = BUF_RECEIVING;
					chunkTotalReceive += chunkedLineLength;
					if (chunkTotalReceive > maxBody)
					{
						filePath.clear();
						size_t contLen = response.find("HTTP/1.1 ");
						response.replace(contLen, 13, "HTTP/1.1 413");
						break;
					}
					continue;
				}
				if (chunkedStatus == BUF_RECEIVING)
				{
					std::string lastLine = requestBuf.substr(0, chunkedLineLength);
					chunkedSaveLine.append(lastLine);
					requestBuf.erase(0, splitter + 2);
					chunkedStatus = LENGTH_RECEIVING;
					if (isLastChunk == true && lastLine.empty())
					{
						isLastChunk = false;
						int openFile = open(filePath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
						if (openFile > 0)
						{
							long res = 0;
							while (!chunkedSaveLine.empty())
							{
								res = write(openFile, chunkedSaveLine.c_str(), chunkedSaveLine.length());
								if (res < 0)
								{
									std::cerr << "ERROR WHILE SAVING FILE WITH CHUNKED LINE\n";
									exit(1);
								}
								chunkedSaveLine = chunkedSaveLine.substr(res);
							}
							close(openFile);
							if (isCgi == true)
							{
								cgiMagic();
								for (int i = 0; cgiEnv[i] != 0; i++)
									delete cgiEnv[i];
								delete[] cgiEnv;
								isCgi = false;
							}
						}
						status = READY_TO_RESPONSE;
						return 1;
					}
				}
			}
		}
		if (filePath.empty())
		{
			size_t splitter;
			while ((splitter = requestBuf.find("\r\n")) != std::string::npos)
			{
				if (chunkedStatus == LENGTH_RECEIVING)
				{
					chunkedRequestLength = requestBuf.substr(0, splitter);
					chunkedLineLength = std::strtol(chunkedRequestLength.c_str(), NULL, 16);
					if (chunkedLineLength == 0)
					{
						isLastChunk = true;
					}
					requestBuf.erase(0, splitter + 2);
					chunkedStatus = BUF_RECEIVING;
					continue;
				}
				if (chunkedStatus == BUF_RECEIVING)
				{
					std::string lastLine = requestBuf.substr(0, chunkedLineLength);
					requestBuf.erase(0, splitter + 2);
					chunkedStatus = LENGTH_RECEIVING;
					if (isLastChunk == true && lastLine.empty())
					{
						isLastChunk = false;
						status = READY_TO_RESPONSE;
						return 1;
					}
					continue;
				}
			}
		}
	}
	return 1;
}

void Client::cgiMagic()
{
	int pid;
	pid = fork();
	char *cgiArgs[2];
	cgiArgs[0] = (char *)cgiPath.c_str();
	cgiArgs[1] = nullptr;
	if (pid == 0)
	{
		int fileIn = open(filePath.c_str(), O_RDONLY);
		if (fileIn < 0)
		{
			std::cerr << "cgi fileIn open issue\n";
			exit(1);
		}
		dup2(fileIn, 0);
		close(fileIn);
		int fileOut = open("./cgiOut.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
		if (fileOut < 0)
		{
			std::cerr << "cgi fileOut open issue\n";
			exit(1);
		}
		dup2(fileOut, 1);
		close(fileOut);
		execve(cgiArgs[0], cgiArgs, cgiEnv);
	}
	int cgiStatus = 0;
	waitpid(pid, &cgiStatus, 0);
	if (cgiStatus > 0)
		std::cerr << "cgi error finished: " << cgiStatus << std::endl;
	int cgiResonseFile = open("./cgiOut.txt", O_RDONLY, 0644);
	if (cgiResonseFile < 0)
	{
		std::cerr << "cgi cgiResponseFile open issue\n";
		exit(1);
	}
	char buf[BUFFER_SIZE];
	memset(buf, '\0', BUFFER_SIZE);
	long readCount;
	std::string	cgiResponse;
	while ((readCount = read(cgiResonseFile, buf, BUFFER_SIZE)) != 0)
	{
		if (readCount == -1)
		{
			std::cerr << "READ COUNT RETURNED -1\n";
			exit(1);
		}
		cgiResponse.append(buf, readCount);
		memset(buf, '\0', BUFFER_SIZE);
	}
	close(cgiResonseFile);
	size_t headerSplitter = cgiResponse.find("\r\n\r\n");
	std::string contentLength;
	if (headerSplitter == std::string::npos)
	{
		contentLength = "Content-Length: " +
						std::to_string(cgiResponse.length());
		replace("Content-Length: ", contentLength);
	}
	else
	{
		contentLength = "Content-Length: " +
						std::to_string(cgiResponse.length() - (headerSplitter + 4));
		replace("Content-Length: ", contentLength);
		std::string newStatus;
		size_t statusBegin = cgiResponse.find("Status");
		if (statusBegin != std::string::npos)
		{
			newStatus = cgiResponse.substr(statusBegin, cgiResponse.find("\r\n", statusBegin));
			newStatus = newStatus.substr(7, 10);
			newStatus.insert(0, "HTTP/1.1 ");
			replace("HTTP/1.1 ", newStatus);
		}
		response.replace(response.find("\r\n\r\n"), 4, "\r\n");
	}
	response.append(cgiResponse);
}

void	Client::replace(const char *toFind, std::string &replaceString)
{
	size_t contLen = response.find(toFind);
	if (contLen == std::string::npos)
	{
		std::cerr << "contlen fault\n";
		exit(1);
	}
	size_t contLenEnd = response.find("\r\n", contLen);
	if (contLen == std::string::npos)
	{
		std::cerr << "contlen END fault\n";
		exit(1);
	}
	response.replace(contLen, contLenEnd - contLen, replaceString);
}
