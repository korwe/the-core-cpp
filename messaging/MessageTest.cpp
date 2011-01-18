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

/**
 * Unit tests for basic message sending and receiving 
**/

#include <iostream>
#include "Messaging.h"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#define Q_SERVER "172.16.68.132"
#define MESSAGE_TEXT "<coreMessage><sessionId>123123123</sessionId><messageType>InitiateSession</messageType></coreMessage>"
#define MESSAGE_TEXT_2 "<coreMessage><sessionId>234234234</sessionId><messageType>InitiateSession</messageType></coreMessage>"

BOOST_AUTO_TEST_CASE(testDirectQueue) {
    MessageReceiver receiver(Q_SERVER, Queues::CLIENT_CORE);
    MessageSender sender(Q_SERVER, Queues::CLIENT_CORE);
    sender.sendMessage(MESSAGE_TEXT, "123123123");
    std::string received = receiver.getNextMessage(1500);
    BOOST_CHECK_EQUAL(received, MESSAGE_TEXT);
}

BOOST_AUTO_TEST_CASE(testPublishSubscribe) {
    MessageReceiver receiver123(Q_SERVER, Queues::CORE_SESSION, "123123124");
    MessageReceiver receiver234(Q_SERVER, Queues::CORE_SESSION, "234234234");
    MessageSender sender(Q_SERVER, Queues::CORE_SESSION);
    sender.sendMessage(MESSAGE_TEXT, "123123124");
    sender.sendMessage(MESSAGE_TEXT_2, "234234234");
    std::string received1 = receiver123.getNextMessage(1500);
    std::string received2 = receiver234.getNextMessage(1500);
    BOOST_CHECK_EQUAL(received1, MESSAGE_TEXT);
    BOOST_CHECK_EQUAL(received2, MESSAGE_TEXT_2);
}

BOOST_AUTO_TEST_CASE(testXmlParseNoError) {
    MessageSender sender(Q_SERVER, Queues::CORE_SESSION);
    MessageReceiver receiver(Q_SERVER, Queues::CLIENT_CORE);
    BOOST_CHECK_NO_THROW( 
    {
        sender.sendMessage(MESSAGE_TEXT, "123123123");
        pugi::xml_document doc = receiver.getNextXmlMessage(1500);
    });
}
