#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Server.hpp"
# include <iostream>
# include <vector>
# include <fstream>
# include <string>
# include <sstream>

class Config
{
private:
	std::vector<Server>	servers;
	strVector			_strings;
	Config(void);
	void	checkValid(void);
public:
	Config(std::string path_to_config);
	Config(Config const &obj);
	Config &operator= (Config const &);
	~Config(void);

	strVector	readFile(std::string path);
	strVector	split(std::string str, std::string charset);
	int		countServers(strVector &strings);
	void	excludeCopy(void);
	std::vector<Server> &getServers();
};

#endif
