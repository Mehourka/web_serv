/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmehour <kmehour@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 12:30:55 by oroy              #+#    #+#             */
/*   Updated: 2024/10/28 16:10:02 by kmehour          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpHandler.hpp"
#include "../includes/CgiHandler.hpp"

HttpHandler::HttpHandler(Config &conf) : _conf(conf), _baseDir("./data"), _content("") {}

HttpHandler::~HttpHandler() {}

/*	Functions	************************************************************* */

void	HttpHandler::_setRequestParameters(HttpRequest const &request)
{
	_autoIndex = false;
	_contentType = "text/html";
	_getContentFromFile = false;
	_htmlFile = _parseTarget(request.target());
	_path = _createPath();
	_statusCode = 200;
}

void	HttpHandler::buildResponse(HttpRequest const &request, HttpResponse &response)
{
	try {
		ConfigServer config = _conf.getServerConfig(request.getHeader("host"), request.target());
		_config = &config;
		_response = &response;
		_cgi_bin = config.getCGIbin();
		
		_conf.printMsg(B, "Server[%s]: request recieved [Method[%s] Target[%s] Version[%s]]", _config->getServerName().c_str(), request.method().c_str(), request.target().c_str(), request.version().c_str());
		_setRequestParameters(request);
		bool	redirect = _check_redirect();
		if (!_check_40x_error( request ) && !redirect)
		{
			_contentType = response.getContentType(getExtension(_htmlFile));
			if (request.method() == "GET" || request.method() == "POST")
				_get_post(request);
			else if (request.method() == "DELETE")
				_delete();
		}
		if (!redirect){
			while (_getContentFromFile)
				_openFile();
		}
		_server_msg();
		_populateResponse(request);
	}
	catch (exception &e){
		cerr << "webserv: " << R << "ERROR" << END << "[" << e.what() << "]" << endl;
	}
}

bool	HttpHandler::_check_redirect(void)
{
	if (_config->getRedirect().first != 0)
	{
		_statusCode = _config->getRedirect().first;
		if ((_statusCode >= 301 && _statusCode <= 304) || _statusCode == 307 || _statusCode == 308)
		{
			_response->setHeader("Location", _config->getRedirect().second);
			return true;
		}
		else
		{
			if (_config->getRedirect().second.empty()){
				_content = _getPage(_statusCode);
			}
			else{
				_content = _config->getRedirect().second;
				return true;
			}
		}
	}
	return false;
}

bool	HttpHandler::_check_40x_error( HttpRequest const &request )
{
	if (_config->getClientMaxBodySize() && request.body().size() > _config->getClientMaxBodySize())
	{
		_content = _getPage(413);
		_statusCode = 413;
		return true;
	}
	else if (!_config->getMethod(request.method()))
	{
		_content = _getPage(405);
		_statusCode = 405;
		return true;
	}
	else if (access(_path.c_str(), F_OK) != 0)
	{
		_content = _getPage(404);
		_statusCode = 404;
		return true;
	}
	return false;
}

bool	HttpHandler::_isDirectory(const char *cpath){
	DIR *dir = opendir(cpath);
	if (dir == NULL){
		return false;
	}
	closedir(dir);
	return true;
}

void	HttpHandler::_server_msg(){
		
	if (_autoIndex && _isDirectory(_path.c_str()))
		_conf.printMsg(B, "Server[%s]: sending response [%s directory listing][%d]", _config->getServerName().c_str() , _path.c_str(),_statusCode);
	else if (_statusCode == 200)
		_conf.printMsg(B, "Server[%s]: sending response [%s][%d]", _config->getServerName().c_str(), _path.c_str(), _statusCode);
	else if (_config->getErrorPage(_statusCode) != "")
		_conf.printMsg(B, "Server[%s]: sending response [%s][%d]", _config->getServerName().c_str(), _path.c_str(), _statusCode);
	else
		_conf.printMsg(B, "Server[%s]: sending response [Default page][%d]", _config->getServerName().c_str(), _statusCode);
}

void	HttpHandler::_populateResponse(HttpRequest const &request)
{
	_response->setConfig(_config);
	_response->setContent(_content);
	_response->setHeader("Allow", _response->getAllowedMethods());
	_response->setHeader("Content-Length", std::to_string(_content.size()));
	_response->setHeader("Content-Type", _contentType);
	_response->setHeader("Date", _conf.getCurrTime());
	_response->setStatusCode(_statusCode);
	_response->setVersion(request.version());
}

std::string	HttpHandler::_getPage(short const & errorCode)
{
	std::string	file = _config->getErrorPage(errorCode);
	std::string path = _baseDir + _config->getRoot() + "/" + file;

	if (file.empty() || access(path.c_str(), F_OK) != 0 || _config->getRedirect().first != 0)
	{
		_getContentFromFile = false;
		return (_response->getDefaultContent(errorCode));
	}
	_getContentFromFile = true;
	_path = path;
	return (file);
}

