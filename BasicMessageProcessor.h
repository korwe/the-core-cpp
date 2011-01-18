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

#ifndef BASICMESSAGEPROCESSOR_H_RS7DSGUO
#define BASICMESSAGEPROCESSOR_H_RS7DSGUO

#include "Messaging.h"
#include "CoreMessageProcessor.h"

class BasicMessageProcessor : public CoreMessageProcessor {
public:
    BasicMessageProcessor(std::string queueServer, std::string sessionId);
    virtual ~BasicMessageProcessor ();
    virtual const std::string queueServer() const;
    virtual void setQueueServer(const std::string& queueServer);
    virtual const std::string sessionId() const;
    virtual void setSessionId(const std::string& sessionId);
    virtual const bool shouldProcessMessage(const CoreMessage* message);
    virtual void processMessage(const CoreMessage* message);
    
private:
    std::string sessionId_;
    std::string queueServer_;
    MessageSender* serviceSender_;
    MessageSender* clientSender_;
    MessageSender* dataSender_;

};

#endif /* end of include guard: BASICMESSAGEPROCESSOR_H_RS7DSGUO */
