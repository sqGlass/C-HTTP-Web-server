/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseRequest.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sglass <sglass@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/06/18 16:21:08 by sglass            #+#    #+#             */
/*   Updated: 2021/06/30 15:40:49 by sglass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef PARSEREQUEST_H
#define PARSEREQUEST_H
    #include <string>
    #include <iostream>
    #include <sstream>
    #include <cstring>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
    #include <stdlib.h>
    #include "../config_parser/Server.hpp"

class ParseRequest
{
    private:
        char** _cgiEnv;

        std::string _req;
        Server &_server;
        int _flagErr;
        bool _autoindex;
        std::string _method;
        std::string _reqPath;
        std::string _httpVersion;
        std::string _queryString;
        std::string _cgiPath;

        std::string _allowMethods;

        std::vector<std::string> _xParameters;
        std::vector<std::string> envs;
		std::string _host;
        std::string _contentLength;
        std::string _contentType;
        std::string _boundary;
        std::string _referer;
        std::string _remoteAddr;
        std::string _remotePort;
        std::string _transferencoding;
        std::string _date;

        std::string tmpRoot;
		std::string _path;
		int _responseCode;
		int _status;
        std::string _fd;
        int _maxBodySize;
        bool _isCgi;


    public:
        ParseRequest(std::string req, Server &serv);
        ParseRequest();
        ~ParseRequest();
        void parseStartString();
        void parseHeaders();

        void validStartString();
        void makePath(Location loca);
        int getStatus();
        std::string getFd();
	    void setResponseCode(int responseCode);
        void processingPutRequest(Location loca);
        
        void processingPostRequest(Location loca);
        void processingGetRequest(Location loca);
		void setStatusAndPrepareFd();
        std::string getPath();

		
        std::string getRequest();
		std::string cutReqReturnParam(std::string& req, int count);

        int countToChar(std::string req,char ch);
        int scipSpace(std::string str);


        int getFlagErr();
        std::string getMethod();
        std::string getReqPath();
        std::string getHttpVersion();
        std::string getHost();
        void setRemotePort(std::string port);
        void setRemoteAddr(std::string addr);
        std::string getRemotePort();
        std::string getRemoteAddr();
		int getResponseCode();
        std::string getAllowMethods();
        char** getCgiEnvs();
        std::vector<std::string> getCgiEnvsInVector();


        int getLocation(std::vector <Location> locs);
        bool isMethodAllowed(std::string method, std::vector <std::string> methods);
        void parseCGI();
        bool getIsCgi();
        Server &getServer();
        std::string getCgiPath();
        int getMaxBodySize();
        void Free(char** A, int length);
        bool getAutoindex();
    };
#endif
