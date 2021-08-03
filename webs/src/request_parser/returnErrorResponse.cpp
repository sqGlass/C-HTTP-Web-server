#include "../config_parser/Server.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "map"

void returnErrorResponse(Server &serv, std::string &response, int errCode)
{
	int fd;
    int fileSize;
    int start;
    int finish;
    std::string contentErrFile;
    std::string tmp;
    struct stat stat_buf;
    std::map<std::string, std::string> errors = serv.getErrors();
    if (errCode != 403 && errCode != 404 && errCode != 405 && errCode != 413)
        errCode = 666;
    std::string error = errors.find(std::to_string(errCode))->second;

    fd = open(error.c_str(), O_RDWR, 666);
    fstat(fd, &stat_buf);
    // fileSize = stat_buf.st_size;
    // contentErrFile.reserve(fileSize);
    char buf[128000];
    int res;
    while ((res = read(fd, buf, 128000)) != 0)
    {
        if (res < 0)
        {
            std::cout << "error while reading\n";
            exit(1);
        }
        contentErrFile.append(buf, res);
    }
    fileSize = contentErrFile.length();
    tmp = "Content-Length: " + std::to_string(fileSize);
    start = response.find("Content-Length: ");
    finish = response.find("\r\n", start);
    response.replace(start, finish - start, tmp);
    tmp = "Content-Type: text/html";
    start = response.find("Content-Type: ");
    finish = response.find("\r\n", start);
    response.replace(start, finish - start, tmp);
    response.append(contentErrFile);
    close(fd);
}