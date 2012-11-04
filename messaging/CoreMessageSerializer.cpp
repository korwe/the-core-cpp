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

#include <syslog.h>
#include <iostream>
#include <sstream>

#include "pugixml.hpp"
#include "CoreMessageSerializer.h"

struct CoreMessageXmlSerializer::XmlSerializerImpl {
    void setResponseFields(CoreResponse* response, const pugi::xml_node& msg) const {
        response->setSuccessful(boost::lexical_cast<bool>(msg.child_value("successful")));
        response->setErrorCode(msg.child_value("errorCode"));
        response->setErrorMessage(msg.child_value("errorMessage"));
    }

    void addResponseNodes(pugi::xml_node& xml, const CoreMessage& message) const {
        const CoreResponse response = dynamic_cast<const CoreResponse&>(message);
        xml.append_child().set_name("errorCode");
        xml.child("errorCode").append_child(pugi::node_pcdata).set_value(response.errorCode().c_str());
        xml.append_child().set_name("errorMessage");
        xml.child("errorMessage").append_child(pugi::node_pcdata).set_value(response.errorMessage().c_str());
        xml.append_child().set_name("successful");
        std::string success = boost::lexical_cast<std::string>(response.isSuccessful());
        xml.child("successful").append_child(pugi::node_pcdata).set_value(success.c_str());
    }
};

CoreMessageXmlSerializer::CoreMessageXmlSerializer() : impl_(new XmlSerializerImpl()) {
}

CoreMessageXmlSerializer::~CoreMessageXmlSerializer() {
}

 CoreMessage* CoreMessageXmlSerializer::deserialize(const std::string& message) const {
    pugi::xml_document doc;
    std::istringstream mstream;
    mstream.str(message);
    pugi::xml_parse_result result = doc.load(mstream);
    if (!result) {
        syslog(LOG_ERR, "Message is not well-formed XML");
        return (CoreMessage*) 0;
    }
    pugi::xml_node msg = doc.child("coreMessage");
    if (msg.empty()) {
        syslog(LOG_ERR, "Message is not a coreMessage");
        return (CoreMessage*) 0;        
    }
    if (msg.child("sessionId").empty()) {
        syslog(LOG_ERR, "Message has no sessionId");
        return (CoreMessage*) 0;        
    }
    std::string sessionId = msg.child_value("sessionId");
    if (msg.child("messageType").empty()) {
        syslog(LOG_ERR, "Message has no messageType");
        return (CoreMessage*) 0;        
    }
    std::string typeName = msg.child_value("messageType");
    if (msg.child("guid").empty()) {
        syslog(LOG_ERR, "Message has no guid");
        return (CoreMessage*) 0;        
    }
    std::string guid = msg.child_value("guid");
    CoreMessage* newMessage;
    switch (CoreMessage::lookupType(typeName)) {
        case CoreMessage::INITIATE_SESSION_REQUEST: {
            newMessage = new InitiateSessionRequest(sessionId);
            newMessage->setGuid(guid);
            break;
        }
        case CoreMessage::INITIATE_SESSION_RESPONSE: {
            InitiateSessionResponse* response = new InitiateSessionResponse(sessionId, guid);
            impl_->setResponseFields(response, msg);
            newMessage = response;
            break;
        }
        case CoreMessage::KILL_SESSION_REQUEST: {
            newMessage = new KillSessionRequest(sessionId);
            newMessage->setGuid(guid);
            break;
        }
        case CoreMessage::KILL_SESSION_RESPONSE: {
            KillSessionResponse* response = new KillSessionResponse(sessionId, guid);
            impl_->setResponseFields(response, msg);
            newMessage = response;
            break;
        }
        case CoreMessage::SERVICE_REQUEST: {
            std::string func = msg.child_value("function");
            ServiceRequest* serviceReq = new ServiceRequest(sessionId, func);
            serviceReq->setGuid(guid);
            pugi::xml_node params = msg.child("parameters");
            for (pugi::xml_node::iterator it = params.begin(); it != params.end(); ++it) {
                serviceReq->setParameter(it->child_value("name"), it->child_value("value"));
            }
            newMessage = serviceReq;
            break;
        }
        case CoreMessage::SERVICE_RESPONSE: {
            bool hasData = boost::lexical_cast<bool>(msg.child_value("hasData"));
            ServiceResponse* response = new ServiceResponse(sessionId, guid, hasData);
            impl_->setResponseFields(response, msg);
            newMessage = response;
            break;
        }
        case CoreMessage::DATA_RESPONSE: {
            std::string data = msg.child_value("data");
            DataResponse* response = new DataResponse(sessionId, guid, data);
            impl_->setResponseFields(response, msg);
            newMessage = response;
            break;
        }
        default:
            break; // Throw exception
    }
    newMessage->setChoreography(msg.child_value("choreography"));
    newMessage->setDescription(msg.child_value("description"));
    newMessage->setTimeStamp(boost::posix_time::from_iso_string(msg.child_value("timeStamp")));
    return newMessage;
}

