#ifndef CONFIGSERVER_HPP
# define CONFIGSERVER_HPP

# include "Common.hpp"
# include "Config.hpp"
# include <vector>
# include <string>
# include <map>
# include <unordered_set>
# include <arpa/inet.h>
# include <sstream>
# include <algorithm>

using namespace std;

# define BOLD "\033[1;37m"
# define END "\033[0m"
# define G "\033[0;32m"
# define B "\033[0;34m"
# define R "\033[0;31m"

class Config;

class ConfigServer {
public:
	ConfigServer(vector<pair<string, unsigned> > const & conf, Config & config);
	ConfigServer ( ConfigServer const & src);
	~ConfigServer( void );

	ConfigServer &		operator=( ConfigServer const & rhs );


	unsigned short		getPort() const;
	string				getHost() const;
	string				getServerName() const;
	long long			getClientMaxBodySize() const;
	string				getErrorPage(short const & errorcode) const;
	map<short, string>&	getErrorPageMap() const;

	bool				ValidatePort(string& line, string N) const;
	string 				trim(const std::string& str);


private:
	vector<pair<string, unsigned> > 	_Block;
	Config&								_Config;
	unsigned short						_Port;
	string								_Host;
	string								_ServerName;
	long long							_ClientMaxBodySize;
	map<short, string>					_ErrorPages;
	// bool								_AutoIndex;

	void				Parseline(pair<string, unsigned> & linepair, string& line);
	void				ParseListen(pair<string, unsigned> & linepair);
	void				ParseHost(pair<string, unsigned> & linepair);
	void				ParseServerName(pair<string, unsigned> & linepair);
	void				ParseClientMaxBodySize(pair<string, unsigned> & linepair);
	void				ParseErrorPage(pair<string, unsigned> & linepair);
	void				ParseErrorCodes(std::vector<std::string> &tokens, unsigned const & linenumber) const;


};

std::ostream &			operator<<( std::ostream & o, ConfigServer const & i );

#endif /* ********************************************************** SERVER_H */