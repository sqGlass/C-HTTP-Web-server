#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include <map>

class Server
{
private:
	strVector							_strings;
	size_t								_start;
	std::string							serverName;
	std::string							host;
	int									port;
	int									client_max_body_size;
	std::map<std::string, std::string>	errors;
	std::vector<Location>				locations;
	std::string							host_port;
	std::string							root;


	void		setValue(void);
	void		setHost(void);
	void		setServerName(void);
	void		setPort(void);
	void		setErrors(void);
	void		setClientMaxBodySize(void);
	void		setLocations(void);
	void		setRoot(void);
	void		sortLocations(void);
public:
	Server(void);
	Server(strVector &strings, size_t start);
	~Server(void);

	void								print();
	std::string							&getServerName(void);
	std::string							&getHost(void);
	int									getPort(void);
	int									getClientMaxBodySize(void);
	std::map<std::string, std::string>	&getErrors(void);
	std::vector<Location>				&getLocations(void);
	std::string							&getHostPort(void);
	std::string							&getRoot(void);
};

#endif
