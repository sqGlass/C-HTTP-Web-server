#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <vector>
# include <fstream>
# include <string>
# include <sstream>
# include <unistd.h>

typedef std::vector<std::string> strVector;

class Location
{
private:
	strVector			_strings;
	size_t				_start;
	strVector			methods;
	std::string*		redirect;
	std::string			uri;
	std::string			try_files;
	std::string			locRoot;
	std::string			cgi_path;
	bool				autoindex;
	int					max_body_size;


	void		setValue(void);
	void		setMethods(void);
	void		setAutoindex(void);
	void		setRedirect(void);
	void		setTryFiles(void);
	void		setLocRoot(void);
	void		setCGIPath(void);
	void		setMaxBodySize(void);
public:
	Location(void);
	Location(strVector strings, size_t start);
	~Location(void);

	void			checkMethods(void);
	strVector		&getMethods(void);
	bool			getAutoindex(void);
	std::string*	getRedirect(void);
	std::string		&getUri(void);
	std::string		&getTryFiles(void);
	std::string		&getLocRoot(void);
	std::string		&getCGIPath(void);
	int				getMaxBodySize(void);
};

#endif