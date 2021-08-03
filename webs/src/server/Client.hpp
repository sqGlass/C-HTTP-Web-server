#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include "./../request_parser/Response.hpp"
#include "./../config_parser/Server.hpp"

class Client
{
private:
	int					clientSocket;
	int					status;
	int					chunkedStatus;
	size_t				chunkTotalReceive;
	size_t				maxBody;
	bool				isCgi;
	char**				cgiEnv;
	Server				serverConfig;
	std::string			filePath;
	std::string			response;
	std::string			cgiPath;
	std::string			chunkedRequestLength;
	std::string			requestBuf;
	std::string			chunkedSaveLine;
	long				chunkedLineLength;
	struct sockaddr_in	clientAddr;
	bool				isLastChunk;

	ParseRequest	&parseHeader(std::map<std::string, std::string> &contentType);
	std::string		ipToString(unsigned long ip);
	void			replace(const char *toFind, std::string &replaceString);
public:
	Client(int socket, struct sockaddr_in &sockAddr, Server &config);
	~Client();
	void	parseRequestHeader(std::map<std::string, std::string> &contentType, std::string &requestHeader);
	int		getSocket();
	int		recieveRequest(std::map<std::string, std::string> &contentType);
	int		getStatus();
	int		sendResponse();
	void	cgiMagic();
	size_t	getReponseLength();
};

#endif
