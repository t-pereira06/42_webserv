/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lde-sous <lde-sous@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/06 15:36:38 by lde-sous          #+#    #+#             */
/*   Updated: 2024/05/17 12:50:24 by lde-sous         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "headers/config/confParser.hpp"
// #include "headers/server/Cluster.hpp"
#include "./headers/webserv.hpp"
#include "./headers/server/Server.hpp"
#include "./headers/server/Cluster.hpp"

//Cluster* GlobalServerPtr = NULL;
volatile sig_atomic_t gSignalStatus = 0;
Cluster* gServerCluster = NULL;
bool chunky = false;


/**
 * @brief Signal handler function for handling signals in the program.
 *
 * @param signum The signal number.
 */
extern "C" void	signalHandler(int signum) 
{

    if (gServerCluster) 
    {
        std::vector<Server*>::const_iterator it;
        for (it = gServerCluster->getServers().begin(); it != gServerCluster->getServers().end(); ++it)
            close((*it)->getFD());
       // gServerCluster->clearServer();
    }
    (void)signum;
	gSignalStatus = 1;
    const char* loadingIcons[] = {".", "..", "..."};
    const int numIcons = sizeof(loadingIcons) / sizeof(loadingIcons[0]);

    std::cout << "Closing program safely ";
    std::cout.flush();

    for (int j = 0; j < 3; ++j)
    {
        std::cout << "\r" << BOLD_BLUE << "Closing program safely " << loadingIcons[j % numIcons] << RESET;
        std::cout.flush();
        usleep(800000); // Sleep for 100 milliseconds (adjust as needed)
    }
    std::cout << "\r" << BOLD_GREEN << "Closing program safely \u2713" << RESET;
    std::cout << "                 \n"; // Clear the rest of the line
    std::cout << std::endl;
}

void printWebServLogo(char **envp)
{
	pid_t pid = fork();
	if (pid == 0)
	{
		 char* argv[] = {NULL};
		if (execve("/usr/bin/clear", argv, envp) == -1)
			exit (EXIT_FAILURE);
	}
	waitpid(pid, NULL, 0);
	std::cout << BOLD << RED << "\n";
	std::cout << " █     █░  ▓█████    ▄▄▄▄       ██████   ▓█████    ██▀███     ██▒   █▓\n";
	std::cout << " ▓█░ █ ░█  ░▓█   ▀   ▓█████▄   ▒██    ▒   ▓█   ▀   ▓██ ▒ ██▒  ▓██░   █▒\n";
	std::cout << "▒█░ █ ░█   ▒███     ▒██▒ ▄██░   ▓██▄     ▒███     ▓██ ░▄█ ▒   ▓██  █▒░\n";
	std::cout << "░█░ █ ░█   ▒▓█  ▄   ▒██░█▀      ▒   ██▒  ▒▓█  ▄   ▒██▀▀█▄      ▒██ █░░\n";
	std::cout << "░░██▒██▓   ░▒████▒░  ▓█  ▀█▓  ▒██████▒  ▒░▒████▒  ░██▓ ▒██▒     ▒▀█░  \n";
	std::cout << "░ ▓░▒ ▒    ░░ ▒░ ░  ░▒▓███▀▒▒   ▒▓▒ ▒   ░░░ ▒░ ░  ░ ▒▓ ░▒▓░     ░ ▐░  \n";
	std::cout << "  ▒ ░ ░     ░ ░  ░  ▒░▒   ░ ░   ░▒  ░   ░ ░ ░  ░    ░▒ ░ ▒░     ░ ░░  \n"; 
	std::cout << "  ░   ░       ░      ░    ░ ░    ░  ░       ░       ░░   ░        ░░  \n";
	std::cout << "    ░         ░  ░   ░              ░       ░  ░     ░             ░  \n";
	std::cout << "                       ░                                 ░" << RESET << std::endl;
}

int		main(int ac, char **av, char **envp)
{
    printWebServLogo(envp);
    std::string filename;
    signal(SIGPIPE, SIG_IGN);
    if (ac != 2)
        std::cerr << "Run the program with a configuration file: ./webserv [configuration file]" << std::endl;
    else
	{
        signal(SIGINT, signalHandler);
        Cluster cluster(av[1]);
        gServerCluster = &cluster;
        cluster.startServers();
    }
	return (EXIT_SUCCESS);
}
