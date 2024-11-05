#include "../includes/WebClient.hpp"
#include "../includes/utils.hpp"
#include <istream>
#include <sstream>
#include <sys/socket.h>

WebClient::WebClient(int accepted_connection, HttpHandler* httpHandler, pollfd *pollFd_ptr)
	: Socket(accepted_connection), _pollFd(pollFd_ptr),
	_state(READING), _httpHandler(httpHandler)

{
	_cgi = NULL;
	_sentBytes = 0;
	setPollFd(pollFd_ptr);
	_updateTime();
}

WebClient::WebClient(WebClient const &other)
	: Socket(other)
{
	*this = other;
}

WebClient::~WebClient() {
}

void	WebClient::_deleteCGI()
{
	if (_cgi)
	{
		delete _cgi;
		_cgi = NULL;
	}
}

WebClient& WebClient::operator = (WebClient const &other) {
	Socket::operator =(other);
	_pollFd = other._pollFd;
	_state = other._state;
	_request = other._request;
	_response = other._response;
	_httpHandler = other._httpHandler;
	_cgi = other._cgi;
	_writeBuffer = other._writeBuffer;
	_last_update = other._last_update;
	_sentBytes = other._sentBytes;

	return *this;
}

void WebClient::_sendData(char const *data, size_t data_len) {
	size_t	rtn = 0;
	size_t chunk_size = 0;
	size_t max_chunk_size = 65536;


	if (_sentBytes >= data_len) {
		_state = COMPLETE;
		return;
	}

	chunk_size = ::min(max_chunk_size, data_len - _sentBytes);
	rtn = send(_socketFD, data + _sentBytes, chunk_size, 0);
	if (rtn <= 0) {
		return;
	}
	_sentBytes += rtn;
}


void WebClient::_processInput()
{
	int bytes_read = 0;
	char buffer[BUFFER_SIZE];

	if (_pollFd->revents & POLLIN)
	{
		bytes_read = recv(_socketFD, buffer, BUFFER_SIZE, 0);
		if (bytes_read == 0) {
			_state = HANDLING_REQUEST;
			return ;
		}
		if (bytes_read < 0)
			return;
	}

	// Parse the request
	_request.parse(buffer, bytes_read);
	if (_request.hasError() || _request.isComplete()) {
		_state = HANDLING_REQUEST;
	}
}

void WebClient::_processCGI() {

	// Init cgi execution
	if (_cgi == NULL)
	{
		_cgi = new CgiHandler(_request, _httpHandler->getCGIbin());
	}
	_cgi->run();

	if (_cgi->completed())
	{
		_response.setContent(_cgi->getContent());
		_deleteCGI();
		_state = SENDING_RESPONSE;
	}
}

void WebClient::_handleRequest(){
	_httpHandler->buildResponse(_request, _response);
	if(_httpHandler->checkCgi(_request))
		_state = HANDLING_CGI;
	else
		_state=SENDING_RESPONSE;
}

std::string WebClient::_printStatus() const{
	std::ostringstream oss;
	std::string state;

	switch (_state) {
		case READING:
			state = "reading request";
			break;
		case HANDLING_REQUEST:
			state = "Handling request";
			break;
		case HANDLING_CGI:
			state = "Handling CGI";
			break;
		case SENDING_RESPONSE:
			state = "Writing to client";
			break;
		case COMPLETE:
			state = "Completed!!!";
			break;
		default:
			state= "[DEFAULT]";
			break;
	}

	oss << "[WebClient] target: " << _request.target()
		<< "\nStatus: " << state
		<< std::endl;
	return oss.str();
}

bool WebClient::process()
{
	switch (_state) {
		case READING:
			_processInput();
			break;
		case HANDLING_REQUEST:
			_handleRequest();
			return false;
		case HANDLING_CGI:
			_processCGI();
			return false;
		case SENDING_RESPONSE:
			_sendData(_response.getResponse().data(), _response.getResponse().size());
			break;
		case COMPLETE:
			std::cout << "[UNIMPLEMENTED] Web Client COMPLETE" << std::endl;
			break;
		case ERROR:
			std::cout << "[UNIMPLEMENTED] Web Clnent ERROR" << std::endl;
			return false;
	}

	printMsg(G, "%s", _printStatus().c_str());

	return !(_state == COMPLETE);
}

int		WebClient::getTime() const
{
	return seconds_since(_last_update);
}

void	WebClient::_updateTime()
{
	_last_update = std::time(NULL);
}

void WebClient::close()
{
	close_socket();
	_pollFd->fd = -1;
}

void	WebClient::setPollFd(struct pollfd *poll_ptr) {
	_pollFd = poll_ptr;
}