void	HttpHandler::_openFile()
{
	std::ifstream	file(_path, std::ios::binary);

	if (file.is_open())
	{
		std::string	str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		_content = str;
		_getContentFromFile = false;
	}
	else
	{
		_content = _getPage(500);
		_statusCode = 500;
	}
	file.close();
}

/*	Target Parsing	********************************************************* */

std::string	HttpHandler::_createPath( void )
{
	std::string	path;
	std::string temp;

	path = _baseDir + _config->getRoot() + _htmlFile;
	if (!_config->getTarget().empty() && _config->getTarget() != "/")
	{
		size_t pos = path.find(_config->getTarget());
		if (pos != std::string::npos)
			path.erase(pos, _config->getTarget().length());
	}
	if (_isDirectory(path.c_str())){
		if (!_htmlFile.empty() && _htmlFile.back() == '/')
			temp = path + _config->getIndex();
		else
			temp = path + "/" + _config->getIndex();
		if (access(temp.c_str(), F_OK) == -1)
		{
			if (_config->getAutoIndex())
				_autoIndex = true;
		}
		else
			path = temp;
	}
	return path;
}

std::string	HttpHandler::_parseTarget(std::string const &target)
{
	size_t	queryPos = target.find('?');

	if (queryPos != std::string::npos)
	{
		return (target.substr(0, queryPos));
	}
	return (target);
}

/*	Methods		************************************************************* */
bool HttpHandler::checkCgi(HttpRequest const &request) const {
	CgiHandler cgi(request, getCGIbin());
	return cgi.isValid();
}


void	HttpHandler::_get_post(HttpRequest const &request)
{
	try
	{
		
		if ((_isDirectory(_path.c_str())))
		{
			if (_autoIndex)
				_content = _autoIndexGenerator(_path, request.target());
			else
			{
				_content = _getPage(404);
				_statusCode = 404;
			}
		}
		else if (checkCgi(request)) {
			return;
		}
		else if (!_isDirectory(_path.c_str()) && access(_path.c_str(), F_OK) == 0) {
			_getContentFromFile = true;
		}
	}
	catch (std::exception const &e)
	{
		std::cerr << "Something wrong with _get() function" << std::endl;
	}
}

void	HttpHandler::_delete( void )
{
	if (std::remove(_path.c_str()) == 0)
	{
		_content = _getPage(200);
		_statusCode = 200;
	}
	else
	{
		_content = _getPage(403);
		_statusCode = 403;
	}
}

/*	AutoIndex	************************************************************* */

std::string const HttpHandler::_autoIndexGenerator(std::string & path, std::string const & target){
	DIR *dir;
	struct dirent *dp;
	const char* cpath = path.c_str();
	if ((dir = opendir(cpath)) == NULL){
		cerr << "Cannot open directory" << endl;
		return "";
	}
	std::string cut_path = path.substr(7);
	if (cut_path.back() != '/')
		cut_path += '/';
	std::string content =\
	"<!DOCTYPE html>\n\
    <html lang=\"en\">\n\
    <head>\n\
            	<meta charset=\"utf-8\">\n\
				<title>Welcome to our webserv</title>\n\
				<link rel=\"stylesheet\" href=\"styles/styles.css\">\n\
    </head>\n\
    <body>\n\
	<div id=\"main-container\">\n\
		<nav id=\"menu\">\n\
			<ul>\n\
				<li><a href=\".\">Home</a></li>\n\
				<li><a href=\"#\">CGI</a>\n\
					<ul>\n\
						<li><a href=\"upload.html\">Upload</a></li>\n\
						<li><a href=\"time.html\">Time</a></li>\n\
					</ul>\n\
				</li>\n\
				<li><a href=\"about.html\">About</a></li>\n\
			</ul>\n\
		</nav>\n\
    <h1>Index of " + cut_path + "</h1>\n\
	<hr>\n\
    <p>\n";

	dp = readdir (dir);
	while((dp = readdir (dir)) != NULL){
		// cout << dp->d_name << endl;
		content += "\t\t<p><a href=\"http://" + _config->getHost() + ":" + to_string(_config->getPort()) + target;
		if (target.back() != '/')
			content += "/";
		content += std::string(dp->d_name) + "\">" + dp->d_name + "</a></p>\n";
	}
	content +="\
	<hr>\n\
	</p>\n\
			<footer>\n\
			<p>\n\
				<img src=\"images/triforce.png\" width=\"16\" height=\"13\">\n\
				&nbsp;&nbsp;All Hail Ganondorf&nbsp;&nbsp;\n\
				<img src=\"images/triforce.png\" width=\"16\" height=\"13\">\n\
			</p>\n\
		</footer>\n\
	</div>\n\
    </body>\n\
    </html>\n";
	closedir(dir);
	return content;
}
