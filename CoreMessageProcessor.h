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

#ifndef COREMESSAGEPROCESSOR_H_U9H3CPRK
#define COREMESSAGEPROCESSOR_H_U9H3CPRK

#include "CoreMessage.h"

class CoreMessageProcessor {
public:
    virtual const std::string queueServer() const = 0;
    virtual void setQueueServer(const std::string& queueServer) = 0;
    virtual const std::string sessionId() const = 0;
    virtual void setSessionId(const std::string& sessionId) = 0;
    virtual const bool shouldProcessMessage(const CoreMessage* message) = 0;
    virtual void processMessage(const CoreMessage* message) = 0;
};

#endif /* end of include guard: COREMESSAGEPROCESSOR_H_U9H3CPRK */
