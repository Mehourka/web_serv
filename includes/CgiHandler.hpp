#pragma once

#include "../includes/HttpRequest.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include "ConfigServer.hpp"


class CgiHandler
{
	enum State {
		COLD,
		SENDING_TO_SCRIPT,
		READING_FROM_SCRIPT,
		COMPLETE,
		TIMED_OUT
		,
		ERROR,
	};

private:
	HttpRequest	const	&_request;

	std::string			_htmlRoot;
	bool				_is_valid;
	State				_state;
	std::string			_scriptName;
	std::string			_scriptPath;
	std::string			_pathInfo;
	std::string			_queryString;
	std::string			_cgi_bin;
	int					_child_to_parent[2], _parent_to_child[2];
	pid_t				_process_id;

public:
	std::string			_cgiResponse;

private:
	std::vector<char const *>	_envp;
	std::vector<std::string>	_env_strings;

	void	_init();
	void	_send_to_cgi(std::string &src, size_t n_bytes);
	void	_read_from_cgi(std::string &dst, size_t n_bytes);
	bool	_timeout_cgi(int process_id, int &wstatus, int timeout_sec);
	void	_add_env_var(std::string key, std::string value);
	void	_setEnvp();
	bool	_spawn_process();
public:
	CgiHandler(HttpRequest const &request, std::string const &cgi_bin);

	bool	isValid() const; 
	bool	completed() const;
	void	run();
	std::string getContent() const { return _cgiResponse; };
};
