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

#ifndef COREMESSAGESERIALIZER_H_KK0IAPGI
#define COREMESSAGESERIALIZER_H_KK0IAPGI

#include <boost/shared_ptr.hpp>
#include "CoreMessage.h"

class CoreMessageSerializer {
public:
    virtual CoreMessage* deserialize(const std::string& message) const = 0;
    virtual const std::string serialize(const CoreMessage& message) const = 0;
};

class CoreMessageXmlSerializer : public CoreMessageSerializer {
public:
    CoreMessageXmlSerializer();
    virtual CoreMessage* deserialize(const std::string& message) const;
    virtual const std::string serialize(const CoreMessage& message) const;
    virtual ~CoreMessageXmlSerializer();
private:
    struct XmlSerializerImpl;
    boost::shared_ptr<XmlSerializerImpl> impl_;
};

#endif /* end of include guard: COREMESSAGESERIALIZER_H_KK0IAPGI */
