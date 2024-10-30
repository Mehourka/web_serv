/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oroy <oroy@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 12:09:00 by oroy              #+#    #+#             */
/*   Updated: 2024/10/30 14:47:22 by oroy             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
# define HTTPHANDLER_HPP

# include <unistd.h>
# include <sys/stat.h>
# include <fstream>
# include <iostream>
# include <sstream>
# include <string>
# include <vector>
# include <dirent.h>
# include <map>
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Config.hpp"
# include "ConfigServer.hpp"
# include "utils.hpp"

class HttpHandler
{
public:
private:

	// Parameters at contruction -- Start

	Config &							_conf;
	ConfigServer*						_config;
	HttpResponse*						_response;
	std::string const					_baseDir;
	

	bool								_autoIndex;
	bool								_getContentFromFile;
	int									_statusCode;
	std::string							_content;
	std::string							_contentType;
	std::string							_htmlFile;
	std::string							_path;
	std::string							_cgi_bin;


	void								_setRequestParameters(HttpRequest const &request);
	std::string							_parseTarget(std::string const &target);
	std::string 						_createPath( void );
	std::string							_getPage(short const & errorCode);
	void								_openFile();
	void								_server_msg( void );
	bool								_isDirectory(const char *path);
	bool								_check_40x_error( HttpRequest const &request );
	bool								_check_redirect(void);

	void								_get_post(HttpRequest const &request);
	void								_delete( void );

	void								_populateResponse(HttpRequest const &request);

	std::string const					_autoIndexGenerator(std::string & path, std::string const & target);
	void								_generateError();


public:

	HttpHandler(Config &conf);
	~HttpHandler();

	ConfigServer*						getConfigServer( void ) { return _config; };
	std::string	const					getPath( void ) const { return _path; };
	std::string							getCGIbin() const { return _cgi_bin; };
	bool								isCgi( void ) const; 
	void								buildResponse(HttpRequest const &request, HttpResponse &response);
	bool								checkCgi(HttpRequest const &request) const;
};

#endif
