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

#define Q_SERVER "172.16.68.142"
//#define Q_SERVER "korwe.dnsalias.com"
#define SESSION_ID "12345678"

int handleService(ServiceRequest& req, MessageSender& sender, MessageReceiver& receiver, MessageReceiver& dataReceiver) {
    sender.sendCoreMessage(req, SESSION_ID);
    boost::shared_ptr<ServiceResponse> serviceResponse = boost::dynamic_pointer_cast<ServiceResponse>(receiver.getNextCoreMessage(1500));
    if (serviceResponse == 0) {
        std::cout << "Failure: No response to service request" << std::endl;
        return 1;
    }
    if (serviceResponse->isSuccessful()) {
            std::cout << "Request successful" << std::endl;
        if (serviceResponse->hasData()) {
            std::cout << "Response has associated data" << std::endl;
            boost::shared_ptr<DataResponse> dataResponse = boost::dynamic_pointer_cast<DataResponse>(dataReceiver.getNextCoreMessage(1500));
            if (dataResponse == 0) {
                std::cout << "Failure: Associated data not available" << std::endl;
                return 1;
            }
            std::cout << "Response data: " << dataResponse->data() << std::endl;
        }
    }
    else {
        std::cout << "Request unsuccessful" << std::endl;
        std::cout << "Error code: " << serviceResponse->errorCode() << std::endl;
        std::cout << "Error message: " << serviceResponse->errorMessage() << std::endl;
    }
    return 0;
}

int main (int argc, char const *argv[])
{
    MessageReceiver receiver(Q_SERVER, Queues::CORE_CLIENT, SESSION_ID);
    MessageReceiver dataReceiver(Q_SERVER, Queues::CORE_DATA, SESSION_ID);
    MessageSender sender(Q_SERVER, Queues::CLIENT_CORE);
    
    std::cout << std::endl << "Test case: Initiate session request" << std::endl;
    sender.sendCoreMessage(InitiateSessionRequest(SESSION_ID), SESSION_ID);
    boost::shared_ptr<InitiateSessionResponse> response = boost::dynamic_pointer_cast<InitiateSessionResponse>(receiver.getNextCoreMessage(500));
    if (response == 0) {
        std::cout << "No response to initiate session" << std::endl;
        return 1;
    }
    else {
        std::cout << "Received initiate session response" << std::endl;
        std::cout << "Session initiation " << (response->isSuccessful() ? "was " : "was not ") << "successful" << std::endl;
    }
    ServiceRequest req(SESSION_ID, "TestMe");
    req.setParameter("Param 0", "Value 0");
    req.setParameter("Param 1", "Value 1");
    req.setParameter("Param 2", "Value 2");
    req.setChoreography("TestService");
    ServiceRequest reqError(SESSION_ID, "TestError");
    reqError.setChoreography("TestService");
    std::cout << std::endl << "Test case: successful service request" << std::endl;
    handleService(req, sender, receiver, dataReceiver);
    std::cout << std::endl << "Test case: unsuccessful service request" << std::endl;
    handleService(reqError, sender, receiver, dataReceiver);
    std::cout << std::endl << "Test case: Terminate session request" << std::endl;
    sender.sendCoreMessage(KillSessionRequest(SESSION_ID), SESSION_ID);
    boost::shared_ptr<KillSessionResponse> killResponse = boost::dynamic_pointer_cast<KillSessionResponse>(receiver.getNextCoreMessage(15000));
    if (killResponse == 0) {
        std::cout << "No response to kill session" << std::endl;
        return 1;
    }
    else {
        std::cout << "Received kill session response" << std::endl;
        std::cout << "Session termination " << (killResponse->isSuccessful() ? "was " : "was not ") << "successful" << std::endl;
    }
    return 0;
}
