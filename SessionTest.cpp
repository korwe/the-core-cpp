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
#include "Messaging.h"

#define Q_SERVER "172.16.68.132"

int main (int argc, char const *argv[]) {    
    std::string sessionId(argv[1]);
    std::cout << "Testing direct queue" << std::endl;
    MessageReceiver receiver(Q_SERVER, Queues::CORE_CLIENT, sessionId);
    MessageSender sender(Q_SERVER, Queues::CLIENT_CORE);
    std::cout << "Sending" << std::endl;
    sender.sendMessage("<coreMessage><sessionId>" + sessionId + "</sessionId><messageType>InitiateSessionRequest</messageType></coreMessage>", "123123123");
    std::cout << "Sent, receiving" << std::endl;
    std::cout << receiver.getNextMessage(500) << std::endl;

    for(int i = 0; i < 10; ++i)
    {
        std::cout << "Sending" << std::endl;
        sender.sendMessage("<coreMessage><sessionId>" + sessionId + "</sessionId><messageType>ServiceRequest</messageType></coreMessage>", "123123123");
        std::cout << "Sent, receiving" << std::endl;
        std::cout << receiver.getNextMessage(500) << std::endl;
   }

   std::cout << "Sending" << std::endl;
   sender.sendMessage("<coreMessage><sessionId>" + sessionId + "</sessionId><messageType>KillSessionRequest</messageType></coreMessage>", "123123123");
   std::cout << "Sent, receiving" << std::endl;
   std::cout << receiver.getNextMessage(500) << std::endl;

    return 0;
}
