/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lde-sous <lde-sous@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 15:45:59 by lde-sous          #+#    #+#             */
/*   Updated: 2024/02/05 15:45:59 by lde-sous         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../../headers/server/Cluster.hpp"

extern volatile sig_atomic_t gSignalStatus;
extern bool chunky;

/* ===================== Orthodox Canonical Form ===================== */

Cluster::Cluster() : _numberOfSv() {}

Cluster::Cluster(const Cluster& original) : _numberOfSv(),
	 _servers(original._servers) /*, _config(original._config)*/ {

}

Cluster& Cluster::operator=(const Cluster& original) {
	if (this != &original) {
		_servers = original._servers;
	}
	return *this;
}

Cluster::~Cluster() {
	std::vector<pollfd>::iterator	it;
	for (it = _pollFDs.end() - 1; it != _pollFDs.begin() - 1; --it) {
		close(it->fd);
		_pollFDs.erase(it);
	}
	for (size_t i = 0; i < _numberOfSv; i++)
		if (_servers[i])                                                                                                                                                                                                                            
			delete _servers[i];
	_servers.clear();
	_serverSockets.clear();
}

/* ===================== Constructors ===================== */

/**
 * @brief Constructs a Cluster object and sets up and starts the servers.
 *
 * @param filepath The path to the server configuration file.
 */
Cluster::Cluster(const std::string& filepath) : _numberOfSv(0), _config(filepath) {
	initServer();
}

/* ===================== Setter functions ===================== */

/**
 * @brief Sets the poll file descriptors for all servers.
 */
void	Cluster::setPollFD() {
	_pollFDs.reserve(_servers.size());
	std::vector<Server*>::iterator it;
	for (it = _servers.begin(); it != _servers.end(); it++) {
		if (it != _servers.end() && (*it)->getServerStatus()) {
			pollfd pfd;
			pfd.fd = (*it)->getFD();
			pfd.events = POLLIN; //read events
			pfd.revents = 0;
			_pollFDs.push_back(pfd);
			_fdToServerMap[(*it)->getFD()] = *it;
		}
	}
}

/* ===================== Getter functions ===================== */

std::vector<Server*>& Cluster::getServers() {
	return (_servers);
}

Server* Cluster::getServer() {
	return (_servers.back());
}

Server* Cluster::getServer(int fd) {
	std::vector<Server*>::iterator it;
	for (it = _servers.begin(); it != _servers.end(); ++it)
		if (fd == (*it)->getFD())
			return (*it);
	return NULL;
}

/* ===================== Webserv Setup Functions ===================== */

/**
 * @brief Sets up and configures multiple servers based on parsed server blocks.
 * 
 * This function iterates over the parsed server blocks from the configuration and sets up each server accordingly.
 * It displays a loading animation while each server is being configured.
 * For each server, it creates a new server instance, parses its configuration, and displays the status of the initServer process.
 * If any errors occur during server initServer, it catches exceptions, displays an error message, and clears the server.
 * After setting up all servers, it establishes server sockets, configures poll file descriptors, and displays server information.
 */
