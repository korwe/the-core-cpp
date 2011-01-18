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


#include <CoreMessage.h>
#include <Messaging.h>


#define SERVICE_ID "TestService"
#define Q_SERVER "172.16.68.142"
//#define Q_SERVER "korwe.dnsalias.com"

int main (int argc, char const *argv[])
{
    MessageReceiver receiver(Q_SERVER, Queues::CORE_SERVICE, SERVICE_ID);
    MessageSender dataSender(Q_SERVER, Queues::CORE_DATA);
    MessageSender sender(Q_SERVER, Queues::SERVICE_CORE);
    
    while (true) {
        boost::shared_ptr<CoreMessage> msg = receiver.getNextCoreMessage(10000);
        if(msg != 0) {
            switch (msg->messageType()) {
                case CoreMessage::SERVICE_REQUEST: {
                    boost::shared_ptr<ServiceRequest> req = boost::dynamic_pointer_cast<ServiceRequest>(msg);
                    std::cout << "Function: " << req->functionName() << std::endl;
                    std::cout << "Parameters: " << std::endl;
                    std::vector<std::string>::const_iterator end = req->parameterNames().end();
                    for (std::vector<std::string>::const_iterator it = req->parameterNames().begin(); it != end; ++it) {
                        std::cout << *it << " = " << req->parameterValue(*it) << std::endl;
                    }
                    if ("TestError" == req->functionName()) {
                        ServiceResponse response(req->sessionId(), req->guid(), false);
                        response.setSuccessful(false);
                        response.setErrorCode("ERR00111");
                        response.setErrorMessage("Testing error response");
                        sender.sendCoreMessage(response, req->sessionId());
                    }
                    else {
                        ServiceResponse response(req->sessionId(), req->guid(), true);
                        DataResponse data(req->sessionId(), req->guid(), "<data>This is the data</data>");
                        sender.sendCoreMessage(response, req->sessionId());
                        dataSender.sendCoreMessage(data, req->sessionId());
                    }
                    break;
                }
                default: {
                    std::cout << "Invalid message type " << CoreMessage::lookupTypeName(msg->messageType());
                    break;
                }
            }
        }
    }
    
    return 0;
}
