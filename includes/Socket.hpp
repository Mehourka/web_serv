/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmehour <kmehour@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/08 18:56:37 by oroy              #+#    #+#             */
/*   Updated: 2024/10/23 16:23:03 by kmehour          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <unistd.h>
# include <iostream>
# include <string>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <fcntl.h>

class Socket
{

protected:
	Socket() {};
	Socket(int socket_fd);
	Socket(std::string const host, unsigned int const port);
	Socket(Socket const &);
	Socket& operator= (Socket const &);
	~Socket();

	struct sockaddr_in	_address;
	std::string			_host;
	unsigned int		_port;
	int					_socketFD;
	std::string			_root;
	void				close_socket();


public:
	int					getSocketFD(void) const;
	int					_errorMessage(std::string const msg) const;
};

#endif
