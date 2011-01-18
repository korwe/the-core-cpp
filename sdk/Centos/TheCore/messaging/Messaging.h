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
 * Messaging.h
 *
 *  Created on: Sep 7, 2009
 *      Author: nithia
 */

#ifndef MESSAGING_H_
#define MESSAGING_H_

#include <string>
#include <qpid/client/Connection.h>
#include <qpid/client/Session.h>
#include <qpid/client/LocalQueue.h>
#include <qpid/client/SubscriptionManager.h>

#include "pugixml.hpp"
#include "CoreMessage.h"
#include "CoreMessageSerializer.h"

#define Q_DIRECT "core.direct"
#define Q_TOPIC "core.topic"

class Queues {
    public:
        enum Queue {
            CLIENT_CORE,
            CORE_SESSION,
            CORE_SERVICE,
            SERVICE_CORE,
            CORE_CLIENT,
            CORE_DATA,
            CORE_TRACE
        };
        static const std::string queueName(Queue queue);
        static const bool isDirect(Queue queue);
};

class MessageSender {
    public:
        MessageSender(std::string queue_server, Queues::Queue destination);
        ~MessageSender();
        void sendMessage(std::string content, std::string sessionId);
        void sendXmlMessage(pugi::xml_document& content, std::string sessionId);
        void sendCoreMessage(const CoreMessage& content, std::string sessionId);
    private:
        std::string _server;
        Queues::Queue _queue;
        qpid::client::Connection* _connection;
        CoreMessageXmlSerializer _serializer;
};

class MessageReceiver {
    public:
        MessageReceiver(std::string queue_server, Queues::Queue source);
        MessageReceiver(std::string queue_server, Queues::Queue source, std::string sessionId);
        ~MessageReceiver();
        std::string getNextMessage(int timeoutMillis);
        pugi::xml_document getNextXmlMessage(int timeoutMillis);
        CoreMessage* getNextCoreMessage(int timeoutMillis);
    private:
        std::string _server;
        std::string _queueName;
        Queues::Queue _queue;
        qpid::client::Connection* _connection;
        qpid::client::Session* _session;
        qpid::client::SubscriptionManager* _subscriptions;
        qpid::client::LocalQueue* _localQueue;
        CoreMessageXmlSerializer _serializer;
};

#endif /* MESSAGING_H_ */
