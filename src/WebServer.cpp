/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmehour <kmehour@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 20:20:26 by olivierroy        #+#    #+#             */
/*   Updated: 2024/08/07 14:54:112 by kmehour          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/WebServer.hpp"
#include <cstddef>
#include <ios>
#include <type_traits>

WebServer::WebServer(std::vector<TcpListener> socketList, Config &conf) : _nfds(0), \
_listeners_list(socketList), _config(conf), _httpHandler(conf)
{
	memset(_fds, 0, sizeof(_fds));
	_clients_list.reserve(1024);
}

WebServer::~WebServer()
{

}

int	WebServer::init(void)
{
	for (size_t i = 0; i < _listeners_list.size(); i++)
	{
		if (_listeners_list[i].createListeningSocket() < 0)
		{
			cleanUpSockets();
			return (-1);
		}
		// Add the listening socket in our fd set
		_fds[i].fd = _listeners_list[i].getSocketFD();
		_fds[i].events = POLLIN;
		_nfds++;
	}
	return (0);
}

int	WebServer::run(void)
{
	signal(SIGINT, _sighandler);
	while (_serverOn)
	{
		if (poll(_fds, _nfds, 0) < 0)
		{
			// std::cerr << "poll() failed" << std::endl;
			continue;
		}
		_handleNewConnections();
		_processClients();
	}
	cleanUpSockets();
	return (0);
}

bool	WebServer::_isListeningSocket(int fd)
{
	for (size_t i = 0; i < _listeners_list.size(); ++i)
	{
		if (fd == _listeners_list[i].getSocketFD())
			return (true);
	}
	return (false);
}


void	WebServer::_acceptConnection(int fd)
{
	int	new_fd = 0;

	new_fd = accept(fd, NULL, NULL);
	if (new_fd < 0)
	{
		// If errno is EWOULDBLOCK, then all connections have been accepted
		if (errno != EWOULDBLOCK)
		{
			std::cerr << "accept() failed" << std::endl;
		}
		return ;
	}
	_fds[_nfds].fd = new_fd;
	_fds[_nfds].events = POLLIN;
	WebClient new_client(new_fd, &_httpHandler, &_fds[_nfds]);
	_clients_list.push_back(new_client);
	_nfds++;
}

void	WebServer::_handleNewConnections()
{
	for (int i = 0; i < _nfds; i++)
	{
		if ((_fds[i].revents & POLLIN) && _isListeningSocket(_fds[i].fd) )
		{
			_acceptConnection(_fds[i].fd);
		}
	}
}

void	WebServer::_processClients()
{
	std::vector<WebClient>::iterator client_it;

	for (client_it = _clients_list.begin(); client_it < _clients_list.end(); ++client_it)
	{
		if (client_it->isComplete() 
			|| client_it->getTime() > CLIENT_TIMEOUT)
		{
			_removeClient(&(*client_it));
			return ;
		}
		client_it->process();
	}
}

void	WebServer::_compressFdsArray(void)
{
	for (int i = 0; i < _nfds; i++)
	{
		if (_fds[i].fd == -1)
		{
			for(int j = i; j < _nfds - 1; j++)
			{
				// Move WebClient pollFd
				WebClient *client = _getClient(_fds[j + 1].fd);
				if (client)
					client->setPollFd(_fds + j);
				_fds[j] = _fds[j + 1];
			}
			memset(&_fds[_nfds - 1], 0, sizeof (struct pollfd));
			i--;
			_nfds--;
		}
	}
	// printFdsArray();
}

void	WebServer::printFdsArray(void) const
{
	for (int i = 0; i < _nfds; ++i)
	{
		std::cout << "====== FD idx " << i << " ======" << std::endl;
		std::cout << "FD = " << _fds[i].fd << std::endl;
	}
}

void	WebServer::cleanUpSockets(void) const
{
	for (int i = 0; i < _nfds; i++)
	{
		if(_fds[i].fd >= 0)
			close(_fds[i].fd);
	}
}

void	WebServer::_sighandler(int signum)
{
	(void) signum;
	_serverOn = false;
}

void	WebServer::_removeClient(WebClient *client_ptr) {
	std::vector<WebClient>::iterator it;

	if (!client_ptr) {
		std::cerr << "[DEBUG] Unexpected error, client_ptr is null";
		return ;
	}

	// Close socket, and set pollfd.fd=-1
	client_ptr->close();
	_compressFdsArray();

	// Remove clinet from _clients_list
	for (it = _clients_list.begin(); it != _clients_list.end(); ++it){
		if(&(*it) == client_ptr){
			_clients_list.erase(it);
			return ;
		}
	}
	std::cerr << "[DEBUG] Client not found in the list" << std::endl;
}

bool	WebServer::_serverOn = true;

WebClient*	WebServer::_getClient(int socketFd) {
	std::vector<WebClient>::iterator it;

	for (it = _clients_list.begin(); it != _clients_list.end(); ++it)
	{
		if (it->getSocketFD() == socketFd)
			return &(*it);
	}
	return (NULL);
}