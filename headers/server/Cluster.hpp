 /* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lde-sous <lde-sous@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 14:36:38 by lde-sous          #+#    #+#             */
/*   Updated: 2024/02/05 14:36:38 by lde-sous         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLUSTER_HPP
# define CLUSTER_HPP

# pragma once
# include "../webserv.hpp"
# include "../structures.hpp"
# include "../config/Configuration.hpp"
# include "Server.hpp"
# include "../requests/Request.hpp"

class Server;
class Configuration;

class	Cluster
{
	private:
		Cluster();
		size_t					_numberOfSv;
		std::vector<Server*>	 _servers;
		std::vector<pollfd> 	_pollFDs;
		std::map<int, Server*> 	_fdMap;
		std::map<int, time_t> 	_activityTime;
		std::vector<int>		_serverSockets;
		Configuration			_config;

	public:
		explicit Cluster(const std::string& filepath);
		Cluster(const Cluster& original);
		Cluster& operator=(const Cluster& original);
		~Cluster();
		void	setPollFD();
		Server* getServer();
		Server* getServer(int fd);
		std::vector<Server*>& getServers();
		void	initServer();
		void	configureServer(Server* server);
		void	createServer(t_listen& listenStruct);
		void	setupServer();
		void	promptInfo();
		void	clearServer();
		int	 	checkTimeout(int epoll_fd, struct epoll_event* event_buffer);
		void	callerSemicolon(Server* server);
		void	callerDoubles(Server *server);
		void	callerServerName(Server* server);
		void	callerListen(std::string& wordStack, t_listen& listen);
		void	callerServerRoot(Server* server);
		void	callerErrorPage(Server* server);
		void	callerIndex(Server* server);
		void	callerMethods(Server* server);
		void	callerClientSize(Server* server);
		void	callerLocations(Server* server);
		void	startServers();
		void	connectionHandler(int fd, Server* server);

		class ClusterException : public std::exception
		{
			private:
				std::string _errMessage;
			public:
				ClusterException(const std::string& error);
				~ClusterException() throw();
				virtual const char* what() const throw();
		};
};

#endif
