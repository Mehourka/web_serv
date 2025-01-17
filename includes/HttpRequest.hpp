#pragma once

#include <sstream>
#include <string>
#include <map>
#include "utils.hpp"
#include <algorithm>

class HttpRequest
{
public:
	enum State {
		READING_REQUEST_LINE,
		READING_HEADERS,
		READING_BODY,
		COMPLETE,
		ERROR
	};

	HttpRequest();

	// Getters
	std::vector<char> const	raw()	const		{ return _buffer; }
	std::string const		method() const		{ return _method; };
	std::string const		target() const		{ return _target; };
	std::string const		version() const		{ return _version; };
	std::vector<char> const	body() const		{ return _body; };
	bool					hasError() const	{ return _state == ERROR; };
	bool					isComplete() const	{ return _state == COMPLETE; };
	std::string const		getHeader(std::string const key) const;
	int						getContentLength() const;

	bool					parse(char *data, int bytes_read);
	void 					reset();

private:
	State								_state;
	std::string							_method;
	std::string							_target;
	std::string							_version;
	std::vector<char>					_buffer;
	std::vector<char>					_body;
	std::map<std::string, std::string>	_headers;
	int									_contentLength;


	void parse();
	void _parse_request_line(char *line);
	void _validate_request_line(std::string const &request_line) const;

	void _parse_headers(char *line);
	void _add_header(std::string const key, std::string const value);
	bool _valid_header(std::string const &line) const;

	void _parse_body();
	bool _expectBody();
	void _extract_line(std::vector<char> &read_buffer, std::vector<char> &dest);


// Debug
public:
	void print_request() const;
	void print_headers() const;

};
