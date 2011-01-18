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
 * CoreSession class
 */


#ifndef CORESESSION_H
#define CORESESSION_H

#include <boost/utility.hpp>
#include "CoreMessage.h"
#include "Messaging.h"
#include "CoreMessageProcessor.h"

class CoreSession : boost::noncopyable {
    public:
        CoreSession();
        CoreSession(std::string queue_server, std::string sessionId);
        ~CoreSession();
        void run();
        void stop();
        const std::string& sessionId() const;
    private:
        std::string _sessionId;
        std::string _server;
        MessageReceiver* _receiver;
        bool _stopRequested;
        CoreMessageProcessor* _processor;
};




#endif /* end of include guard: CORESESSION_H */
