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


#include <iostream>
#include <CoreMessage.h>
#include <Messaging.h>
#include <boost/version.hpp>


#if ((BOOST_VERSION / 100000) == 1) && ((BOOST_VERSION / 100 % 1000) <= 33)
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#else 
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#endif

#define Q_SERVER "172.16.68.132"
#define SESSION_ID "12345678"

static MessageReceiver receiver(Q_SERVER, Queues::CORE_CLIENT, SESSION_ID);
static MessageReceiver dataReceiver(Q_SERVER, Queues::CORE_DATA, SESSION_ID);
static MessageSender sender(Q_SERVER, Queues::CLIENT_CORE);

BOOST_AUTO_TEST_CASE(testInitiateSession) {
    sender.sendCoreMessage(InitiateSessionRequest(SESSION_ID), SESSION_ID);
    boost::shared_ptr<InitiateSessionResponse> response = boost::dynamic_pointer_cast<InitiateSessionResponse>(receiver.getNextCoreMessage(500));
    BOOST_REQUIRE(response != 0);
    BOOST_CHECK(response->isSuccessful());
}

BOOST_AUTO_TEST_CASE(testSuccessfulServiceRequest) {
    ServiceRequest req(SESSION_ID, "TestMe");
    req.setParameter("Param 0", "Value 0");
    req.setParameter("Param 1", "Value 1");
    req.setParameter("Param 2", "Value 2");
    req.setChoreography("TestService");
    sender.sendCoreMessage(req, SESSION_ID);
    boost::shared_ptr<ServiceResponse> serviceResponse = boost::dynamic_pointer_cast<ServiceResponse>(receiver.getNextCoreMessage(1500));
    BOOST_REQUIRE(serviceResponse != 0);
    BOOST_REQUIRE(serviceResponse->isSuccessful());
    BOOST_REQUIRE(serviceResponse->hasData());
    boost::shared_ptr<DataResponse> dataResponse = boost::dynamic_pointer_cast<DataResponse>(dataReceiver.getNextCoreMessage(1500));
    BOOST_REQUIRE(dataResponse != 0);
    BOOST_CHECK_EQUAL(dataResponse->data(), "<data>This is the data</data>");
}

BOOST_AUTO_TEST_CASE(testServiceRequestError) {
    ServiceRequest reqError(SESSION_ID, "TestError");
    reqError.setChoreography("TestService");
    sender.sendCoreMessage(reqError, SESSION_ID);
    boost::shared_ptr<ServiceResponse> serviceResponse = boost::dynamic_pointer_cast<ServiceResponse>(receiver.getNextCoreMessage(1500));
    BOOST_REQUIRE(serviceResponse != 0);
    BOOST_CHECK(!serviceResponse->isSuccessful());
    BOOST_CHECK(!serviceResponse->hasData());
    BOOST_CHECK_EQUAL(serviceResponse->errorCode(), "ERR00111");
    BOOST_CHECK_EQUAL(serviceResponse->errorMessage(), "Testing error response");
}

BOOST_AUTO_TEST_CASE(testKillSession) {
    sender.sendCoreMessage(KillSessionRequest(SESSION_ID), SESSION_ID);
    boost::shared_ptr<KillSessionResponse> killResponse = boost::dynamic_pointer_cast<KillSessionResponse>(receiver.getNextCoreMessage(15000));
    BOOST_REQUIRE(killResponse != 0);
    BOOST_CHECK(killResponse->isSuccessful());
}