void	Cluster::initServer() {
	t_listen listenStruct;
	int i = 0;
	std::cout << BOLD << GREEN << "─────────────────────────[Parsing " << _config.getSvBlocks().size() << " server(s)]──────────────────────────" << RESET << std::endl;
	while (!_config.getSvBlocks().empty()) {
		try{
			std::cout << "Server " << i + 1 << " \u2192 ";
			std::cout.flush();
			const char* loadingIcons[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
			const int numIcons = sizeof(loadingIcons) / sizeof(loadingIcons[0]);
			for (int j = 0; j < 10; ++j) {
				std::cout << "\r" << BOLD << YELLOW << "Server " << i + 1 << " \u2192 " << loadingIcons[j % numIcons] << RESET;
				std::cout.flush();
				usleep(50000); // Sleep for 100 milliseconds (adjust as needed)
			}
			invertStack(_config.getSvBlocks());
			createServer(listenStruct);
			configureServer(_servers.back());
			std::cout << "\r" << BOLD << GREEN << "Server " << i + 1 << ": " << CYAN << listenStruct.port << GREEN <<" \u2192 \u2713" << RESET << std::endl; // Unicode check mark symbol
			i++;
		} catch (std::exception &e) {
			std::cout << "\r" << BOLD << RED << "Server " << i + 1 << ": " << listenStruct.port << " \u2192 \u2717 : " << RESET; // Unicode cross symbol
			std::cout << e.what() << std::endl;
			clearServer();
		}
	}
	setupServer();
	setPollFD();
	promptInfo();
}

/**
 * @brief Parses and configures settings for a single server instance.
 * 
 * This function is responsible for parsing and configuring various settings for a single server instance.
 * It ensures that semicolons are present at the end of each configuration directive, performs double-checking for configuration consistency,
 * fetches server name, root directory, index files, supported HTTP methods, error pages, client request size limit, and location blocks.
 * After parsing and configuring all settings, it increments the server count and removes the parsed server block from the configuration.
 * 
 * @param server A pointer to the server instance to be parsed and configured.
 */
void	Cluster::configureServer(Server* server) {
	callerSemicolon(server);
	callerDoubles(server);
	callerServerName(server);
	callerServerRoot(server);
	callerIndex(server);
	callerMethods(server);
	callerErrorPage(server);
	callerClientSize(server);
	callerLocations(server);
	_config.getSvBlocks().pop();
	_numberOfSv++;
}

/**
 * @brief Creates a new server instance and initializes its configuration.
 * 
 * This function extracts the listen directive from the top server block in the configuration,
 * creates a new server instance with the specified listen configuration, and initializes its body
 * by parsing the content of the server block. The parsed server instance is then added to the
 * list of servers managed by the server cluster.
 * 
 * @param listenStruct A reference to the structure containing the listen configuration for the new server.
 */
void	Cluster::createServer(t_listen& listenStruct) {
	callerListen(_config.getSvBlocks().top(), listenStruct);
	_servers.push_back(new Server(listenStruct));
	std::istringstream iss(_config.getSvBlocks().top());
	_servers.back()->bodyFiller(iss);
}

/**
 * @brief Initializes server sockets for all servers in the cluster.
 * 
 * This function iterates over the servers in the cluster, initializes each server's socket, and starts the server.
 * While initializing, it displays a loading icon to indicate the initialization process. Upon successful initialization
 * of a server, a check mark symbol is displayed. If an error occurs during initialization, a cross symbol is displayed
 * along with the error message, and the server is removed from the cluster.
 */
void Cluster::setupServer() {
    invertVector(_servers);
    std::cout << std::endl << BOLD << GREEN << "───────────────────────[Initializing " << _servers.size() << " server(s)]───────────────────────" << RESET << std::endl;

    for (std::vector<Server*>::iterator it = _servers.begin(); it != _servers.end();) {
        try {
            const char* loadingIcons[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
            const int numIcons = sizeof(loadingIcons) / sizeof(loadingIcons[0]);
            for (int j = 0; j < 10; ++j) {
                if (*it) {
                    std::cout << "\r" << BOLD << YELLOW << "Server " << (*it)->getDynamicListen().port << " \u2192 " << loadingIcons[j % numIcons] << RESET;
                    std::cout.flush();
                    usleep(50000); // Sleep for 100 milliseconds (adjust as needed)
                }
            }

            if (*it) {
                (*it)->initServer();
                std::cout << "\r" << BOLD << GREEN << "Server " << (*it)->getDynamicListen().port << " \u2192 \u2713" << RESET << std::endl; // Unicode check mark symbol
                // UPDATE TO OK
                ++it; // Move to the next iterator position
            }
        } catch (std::exception &e) {
            std::cout << "\r" << BOLD << RED << "Server " << (*it)->getDynamicListen().port << " \u2192 \u2717 : " << RESET; // Unicode cross symbol
            std::cerr << e.what() << std::endl;
            _numberOfSv--;
            delete *it;
            it = _servers.erase(it); // Erase the current element and obtain the iterator to the next element
        }
    }

    std::cout << std::endl << BOLD << GREEN << "─────────────────────────[Running " << _numberOfSv << " server(s)]──────────────────────────" << RESET << std::endl;
}

/* ===================== Server Monitoring and Cleanup Functions ===================== */

/**
 * @brief Clears the last server and its corresponding server block.
 * 
 * This function removes the last server object from the '_servers' vector and the last server block
 * from the '_config' object. It deletes the server object to free up memory and pops it from the vector.
 * If there are no servers or server blocks left, the function does nothing.
 */
void	Cluster::clearServer() {
	if (!_servers.empty()) {
		while (!_servers.back()->getDynamicBody().empty())
			_servers.back()->getDynamicBody().pop_back();
		delete _servers.back();
		_servers.pop_back();
	}
	if (!_config.getSvBlocks().empty())
		_config.getSvBlocks().pop();
}

/**
 * @brief Checks for socket activity and closes inactive connections.
 * 
 * This function iterates over the server sockets and checks if any of them have exceeded the
 * allowed lifetime without any activity. If an inactive connection is found, it is closed and
 * removed from the event buffer.
 * 
 * @param epoll_fd The file descriptor for the epoll instance.
 * @param event_buffer The buffer containing epoll events.
 * @return Returns 0 if no sockets have been closed, 2 if an inactive connection has been closed.
 */
int Cluster::checkTimeout(int epoll_fd, struct epoll_event* event_buffer) {

	// Get current system time
	time_t now = time(NULL);
	std::map<int, Server*>::iterator it = _fdToServerMap.begin();

	// Skip first N positions on map so not to close base servers
	std::advance(it, _numberOfSv);

	for (; it != _fdToServerMap.end(); ++it) {
		int fd = it->first;

		// Check if last activity time for each server is initialized and if it's passed the allowed lifetime
		if (_activityTime[fd] && now - _activityTime[fd] > ACTIVITY_TIMEOUT) {
			
			// Inactive connection found, remove from event_buffer
			it->second->closeConnections(fd, epoll_fd, event_buffer, _fdToServerMap, _activityTime);
			return (2);
		}
	}

	// Return 0 if no sockets have closed
	return (0);
}

/* ===================== Configuration Parser Caller Functions ===================== */

void	Cluster::callerSemicolon(Server* server) {
	_config.checkSemicolon(server->getDynamicBody());
}

void	Cluster::callerDoubles(Server *server) {
	_config.checkDoubles(server->getDynamicBody());
}

void	Cluster::callerServerName(Server* server) {
	_config.checkSvName(server->getDynamicBody(), server->getDynamicConfig());
}

void	Cluster::callerListen(std::string& wordStack, t_listen &listen) {
	std::istringstream iss(wordStack);
	_config.checkListen(iss, listen);
}

void	Cluster::callerServerRoot(Server* server) {
	_config.checkSvRoot(server->getDynamicBody(), server->getDynamicConfig());
}

void Cluster::callerErrorPage(Server* server)
{
	_config.checkErrPage(server->getDynamicBody(), server->getDynamicConfig());
}

void	Cluster::callerIndex(Server* server) {
	_config.checkIndex(server->getDynamicBody(), server->getDynamicConfig());
}

void	Cluster::callerMethods(Server* server) {
	_config.checkMethods(server->getDynamicBody(), server->getDynamicConfig());
}

void	Cluster::callerClientSize(Server* server) {
	_config.checkClientBodySize(server->getDynamicBody(), server->getDynamicConfig());
}

void	Cluster::callerLocations(Server* server) {
	_config.checkLocations(server, server->getDynamicBody(), server->getDynamicConfig());
}

/* ===================== Webserv Execution Functions ===================== */

/**
 * @brief Starts the servers and manages incoming connections.
 * 
 * This function initializes the server sockets, sets up epoll to monitor events,
 * and manages incoming connections. It listens for incoming events and delegates
 * connection handling to the appropriate server instances.
 */
void	Cluster::startServers() {
	try {

		int epoll_fd = epoll_create(_numberOfSv);
		int numEvents;
		if (epoll_fd < 0)
			throw ClusterException("Failed creating EPOLL_FD");
		struct epoll_event events, event_buffer[10];
		events.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;

		// Create and link each base server socket to their corresponding servers via map
		for (size_t i = 0; i < _pollFDs.size(); i++) {
			events.data.fd = _pollFDs[i].fd;
			_fdToServerMap[_pollFDs[i].fd] = getServer(_pollFDs[i].fd);
			_serverSockets.push_back(_pollFDs[i].fd);
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, events.data.fd, &events) < 0)
				throw ClusterException("Failed controlling epoll for server::" + intToStr(_fdToServerMap[_pollFDs[i].fd]->getListen().port));
		}

		// Main Servers Listen
		while (!gSignalStatus) {

			// Wait for an event/request
			numEvents = epoll_wait(epoll_fd, event_buffer, 10, 5000);
			if (numEvents < 0) {
				if (WIFSIGNALED(gSignalStatus))
					return ; //check favicon < 0
				throw ClusterException("EPOLL_WAIT Failed");
			}

			// Create a buffer for each server socket that manages events
			for (int i = 0; i < numEvents; i++) {

				// First N times, client socket will be each servers base socket
				int client_socket = event_buffer[i].data.fd;
				if(event_buffer[i].events & EPOLLERR) {
					std::cerr << "EPOLLERR event on fd " << client_socket << std::endl;
					close(event_buffer[i].data.fd);
					continue;
				}
				else if(event_buffer[i].events & EPOLLHUP) {
					std::cerr << "EPOLLHUP event on fd " << client_socket << std::endl;
					close(event_buffer[i].data.fd);
					continue;
				}
				else if (std::find(_serverSockets.begin(), _serverSockets.end(), client_socket) != _serverSockets.end()) {
					struct sockaddr_in client_address;
					socklen_t addrlen = sizeof(sockaddr);

					// Generate a new connection socket
					client_socket = accept(event_buffer[i].data.fd, (sockaddr*)&client_address, (socklen_t*)&addrlen);
					if (client_socket < 0)
						continue ;
					event_buffer[i].events = EPOLLIN;
					event_buffer[i].events |= EPOLLOUT;

					// Link connection socket to the corresponding server socket
					// While setting non-block flags for the connection
					_fdToServerMap[client_socket] = _fdToServerMap[event_buffer[i].data.fd];
					_fdToServerMap[client_socket]->setNonBlock(client_socket);
					event_buffer[i].data.fd = client_socket;

					// Add the connection to the buffer
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event_buffer[i]) < 0)
						throw ClusterException("Failed controlling epoll for connection_fd::" + intToStr(client_socket));
				}
				else {
					// Added try catch if need to do any throws on connection (request <-> response) process
					try {
						if(event_buffer[i].events & EPOLLIN)
							connectionHandler(client_socket, _fdToServerMap[client_socket]);
					} catch (std::exception &e) {
						_fdToServerMap[client_socket]->closeConnections(client_socket, epoll_fd, event_buffer, _fdToServerMap, _activityTime);
						std::cerr << e.what() << std::endl;
					}

					// If checkTimeout closes a fd then go back to the beginning, so as not to iterate over possible removed FDs from buffer
					if (checkTimeout(epoll_fd, event_buffer) > 0)
						break ;	
				}
			}
		}
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		clearServer();
	}
}

