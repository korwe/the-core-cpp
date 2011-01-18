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
 * Core Session implementation
 */
 
#include "CoreSession.h"
#include "BasicMessageProcessor.h"

CoreSession::CoreSession() {}

CoreSession::CoreSession(std::string queue_server, std::string sessionId) {
    _sessionId = sessionId;
    _server = queue_server;
    _stopRequested = false;
    _receiver = new MessageReceiver(_server, Queues::CORE_SESSION, _sessionId);
    _processor = new BasicMessageProcessor(_server, _sessionId);
}

CoreSession::~CoreSession() {
    delete _receiver;
    delete _processor;
}

const std::string& CoreSession::sessionId() const {
    return _sessionId;
}

void CoreSession::stop() {
    _stopRequested = true;
}

void CoreSession::run() {
    while (!_stopRequested) {
        CoreMessage* message = _receiver->getNextCoreMessage(10000);
        if (message) {
            if (CoreMessage::KILL_SESSION_REQUEST == message->messageType()) {
                stop();
                break;
            }
            else if (_processor->shouldProcessMessage(message)) {
                _processor->processMessage(message);
            }
        }
    }
}

