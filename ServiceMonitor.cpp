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
 * The Core Session Manager
 * Compile with g++ -o CoreSessionManager SessionManager.cpp Messaging.cpp \
 * CoreSession.cpp pugixml.cpp -lboost_thread -L/usr/local/lib -lqpidclient
 * To run as a daemon run CoreStart.
 *
 */
 
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include <syslog.h>
#include <boost/lexical_cast.hpp>
#include <Messaging.h>
#include <CoreMessageSerializer.h>


void signal_handler( int sig)
{
    switch(sig) {
    case SIGHUP:
        // TODO: Reload config
        syslog(LOG_INFO,"hangup signal caught");
        break;
    case SIGTERM:
        syslog(LOG_INFO,"terminate signal caught");
        syslog(LOG_INFO,"exiting");
        exit(0);
        break;
    }
}

int main(int argc, char* argv[]) {
    openlog("CoreServiceMonitor", LOG_PID, LOG_DAEMON);
    signal(SIGHUP, signal_handler); /* catch hangup signal */
    signal(SIGTERM,signal_handler); /* catch kill signal */
    std::string queue_server(argv[1]);
    int port = 5672;
    bool trace = false;
    if (argc > 2) {
        std::string port_str(argv[2]);
        port = boost::lexical_cast<int>(port_str);
    }
    if (argc > 3) {
        if (strcmp(argv[3], "trace") == 0) {
            trace = true;
        }
    }

    MessageReceiver receiver(queue_server, Queues::SERVICE_CORE, port);
    syslog(LOG_INFO,"Created receiver");

    MessageSender traceSender(queue_server, Queues::CORE_TRACE, port);
    syslog(LOG_INFO,"Created trace sender");
    MessageSender sessionSender(queue_server, Queues::CORE_SESSION, port);
    syslog(LOG_INFO,"Checking for messages");
    CoreMessageXmlSerializer serializer;

    while (true) {

        std::string rawMessage = receiver.getNextMessage(10000);
        syslog(LOG_DEBUG, "%s", rawMessage.c_str());
        if (!rawMessage.empty()) {
            try {
                if (trace) {
                    traceSender.sendMessage(rawMessage, "");
                }
                CoreMessage* message = serializer.deserialize(rawMessage);
                if (message) {
                    std::string sessionId = message->sessionId();

                    syslog(LOG_DEBUG, "%s", sessionId.c_str());
                    syslog(LOG_DEBUG, "%s", CoreMessage::lookupTypeName(message->messageType()).c_str());
                    switch (message->messageType()) {

                        case CoreMessage::DATA_RESPONSE:
                        case CoreMessage::SERVICE_RESPONSE: {
                            syslog(LOG_DEBUG, "Forwarding to session");
                            sessionSender.sendMessage(serializer.serialize(*message), sessionId);
                            break;
                        }
                        default: {
                            syslog(LOG_WARNING, "Invalid message");
                            break;
                        }
                    }
                    delete message;
                }
                else {
                    syslog(LOG_ERR, "Message could not be deserialized");
                }
            }
            catch (std::exception& e) {
                syslog(LOG_ERR, "Exception caught: %s", e.what());
            }
        }
        else {
            syslog(LOG_DEBUG, "No service messages for 10 sec.");
        }
    }
    
    syslog(LOG_DEBUG, "Done");
    
}

