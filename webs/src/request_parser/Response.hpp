/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sglass <sglass@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/06/17 18:34:35 by sglass            #+#    #+#             */
/*   Updated: 2021/06/30 16:04:58 by sglass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef RESPONSE_H
#define RESPONSE_H
    #include <string>
    #include <iostream>
    #include <sstream>
    #include <cstring> 
	#include "ParseRequest.hpp"
	#include <map>
	#include <dirent.h>
	#include <iostream>
	#include <list>

class Response
{
    private:
		std::string _response;
		ParseRequest &_pars;
		int _fd;
		int _fileSize;
		std::string _contentFile;

    public:
        Response(ParseRequest &pars);
        ~Response();
		void makeResponseHead(std::map<std::string, std::string> &contentType);
		void makeResponseBody();
		void validRequest();
		std::string &getResponse();
		std::string &getContentFile();
		int getFd();
	    int getFileSize();


		std::string findContentType(std::string path, std::map<std::string, std::string> &contentType);
		std::string getFileExtension(std::string path);

		std::list<std::string> listDirs(std::string &currentDirectory, std::string &requestUrl);
		std::string	makeAutoindexBody(std::list<std::string> &listingDirs);
};
void returnErrorResponse(Server &serv, std::string &response, int errCode);

#endif