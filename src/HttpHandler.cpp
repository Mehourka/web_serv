/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmehour <kmehour@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 12:30:55 by oroy              #+#    #+#             */
/*   Updated: 2024/07/13 15:43:42y kmehour          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpHandler.hpp"
#include "../includes/HttpRequest.hpp"

HttpHandler::HttpHandler(void)
{

}

HttpHandler::~HttpHandler()
{

}

/*	Field List	************************************************************* */

std::string const	HttpHandler::_fieldList[FIELD_COUNT] = \
{
	"Host"
};

/*	Functions	************************************************************* */

std::string const	HttpHandler::handleRequest(std::string request)
{
	_request = request;

	HttpRequest req(_request);

	_method = req.method();
	_target = req.target();
	_version = req.version();

	if (_target == "/cgi-bin/upload.py")
		_execCgiScript();

	_buildResponse();
	return (_response);
}

/**
 * Don't forget to fix issue with not found header field in array
*/
size_t	HttpHandler::_parseHeaderFields(size_t i, size_t f)
{
	// std::string	key;
	// std::string	value;
	// size_t 		count = 0;

	// while (count < FIELD_COUNT)
	// {
	// 	i = _request.find('\n', i) + 1;
	// 	f = _request.find(':', i);
	// 	key = _request.substr(i, f - i);
	// 	if (_findField(key) >= 0)
	// 	{
	// 		i = f + 1;
	// 		i = _request.find_first_not_of(' ', i);
	// 		f = _request.find('\n', i);
	// 		value = _request.substr(i, f - i);
	// 		_fields.insert(std::pair<std::string, std::string>(key, value));
	// 		count++;
	// 	}
	// }
	while (i + 1 != f)	// Skip all other header fields
	{
		i = _request.find('\n', i) + 1;
		f = _request.find('\n', i);
	}
	return (f + 1);
}

size_t	HttpHandler::_findField(std::string const key) const
{
	for (size_t i = 0; i < FIELD_COUNT; ++i)
	{
		if (key == _fieldList[i])
			return (i);
	}
	return (-1);
}

void	HttpHandler::_buildResponse()
{
	std::ifstream		f("./data/www" + _target, std::ios::binary);
	std::ostringstream	oss;
	std::string			content = _request;
	int					errorCode = 404;

	if (f.good())
	{
		std::string	str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		content = str;
		errorCode = 200;
	}
	f.close();

	oss << "HTTP/1.1 " << errorCode << " OK\r\n";
	oss << "Cache-Control: no-cache, private\r\n";
	// oss << "Content-Type: " << contentType << "\r\n";
	oss << "Content-Length: " << content.size() << "\r\n";
	oss << "\r\n";
	oss << content;

	_response = oss.str();
}

void	HttpHandler::_execCgiScript(void) const
{
	std::vector<char const *>	argv;
	std::vector<char const *>	envp;
	const char					*python_path = "/usr/bin/python";
	pid_t						process_id;

	argv.push_back(python_path);
	argv.push_back("/Users/oroy/Documents/cursus42/webserv/cgi-bin/upload.py");
	argv.push_back(NULL);

	process_id = fork();
	if (process_id < 0)
		perror ("fork() failed");
	else if (process_id == 0)
	{
		execve (python_path, const_cast<char * const *>(argv.data()), NULL);
		perror ("execve() failed");
		exit (EXIT_FAILURE);
	}
}

/*	Utils	***************************************************************** */

char const	*HttpHandler::_getScriptName(void) const
{
	size_t	i;
	
	i = _target.find('/');
	i = _target.find('/', i);

	return (&_target[i + 1]);
}

void	HttpHandler::_printHeaderFields(void) const
{
	for (std::map<std::string, std::string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
	{
		std::cout << it->first << ": " << it->second << "\n";
	}
}