const std::string CoreMessageXmlSerializer::serialize(const CoreMessage& message) const {
    pugi::xml_document doc;
    pugi::xml_node msgNode = doc.append_child();
    msgNode.set_name("coreMessage");
    msgNode.append_child().set_name("sessionId");
    msgNode.child("sessionId").append_child(pugi::node_pcdata).set_value(message.sessionId().c_str());
    msgNode.append_child().set_name("messageType");
    std::string typeName = CoreMessage::lookupTypeName(message.messageType());
    msgNode.child("messageType").append_child(pugi::node_pcdata).set_value(typeName.c_str());
    msgNode.append_child().set_name("guid");
    msgNode.child("guid").append_child(pugi::node_pcdata).set_value(message.guid().c_str());
    msgNode.append_child().set_name("choreography");
    msgNode.child("choreography").append_child(pugi::node_pcdata).set_value(message.choreography().c_str());
    msgNode.append_child().set_name("description");
    msgNode.child("description").append_child(pugi::node_pcdata).set_value(message.description().c_str());
    msgNode.append_child().set_name("timeStamp");
    msgNode.child("timeStamp").append_child(pugi::node_pcdata).set_value(message.timeStampString().c_str());

    switch (message.messageType()) {
        case CoreMessage::INITIATE_SESSION_RESPONSE: //Fall-through
        case CoreMessage::KILL_SESSION_RESPONSE: {
            impl_->addResponseNodes(msgNode, message);
            break;
        }
        case CoreMessage::SERVICE_REQUEST: {
            const ServiceRequest req = dynamic_cast<const ServiceRequest&>(message);
            msgNode.append_child().set_name("function");
            msgNode.child("function").append_child(pugi::node_pcdata).set_value(req.functionName().c_str());
            pugi::xml_node params = msgNode.append_child();
            params.set_name("parameters");
            std::vector<std::string>::const_iterator end = req.parameterNames().end();
            for (std::vector<std::string>::const_iterator it = req.parameterNames().begin();
                    it != end; ++it) {
                pugi::xml_node param = params.append_child();
                param.set_name("parameter");
                param.append_child().set_name("name");
                param.child("name").append_child(pugi::node_pcdata).set_value(it->c_str());
                param.append_child().set_name("value");
                param.child("value").append_child(pugi::node_pcdata).set_value(req.parameterValue(*it).c_str());
            }
            break;
        }
        case CoreMessage::SERVICE_RESPONSE: {
            impl_->addResponseNodes(msgNode, message);
            const ServiceResponse resp = dynamic_cast<const ServiceResponse&>(message);
            msgNode.append_child().set_name("hasData");
            std::string hasData = boost::lexical_cast<std::string>(resp.hasData());
            msgNode.child("hasData").append_child(pugi::node_pcdata).set_value(hasData.c_str());
            break;
        }
        case CoreMessage::DATA_RESPONSE: {
            impl_->addResponseNodes(msgNode, message);
            const DataResponse resp = dynamic_cast<const DataResponse&>(message);
            msgNode.append_child().set_name("data");
            msgNode.child("data").append_child(pugi::node_pcdata).set_value(resp.data().c_str());
            break;
        }
        default:
            break;
    }

    std::stringstream ss;
    pugi::xml_writer_stream writer(ss);
    doc.print(writer);
    return ss.str();
}

