#include "../includes/HttpRequest.hpp"

#include <iostream>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>

#include <iomanip>
// ========== ========== Constructor ========== ==========
HttpRequest::HttpRequest()
	: _state(READING_REQUEST_LINE), _contentLength(0) {
}

// ========== ========== Vector<char> manipulatoins ========== ==========

bool _extract_http_line(std::vector<char> &buffer, std::vector<char> &line)
{
	std::vector<char>::const_iterator pos;
	std::string delimiter = "\r\n";

	pos = std::search(buffer.begin(), buffer.end(), delimiter.begin(), delimiter.end());

	if (pos == buffer.end())
		return false;
	insert_vector_data(line, buffer.data(), (pos - buffer.begin()));
	buffer.erase(buffer.begin(), pos + delimiter.size());
	return true;
}

// ========== ========== Validation ========== ==========
bool HttpRequest::parse(char *buffer, int read_bytes)
{
	if (!buffer)
	{
		_state = ERROR;
		return false;
	}
	std::vector<char> line;
	_buffer.insert(_buffer.end(), buffer, buffer + read_bytes);



	while (_state != READING_BODY && _extract_http_line(_buffer,line) )
	{
		line.push_back('\0');
		// std::cout << "\n[LINE] " << line.data() << std::endl;
		switch (_state) {
			case READING_REQUEST_LINE:
				// std::cout << "[DEBUG] Parsing request line ..." << std::endl;
				_parse_request_line(line.data());
				break;
			case READING_HEADERS:
				// std::cout << "[DEBUG] Parsing headers ..." << std::endl;
				_parse_headers(line.data());
				break;
			case READING_BODY:
				break;
			case COMPLETE:
				return true;
				break;
			case ERROR:
				std::cout << "HttpRequest [ERROR], aborting." << std::endl;
				return true;
				break;
		}
		line.clear();
	}

	if (_state == READING_BODY)
	{
		// std::cout << "[DEBUG] Parsing body ..." << std::endl;
		_parse_body();
	}
	return false;
}

// ========== ========== Getters ========== ==========
std::string const HttpRequest::getHeader(std::string key) const
{
	toLowerCase(key);
	if (_headers.find(key) != _headers.end())
		return _headers.at(key);
	return "";
}

int	HttpRequest::getContentLength() const
{

	std::string _contentLength;
	_contentLength = getHeader("content-length");
	if (!is_number(_contentLength))
		return 0;

	return std::stoi(_contentLength);

}


void HttpRequest::_add_header(std::string key, std::string value)
{
	value = trim(value);
	toLowerCase(key);

	_headers[key] = value;
}


void HttpRequest::_parse_request_line(char *start_line)
{
	try {
		std::istringstream iss(start_line);
		_validate_request_line(start_line);
		iss >> _method >> _target >> _version;
		_state = READING_HEADERS;
	}
	catch (std::exception const &e)
	{
		_state = ERROR;
	}
}

void HttpRequest::_validate_request_line(std::string const &request_line) const {

	std::regex re_request_line(
		"^(GET|POST|DELETE|HEAD|PUT)"
		" \\/\\S* "
		"HTTP\\/1.[1,3]$");

	if (!std::regex_match(request_line, re_request_line))
	{
		throw std::exception();
	}
}

bool HttpRequest::_expectBody()
{
	std::string str;

	if ((str = getHeader("content-length")).empty())
		return false;
	try {
		_contentLength = getContentLength();
		return  _contentLength > 0 && _method == "POST";
	} catch( std::exception &e ) {
		return false;
	}
}

void HttpRequest::_parse_headers(char *line)
{
	std::string header_line = line;

	// End of headers
	if (header_line.empty()){
		_state = _expectBody() ? READING_BODY : COMPLETE;
		return ;
	}

	// Stop on invalid headers
	if (!_valid_header(header_line)) {
		_state = ERROR;
		std::cout << "[DEBUG] Invalid Header" << std::endl;
		return ;
	}

	// Add header to map
	int const pos = header_line.find(":");
	std::string key = header_line.substr(0, pos);
	std::string value = header_line.substr(pos + 1, header_line.size());
	_add_header(key, value);

}


bool HttpRequest::_valid_header(std::string const &line) const
{
	std::regex const re("^\\S+:[ ]?.*[ ]?$");

	return std::regex_match(line, re);
}



void HttpRequest::_parse_body()
{
	int read_bytes = 0;

	if (_contentLength <= 0) {
		_state = ERROR;
		return;
	}

    if (_body.capacity() < static_cast<size_t>(_contentLength)) {
        _body.reserve(_contentLength);
    }
	read_bytes = ::min(_buffer.size(), _contentLength - _body.size());
	_body.insert(_body.end(), _buffer.begin(), _buffer.begin() + read_bytes);
	_buffer.clear();

	if (static_cast<int>(_body.size()) >= _contentLength) {
		_state = COMPLETE;
		return;
	}
}

void HttpRequest::print_request() const
{
	char const BLUE[] = "\033[34m";
	char const RESET[] = "\033[00m";
	char const RED[] = "\033[31m";

	if (hasError()) {
		std::cerr << RED << "\nInvalid Request."  << RESET << std::endl;
		return;
	}
	std::cout << BLUE << "\nHttpRequest:\n" << std::setw(10);
	std::cout << "  Method: " << _method << std::endl;
	std::cout << "  Target: " << _target << std::endl;
	std::cout << "  Version: " << _version << std::endl;
	std::cout << "  Headers:" << std::endl;
	print_headers();
	std::cout << std::boolalpha << "parsed contentLenght:  " << _contentLength  << std::endl;
	std::cout << std::boolalpha << "Body size: " << _body.size()  << std::endl;
	if (!isComplete()) {
		std::cerr << RED << "\n[DEBUG] Incomplete Request." << std::endl;
		return;
	}
	std::cout << RESET;
}

void HttpRequest::print_headers() const
{
	std::map<std::string, std::string>::const_iterator cit;

	for(cit = _headers.begin(); cit != _headers.end(); ++cit)
		std::cout << "  - " <<  cit->first << ": " << cit->second << std::endl;
}
