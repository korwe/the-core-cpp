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
 
#include <string>
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
#include <queue>
#include <boost/thread/thread.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include <CoreMessage.h>
#include <CoreMessageSerializer.h>
#include "CoreSession.h"

// Oh horrible, oh horrible, most horible!
boost::ptr_map<std::string, CoreSession> sessions;
boost::ptr_map<std::string, boost::thread> threads;
std::queue<std::string> toCreate;

void createAndRunSession() {
    if (toCreate.empty()) return;
    std::string key = toCreate.front();
    toCreate.pop();
    int separator = key.find("|");
    std::string sessionId = key.substr(0, separator);
    std::string server = key.substr(separator + 1);
    syslog(LOG_INFO, "Starting thread for session %s", sessionId.c_str());
    sessions.insert(sessionId, new CoreSession(server, sessionId));
    CoreSession& created = sessions[sessionId];
    created.run();
}


void stopSessions() {
    for(boost::ptr_map<std::string,CoreSession>::iterator i = sessions.begin(), e = sessions.end();
        i != e; ++i ) {
        i->stop();
        threads[i->sessionId()].join();
    }
}

void removeSession(std::string sessionId) {
    sessions[sessionId].stop();
    threads[sessionId].join();
    threads.erase(threads.find(sessionId));
    sessions.erase(sessions.find(sessionId));
}

void signal_handler( int sig)
{
    switch(sig) {
    case SIGHUP:
        // TODO: Reload config
        syslog(LOG_INFO,"hangup signal caught");
        break;
    case SIGTERM:
        syslog(LOG_INFO,"terminate signal caught");
        syslog(LOG_INFO,"stopping sessions");
        stopSessions();
        syslog(LOG_INFO,"exiting");
        exit(0);
        break;
    }
}

int main(int argc, char* argv[]) {
    openlog("CoreSessionManager", LOG_PID, LOG_DAEMON);
    signal(SIGHUP, signal_handler); /* catch hangup signal */
    signal(SIGTERM,signal_handler); /* catch kill signal */
    std::string queue_server(argv[1]);

    MessageReceiver receiver(queue_server, Queues::CLIENT_CORE);        
    syslog(LOG_INFO,"Created receiver");

    MessageSender traceSender(queue_server, Queues::CORE_TRACE);
    syslog(LOG_INFO,"Created trace sender");
    MessageSender sessionSender(queue_server, Queues::CORE_SESSION);
    MessageSender clientSender(queue_server, Queues::CORE_CLIENT);
    syslog(LOG_INFO,"Checking for messages");
    CoreMessageXmlSerializer serializer;

    while (true) {

        std::string rawMessage = receiver.getNextMessage(10000);
        syslog(LOG_DEBUG, rawMessage.c_str());
        if (!rawMessage.empty()) {
            try {
                traceSender.sendMessage(rawMessage, "");
                CoreMessage* message = serializer.deserialize(rawMessage);
                if (message) {
                    std::string sessionId = message->sessionId();

                    syslog(LOG_DEBUG, sessionId.c_str());
                    syslog(LOG_DEBUG, CoreMessage::lookupTypeName(message->messageType()).c_str());
                    switch (message->messageType()) {
                        case CoreMessage::INITIATE_SESSION_REQUEST: {
                            if (sessions.find(sessionId) == sessions.end()) {
                                syslog(LOG_INFO, "Starting session");
                                std::string sessionStartKey = sessionId + "|" + queue_server;
                                toCreate.push(sessionStartKey);
                                threads.insert(sessionId, new boost::thread(createAndRunSession));
                                InitiateSessionResponse response(sessionId, message->guid());
                                response.setSuccessful(true);
                                clientSender.sendMessage(serializer.serialize(response), sessionId);
                                syslog(LOG_DEBUG, serializer.serialize(response).c_str());
                            }
                            else {
                                syslog(LOG_WARNING, "Session %s already exists", sessionId.c_str());
                                InitiateSessionResponse response(sessionId, message->guid());
                                response.setSuccessful(false);
                                response.setErrorCode("BadSession");
                                response.setErrorMessage("Session " + sessionId + " already exists");
                                clientSender.sendMessage(serializer.serialize(response), sessionId);
                                syslog(LOG_DEBUG, serializer.serialize(response).c_str());
                            }
                            break;
                        }
                        case CoreMessage::KILL_SESSION_REQUEST: {
                            if (sessions.find(sessionId) != sessions.end()) {
                                syslog(LOG_INFO, "Killing session %s", sessionId.c_str());
                                sessionSender.sendMessage(serializer.serialize(*message), sessionId);
                                removeSession(sessionId);
                                KillSessionResponse response(sessionId, message->guid());
                                response.setSuccessful(true);
                                clientSender.sendMessage(serializer.serialize(response), sessionId);
                                syslog(LOG_DEBUG, serializer.serialize(response).c_str());
                            }
                            else {
                                syslog(LOG_WARNING, "Invalid session %s", sessionId.c_str());
                                KillSessionResponse response(sessionId, message->guid());
                                response.setSuccessful(false);
                                response.setErrorCode("BadSession");
                                response.setErrorMessage("Session " + sessionId + " does not exist");
                                clientSender.sendMessage(serializer.serialize(response), sessionId);
                                syslog(LOG_DEBUG, serializer.serialize(response).c_str());
                            }                            
                            break;
                        }
                        case CoreMessage::SERVICE_REQUEST: {
                            if (sessions.find(sessionId) != sessions.end()) {
                                syslog(LOG_DEBUG, "Forwarding to session");

                                sessionSender.sendMessage(serializer.serialize(*message), sessionId);
                            }
                            else {
                                syslog(LOG_WARNING, "Invalid session %s", sessionId.c_str());
                                ServiceResponse response(sessionId, message->guid(), false);
                                response.setSuccessful(false);
                                response.setErrorCode("BadSession");
                                response.setErrorMessage("Session " + sessionId + " does not exist");
                                clientSender.sendMessage(serializer.serialize(response), sessionId);
                                syslog(LOG_DEBUG, serializer.serialize(response).c_str());
                            }
                            break;
                        }
                        default: {
                            syslog(LOG_WARNING, "Invalid message");
                            ServiceResponse response(sessionId, message->guid(), false);
                            response.setSuccessful(false);
                            response.setErrorCode("BadMessage");
                            response.setErrorMessage("Invalid message type");
                            clientSender.sendMessage(serializer.serialize(response), sessionId);
                            syslog(LOG_DEBUG, serializer.serialize(response).c_str());
                            break;
                        }
                    }
                    delete message;
                }
            }
            catch (std::exception& e) {
                syslog(LOG_ERR, "Exception caught: %s", e.what());
            }
        }
        else {
            syslog(LOG_DEBUG, "No messages for 10 sec. Nobody loves me anymore");
        }
    }
    
    syslog(LOG_DEBUG, "Done");
    
}