class MatchFd {
    int _fd;
public:
    MatchFd(int fd) : _fd(fd) {}
    bool operator()(const Connection& c) const {
        return c.getConnectionFD() == _fd;
    }
};

/**
 * @brief Handles an incoming connection.
 * 
 * This function is called when a new connection is accepted. It updates the last activity
 * time for the connection, sets the connection for the server, and initiates the sending process.
 * 
 * @param fd The file descriptor of the incoming connection.
 * @param server Pointer to the server instance handling the connection.
 */
void	Cluster::connectionHandler(int fd, Server* server) {
	_activityTime[fd] = time(NULL);
	if (std::find_if(server->getConnectionVector().begin(), server->getConnectionVector().end(), MatchFd(fd)) == server->getConnectionVector().end()) {
		server->setConnection(fd);
	}
	server->sender(fd);
}

/* ===================== Info Display Functions ===================== */

/**
 * @brief Displays information about each server.
 * 
 * This function prints a formatted table containing details about each server, including
 * the timestamp, server name, address, and port.
 */
void Cluster::promptInfo() {
    std::cout << BOLD << CYAN << "─────────────────────────────────────────────────────────────────────────" << std::endl;
    std::cout << "| " << BOLD << std::setw(20) << "Timestamp" << " | " 
              << BOLD << std::setw(16) << "Server Name" << " | " 
              << BOLD << std::setw(13) << "Address" << " | " 
              << BOLD << std::setw(10) << "Port" << " |" << std::endl;
    std::cout << "─────────────────────────────────────────────────────────────────────────" << RESET << std::endl;
	std::map<int, Server*>::iterator	it = _fdToServerMap.begin();
    for (; it != _fdToServerMap.end(); ++it) {
		Server* server = it->second;
        std::time_t timestamp = std::time(NULL);
        char buff[50];
        std::strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
		std::string serverName = server->getConf().server_name.back();
		std::string address = ipToStr(server->getListen().host);
		int port = server->getListen().port;
		std::cout << CYAN << "| " << GREEN << std::setw(20) << buff << CYAN << " | " 
				<< YELLOW << std::setw(16) << serverName << CYAN << " | " 
				<< YELLOW << std::setw(13) << address << CYAN << " | " 
				<< YELLOW << std::setw(10) << port << CYAN << " |" << RESET << std::endl;
    }
    std::cout << BOLD << CYAN << "─────────────────────────────────────────────────────────────────────────" << RESET << std::endl;
}

/* ===================== Exceptions ===================== */

Cluster::ClusterException::ClusterException(const std::string& error) {
	_errMessage = "Error: " + error;
}

Cluster::ClusterException::~ClusterException() throw() {}


const char *Cluster::ClusterException::what() const throw() {
	return (_errMessage.c_str());
}

