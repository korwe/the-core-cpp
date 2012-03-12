/* 
 * Copyright 2010 Korwe Software
 *
 * This file is part of TheCore.
 *
 * TheCore is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * TheCore is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TheCore.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
X Daemon Server Programming Sample Program
Based on Levent Karakas <levent at mektup dot at> May 2001
Date: 24 Aug 2009 DJH

To compile: cpp -o CoreStart CoreStart.cpp ...
To run:     ./CoreStart
To test daemon: ps -ef|grep CoreSessionManager
To test log:    tail -f /var/log/messages
To test signal: kill -HUP `cat /var/run/CoreSessionManager.pid`
To terminate:   kill `cat /var/run/CoreSessionManager.pid`
*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <syslog.h>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "Version.h"


#define RUN_DIR "/var/run"
#define LOCK_FILE   "CoreClientMonitor.pid"

void daemonize() {
     struct rlimit resourceLimit = { 0 };
     int status = -1;
     int fileDesc = -1;
     int lockFileDesc = -1;
     char str[10];
     status = fork();
     switch (status)
     {
     case -1:
         perror("fork()");
         exit(1);
     case 0: /* child process */
         break;
     default: /* parent process */
         exit(0);
     }
     /*
      * child process
      */
     resourceLimit.rlim_max = 0;
     status = getrlimit(RLIMIT_NOFILE, &resourceLimit);
     if (-1 == status) /* shouldn't happen */
     {
         perror("getrlimit()");
         exit(1);
     }
     if (0 == resourceLimit.rlim_max)
     {
         printf("Max number of open file descriptors is 0!!\n");
         exit(1);
     }
     for (int i = 0; i < resourceLimit.rlim_max; i++)
     {
         (void) close(i);
     }
     status = setsid();
     if (-1 == status)
     {
         perror("setsid()");
         exit(1);
     }
     status = fork();
     switch (status)
     {
     case -1:
         perror("fork()");
         exit(1);
     case 0: /* (second) child process */
         break;
     default: /* parent process */
         exit(0);
     }
     /*
      * now we are in a new session and process
      * group than process that started the
      * daemon. We also have no controlling
      * terminal */
     chdir(RUN_DIR);
     umask(0);
     lockFileDesc = open(LOCK_FILE, O_RDWR|O_CREAT, 0640);

     if (lockFileDesc < 0) exit(1); /* cannot open */

     if (lockf(lockFileDesc, F_TLOCK, 0) < 0) exit(0); /* cannot lock */

     /* first instance continues */
     sprintf(str, "%d\n", getpid());
     write(lockFileDesc, str, strlen(str)); /* record pid to lockfile */
     fileDesc = open("/dev/null", O_RDWR);/* stdin */
     (void) dup(fileDesc);  /* stdout */
     (void) dup(fileDesc);  /* stderr */
}

int main(int argc, char* argv[])
{
    std::string queue_server;
    std::string queue_port;
    po::options_description generic("Generic options");
    generic.add_options()
        ("version,v", "Print version string")
        ("help,h", "Print help message")
        ;
    po::options_description config("Configuration");
    config.add_options()
        ("queue-server,q", po::value(&queue_server), "Queue server hostname/IP")
        ("queue-port,p", po::value(&queue_port), "Queue server port")
        ;
    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config);
    po::options_description file_options;
    file_options.add(config);
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmdline_options), vm);
    std::ifstream ifs("thecore.conf");
    po::store(po::parse_config_file(ifs, file_options), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << cmdline_options << "\n";
        return 0;
    }

    if (vm.count("version")) {
        printf("The Core, version %s\n", THE_CORE_VERSION);
        return 0;
    }
    
    if (!vm.count("queue-server")) {
        printf("Cannot start. Queue server not specified.\n");
        syslog(LOG_CRIT,"Cannot start. Queue server not specified.");
        return 1;
    }
    
    char path[200];
    getcwd(path, 199);
    daemonize();
    int status;
    status = fork();
    switch (status)
    {
    case -1:
        perror("fork()");
        exit(1);
    case 0: /* (third) child process */
        strcat(path, "/CoreServiceMonitor");
        if (!queue_port.empty()) {
            status = execlp(path, path, queue_server.c_str(), queue_port.c_str(), 0);
        } else {
            status = execlp(path, path, queue_server.c_str(), 0);
        }
        wait(&status);
        break;
    default: /* parent process */
        strcat(path, "/CoreClientMonitor");
         if (!queue_port.empty()) {
            status = execlp(path, path, queue_server.c_str(), queue_port.c_str(), 0);
        } else {
            status = execlp(path, path, queue_server.c_str(), 0);
        }
        wait(&status);
        break;
    }
}

/* EOF */

