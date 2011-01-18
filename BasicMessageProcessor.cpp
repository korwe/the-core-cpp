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

#include "BasicMessageProcessor.h"
#include <syslog.h>

BasicMessageProcessor::BasicMessageProcessor(std::string queueServer, std::string sessionId) {
    sessionId_ = sessionId;
    queueServer_ = queueServer;
    serviceSender_ = new MessageSender(queueServer_, Queues::CORE_SERVICE);
    clientSender_ = new MessageSender(queueServer_, Queues::CORE_CLIENT);
    dataSender_ = new MessageSender(queueServer_, Queues::CORE_DATA);
}

BasicMessageProcessor::~BasicMessageProcessor() {
    delete serviceSender_;
    delete clientSender_;
    delete dataSender_;
}

const std::string BasicMessageProcessor::sessionId() const {
    return sessionId_;
}

const std::string BasicMessageProcessor::queueServer() const {
    return queueServer_;
}

void BasicMessageProcessor::setSessionId(const std::string& sessionId) {
    sessionId_ = sessionId;
}

void BasicMessageProcessor::setQueueServer(const std::string& queueServer) {
    queueServer_ = queueServer;
}

const bool BasicMessageProcessor::shouldProcessMessage(const CoreMessage* message) {
    return (message->messageType() == CoreMessage::SERVICE_REQUEST  ||
            message->messageType() == CoreMessage::SERVICE_RESPONSE ||
            message->messageType() == CoreMessage::KILL_SESSION_REQUEST) &&
            message->sessionId() == sessionId();
}

void BasicMessageProcessor::processMessage(const CoreMessage* message) {
    switch (message->messageType()) {
        case CoreMessage::SERVICE_REQUEST: {
            syslog(LOG_DEBUG, "Got request, forwarding to service");
            serviceSender_->sendCoreMessage(*message, message->choreography());
            break;
        }
        case CoreMessage::SERVICE_RESPONSE: {
            syslog(LOG_DEBUG, "Got response, forwarding to client");
            clientSender_->sendCoreMessage(*message, sessionId());
            break;
        }
        default:
            break;
    }
}
