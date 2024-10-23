#pragma once

#include <ctime>
#include <sys/poll.h>
#include <vector>
#include "Socket.hpp"
#include "HttpRequest.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/HttpHandler.hpp"
#include <poll.h>

#define BUFFER_SIZE 51200

class HttpHandler;

class WebClient: public Socket {
	enum State {
		READING,
		HANDLING_REQUEST,
		HANDLING_CGI,
		SENDING_RESPONSE,
		COMPLETE,
		ERROR
	};

public:
	WebClient(int accepted_connection, HttpHandler *httpHandler, struct pollfd *pollFd);
	WebClient(WebClient const &other);
	WebClient& operator=(WebClient const &other);
	~WebClient();

	HttpRequest getRequest() { return _request; };
	void	_sendData(char const *data, size_t data_len);
	void	_handleRequest();
	int		getTime() const;
	bool	process();
	void	close();
	bool	isComplete() const { return _state == COMPLETE; };
	void	setPollFd(struct pollfd *poll_ptr);

	void	_updateTime();
private:
	struct pollfd		*_pollFd;
	State				_state;
	HttpRequest			_request;
	HttpResponse		_response;
	HttpHandler			*_httpHandler;
	CgiHandler			*_cgi;
	std::vector<char>	_writeBuffer;
	std::time_t			_last_update;
	void				_deleteCGI();


	void	_processInput();
	void	_processCGI();

	WebClient();
};