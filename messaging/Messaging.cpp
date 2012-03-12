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

/** * Messaging implementation */

#include <sstream>
#include <qpid/client/Message.h>
#include <qpid/sys/Time.h>
#include <syslog.h>
#include "Messaging.h"


const std::string Queues::queueName(Queues::Queue queue) {
    switch(queue) {
        case Queues::CLIENT_CORE:  return "core.client-core";
        case Queues::CORE_SESSION: return "core.core-session";
        case Queues::CORE_SERVICE: return "core.core-service";
        case Queues::SERVICE_CORE: return "core.service-core";
        case Queues::CORE_CLIENT:  return "core.core-client";
        case Queues::CORE_DATA:    return "core.data";
        case Queues::CORE_TRACE:   return "core.trace";
        default:                   return "unknown";
    }
}

const bool Queues::isDirect(Queues::Queue queue) {
    switch (queue) {
        case Queues::CLIENT_CORE:  // Fall through
        case Queues::SERVICE_CORE: // Fall through
        case Queues::CORE_TRACE:   return true;
        default:                   return false;
    }
}

MessageSender::MessageSender(std::string queue_server, Queues::Queue destination, int server_port) {
    _server = queue_server;
    _port = server_port;
    _queue = destination;
    _connection = new qpid::client::Connection();
    syslog(LOG_DEBUG, "Connecting to %s:%d", _server.c_str(), _port);
    _connection->open(_server, _port);
    syslog(LOG_DEBUG, "Connected");
}

MessageSender::~MessageSender() {
    _connection->close();
    delete _connection;
}


void MessageSender::sendMessage(std::string content, std::string sessionId) {
    std::string amqDestination = Q_DIRECT;
    std::string amqRouting = Queues::queueName(_queue);
    if (!Queues::isDirect(_queue)) {
        amqDestination = Q_TOPIC;
        amqRouting = Queues::queueName(_queue) + "." + sessionId;
    }
    using namespace qpid::client;
    Message amqMessage;
    amqMessage.setData(content);
    amqMessage.getDeliveryProperties().setRoutingKey(amqRouting);
    Session session = _connection->newSession();
    session.messageTransfer(arg::content=amqMessage, arg::destination=amqDestination);
    session.close();
}

void MessageSender::sendXmlMessage(pugi::xml_document& content, std::string sessionId) {
    std::stringstream ss;
    pugi::xml_writer_stream writer(ss);
    content.print(writer);
    sendMessage(ss.str(), sessionId);
}

void MessageSender::sendCoreMessage(const CoreMessage& content, std::string sessionId) {
    sendMessage(_serializer.serialize(content), sessionId);
}

MessageReceiver::MessageReceiver(std::string queue_server, Queues::Queue source, int server_port) {
    _server = queue_server;
    _port = server_port;
    _queue = source;
    _queueName = Queues::queueName(_queue);
    syslog(LOG_DEBUG, "Connecting to %s:%d", _server.c_str(), _port);
    _connection = new qpid::client::Connection();
    _connection->open(_server, _port);
    syslog(LOG_DEBUG, "Creating session");
    _session = new qpid::client::Session(_connection->newSession());
    syslog(LOG_DEBUG, "Creating subs manager");
    _subscriptions = new qpid::client::SubscriptionManager::SubscriptionManager(*_session);
    syslog(LOG_DEBUG, "Creating local queue");
    _localQueue = new qpid::client::LocalQueue();
    syslog(LOG_DEBUG, "Creating subscription");
    _subscriptions->subscribe(*_localQueue, _queueName);
    syslog(LOG_DEBUG, "Done creating receiver");
}

MessageReceiver::MessageReceiver(std::string queue_server, Queues::Queue source, std::string sessionId, int server_port) {
    _server = queue_server;
    _port = server_port;
    _queue = source;
    syslog(LOG_DEBUG, "Connecting to %s:%d", _server.c_str(), _port);
    _connection = new qpid::client::Connection();
    _connection->open(_server, _port);
    _queueName = Queues::queueName(_queue) + "." + sessionId;
    using namespace qpid::client;
    _session = new qpid::client::Session(_connection->newSession());
    _session->queueDeclare(arg::queue=_queueName, arg::exclusive=false, arg::autoDelete=true);
    _session->exchangeBind(arg::exchange=Q_TOPIC, arg::queue=_queueName, arg::bindingKey=_queueName);
    _subscriptions = new SubscriptionManager::SubscriptionManager(*_session);
    _localQueue = new qpid::client::LocalQueue();
    _subscriptions->subscribe(*_localQueue, _queueName);
}

MessageReceiver::~MessageReceiver() {
    _subscriptions->cancel(Queues::queueName(_queue));
    delete _subscriptions;
    delete _localQueue;
    _session->close();
    delete _session;
    _connection->close();
    delete _connection;
}

std::string MessageReceiver::getNextMessage(int timeoutMillis) {
    qpid::client::Message message;
    if (_localQueue->get(message, timeoutMillis*qpid::sys::TIME_MSEC)) {
        return message.getData();
    }
    return std::string();
    
}

pugi::xml_document MessageReceiver::getNextXmlMessage(int timeoutMillis) {
    pugi::xml_document doc;
    std::string messageData = getNextMessage(timeoutMillis);
    if (!messageData.empty()) {
        doc.load(messageData.c_str());
    }
    return doc;
}

boost::shared_ptr<CoreMessage> MessageReceiver::getNextCoreMessage(int timeoutMillis) {
    std::string messageData = getNextMessage(timeoutMillis);
    if (!messageData.empty()) {
        return boost::shared_ptr<CoreMessage>(_serializer.deserialize(messageData));
    }
    return boost::shared_ptr<CoreMessage>();
}
