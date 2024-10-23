/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmehour <kmehour@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/09 15:53:01 by oroy              #+#    #+#             */
/*   Updated: 2024/10/23 17:17:23 by kmehour          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Socket.hpp"
#include <string.h>

Socket::Socket(std::string const host, unsigned int const port) : _host(host), _port(port), _socketFD(-1)
{
	memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_port = htons(_port);
	_address.sin_addr.s_addr = htonl(INADDR_ANY);
	// if (inet_pton(AF_INET, _host.c_str(), &_address.sin_addr) <= 0)
	// {
	// 	perror("inet_pton failed");
    // 	exit(1);
	// }
	return ;
}

Socket::Socket(int socket_fd)
	: _socketFD(socket_fd)
{
	bzero(&_address, sizeof(_address));
}

Socket::~Socket()
{
	return ;
}

Socket::Socket(Socket const &other)
{
	*this = other;
}

Socket& Socket::operator= (Socket const &other)
{
	_address = other._address;
	_host = other._host;
	_port = other._port;
	_socketFD = other._socketFD;
	_root = other._root;
	return *this;
}


int	Socket::getSocketFD(void) const
{
	return (_socketFD);
}

int	Socket::_errorMessage(std::string const msg) const
{
	std::cerr << msg << std::endl;
	if (_socketFD >= 0)
	{
		close (_socketFD);
	}
	return (-1);
}

void Socket::close_socket() {
	close(_socketFD);
}