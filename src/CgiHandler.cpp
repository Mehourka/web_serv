#include "../includes/CgiHandler.hpp"
#include "../includes/utils.hpp"
#include <strings.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#define TIMEOUT 300

CgiHandler::CgiHandler(HttpRequest const &request, std::string const &cgi_bin)
	: _request(request), _htmlRoot("./data/www"), _cgi_bin(cgi_bin)
{
	if (request.hasError())
		throw std::exception();

	_env_strings.reserve(32);
	_envp.reserve(32);
	_is_valid = false;
	_state = COLD;
	_sent_bytes = 0;
	_request_body = _request.body();

	_init();
}

void CgiHandler::_init() {

	const std::string target = _request.target();

	if (_cgi_bin.back() != '/')
		_cgi_bin += "/";

	size_t	it = target.find(_cgi_bin);

	if (it == 0) {
		it = target.find_first_of("/?", _cgi_bin.size());
		_scriptName = target.substr(0, it);
	}
	else if (target.find(".bla") != std::string::npos)
		_scriptName = "/cgi-bin/cgi_tester";
	else {
		return;
	}

	_scriptPath = _htmlRoot + _scriptName;

	if (access(_scriptPath.data(), X_OK) != 0) {
		return ;
	}

	if (target[it] == '/') {
		size_t	it_query = target.find_first_of('?', it);
		_pathInfo = target.substr(it, it_query - it);
		it = it_query;
	}

	if (target[it] == '?') {
		_queryString = target.substr(it + 1);
	}

	// printMsg(B, "CGI: %s _scriptName", _scriptName.c_str());
	// printMsg(B, "CGI: %s _pathInfo", _pathInfo.c_str());
	// printMsg(B, "CGI: %s _queryString", _queryString.c_str());

	this->_is_valid = true;
	_setEnvp();
}

void CgiHandler::_setEnvp() {
	_add_env_var("SCRIPT_NAME", _scriptName);
	_add_env_var("PATH_INFO", _pathInfo);
	_add_env_var("QUERY_STRING", _queryString);
	_add_env_var("ROOT", _htmlRoot);
	_add_env_var("HTTP_VERSION", _request.version());
	_add_env_var("REQUEST_METHOD", _request.method());
	_add_env_var("FILENAME", "/data/www/upload/test.txt");
	_add_env_var("UPLOAD_DIR", "./data/www/upload2");
	_add_env_var("CONTENT_TYPE", _request.getHeader("content-type"));
	_add_env_var("SERVER_PROTOCOL", "HTTP/1.1");
	_envp.push_back(NULL);

}
void CgiHandler::_add_env_var(std::string key, std::string value) {
	_env_strings.push_back(key + "=" + value);
	_envp.push_back(_env_strings.back().c_str());
}

bool CgiHandler::isValid() const
{
	return _is_valid;
}

bool CgiHandler::completed() const {
	return (_state == COMPLETE || _state == TIMED_OUT);
}

bool CgiHandler::_spawn_process() {
	std::vector<char const *>	argv;

	std::stringstream			content_length;
	content_length << "CONTENT_LENGTH=" << _request.body().size();
	_exec_start = std::time(NULL);


	pipe(_child_to_parent);
	pipe(_parent_to_child);

	// Set pipe to non-blocking
	fcntl(_child_to_parent[0], F_SETFL,  O_NONBLOCK);
	fcntl(_child_to_parent[1], F_SETFL,  O_NONBLOCK);
	fcntl(_parent_to_child[0], F_SETFL,  O_NONBLOCK);
	fcntl(_parent_to_child[1], F_SETFL, O_NONBLOCK);

	_process_id = fork();
	if (_process_id < 0)
		perror ("fork() failed");
	else if (_process_id == 0)
	{
		// _webServer.cleanUpSockets();
		close(_parent_to_child[1]);
		dup2(_parent_to_child[0], STDIN_FILENO);
		close(_parent_to_child[0]);

		close(_child_to_parent[0]);
		dup2(_child_to_parent[1], STDOUT_FILENO);
		close(_child_to_parent[1]);

		argv.push_back(_scriptPath.data());
		argv.push_back(NULL);

		execve (_scriptPath.data(), const_cast<char * const *>(argv.data()), const_cast<char * const *>(_envp.data()));
		exit (127);
	}

	close(_parent_to_child[0]);
	close(_child_to_parent[1]);
	return true;
}

void	CgiHandler::run()
{
	int							wstatus;

	if (_state == COLD) {
		_spawn_process();
		if (_request.getContentLength() > 0) {
			_state = SENDING_TO_SCRIPT;
		} else {
			_state = READING_FROM_SCRIPT;
		}
	}

	// Check time-out
	if (_timeout_cgi(_process_id, wstatus, TIMEOUT))
	{
		kill(_process_id, 9);
		_cgiResponse = "<h1>[CGI] Script timed out!</h1>";
		_state = TIMED_OUT;
		return;
	}

	if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) != 0)
	{
		// DEBUG
		printMsg(B, "CGI: [CGI] Process exited with error code");
		std::cout << std::boolalpha << B << "Exited: " << WIFEXITED(wstatus)\
			<< "\nExit status: " << WEXITSTATUS(wstatus)\
			<< "\nSignaled: " << WIFSIGNALED(wstatus)\
			<< END << std::endl;

		_cgiResponse = "<h1>[DEBUG] Error in executing CGI script</h1>\r\n";
		_state = COMPLETE;
		return;
	}

	if (_state == SENDING_TO_SCRIPT)
	{
		// write chunk
		unsigned long chunk_size = 65536;
		if(_sent_bytes >= _request.getContentLength())
		{
			close(_parent_to_child[1]);
			_state = READING_FROM_SCRIPT;
		}
		else {
			chunk_size = ::min(chunk_size, _request_body.size() - _sent_bytes);
			write(_parent_to_child[1], _request_body.data() + _sent_bytes, chunk_size);
			_sent_bytes += chunk_size;
		}
		printMsg(G, "[CGI] %d  sent to script", chunk_size);
		return;
	}

	if (_state == READING_FROM_SCRIPT)
	{
		// Read chunk
		#define BUFFERSIZE 65536
		char buffer[BUFFERSIZE];
		bzero(buffer, BUFFERSIZE);
		int bytes_read;

		bytes_read = read (_child_to_parent[0], buffer, BUFFERSIZE);
		// printMsg(G, "CGI: bytes_read(%i) buffer: %s", bytes_read, buffer);
		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
			_cgiResponse += buffer;
		}

		if (bytes_read == 0 && _cgiResponse.size() > 0)
			_state = COMPLETE;
	}
	// If the handler is marked as complete, web client reads the _cgiResponse, and sends it
	// TODO [Optional]: make the webclient send data as it comes, with chunked transfer.
}

bool CgiHandler::_timeout_cgi(int _process_id, int &wstatus, int timeout_sec)
{
	// printMsg(B, "CGI: [Timing for ] %d", timeout_sec);
	// printMsg(B, "CGI: Elapsed time: %d", seconds_since(_exec_start));
	waitpid(_process_id, &wstatus, WNOHANG);
	if (seconds_since(_exec_start) > timeout_sec) {
		printMsg(B, "CGI: Cgi-handler [TIMEOUT]");
		return true;
	}
	return false;
}
