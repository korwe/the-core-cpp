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
 * Message classes. These use xml for serialization.
 */

#ifndef COREMESSAGE_H_WBV376I1
#define COREMESSAGE_H_WBV376I1


#include <string>
#include <vector>
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>

class CoreMessage { 
    public:
        enum MessageType {
            UNKNOWN_MESSAGE_TYPE = -1,
            INITIATE_SESSION_REQUEST,
            KILL_SESSION_REQUEST,
            SERVICE_REQUEST,
            INITIATE_SESSION_RESPONSE,
            KILL_SESSION_RESPONSE,
            SERVICE_RESPONSE,
            DATA_RESPONSE
        };
        static const std::string lookupTypeName(MessageType messageType);
        static const MessageType lookupType(const std::string& messageTypeName);
        
        const std::string description() const;
        const std::string choreography() const;
        const MessageType messageType() const;
        const std::string sessionId() const;
        const std::string guid() const;
        const boost::posix_time::ptime& timeStamp() const;
        const std::string timeStampString() const;
        void updateTimeStamp();
        void setChoreography(const std::string& choreography);
        void setDescription(const std::string& description);
        void setGuid(const std::string& guid);
        void setTimeStamp(const boost::posix_time::ptime&);
        virtual ~CoreMessage();
    protected:
        CoreMessage(const std::string& sessionId, MessageType messageType);
    private:
        std::string _description;
        std::string _sessionId;
        std::string _choreography;
        std::string _guid;
        boost::posix_time::ptime _timeStamp;
        MessageType _messageType;
};

class CoreRequest : public CoreMessage {
    public:
        virtual ~CoreRequest();
    protected:
        CoreRequest(const std::string& sessionId, CoreMessage::MessageType messageType);
    private:
        void createGuid();
};

class CoreResponse : public CoreMessage {
    public:
        const bool isSuccessful() const;
        const std::string errorCode() const;
        const std::string errorMessage() const;
        void setSuccessful(const bool successful);
        void setErrorCode(const std::string& errorCode);
        void setErrorMessage(const std::string& errorMessage);
        virtual ~CoreResponse();
    protected:
        CoreResponse(const std::string& sessionId, const std::string& guid, CoreMessage::MessageType messageType);
    private:
        bool _successful;
        std::string _errorCode;
        std::string _errorMessage;
};

class InitiateSessionRequest : public CoreRequest {
    public:
        InitiateSessionRequest(const std::string& sessionId);
        virtual ~InitiateSessionRequest();
};

class InitiateSessionResponse : public CoreResponse {
    public:
        InitiateSessionResponse(const std::string& sessionId, const std::string& guid);
        virtual ~InitiateSessionResponse();
};

class KillSessionRequest : public CoreRequest {
    public:
        KillSessionRequest(const std::string& sessionId);
        virtual ~KillSessionRequest();
};

class KillSessionResponse : public CoreResponse {
    public:
        KillSessionResponse(const std::string& sessionId, const std::string& guid);
        virtual ~KillSessionResponse();
};

class ServiceRequest : public CoreRequest {
    public:
        ServiceRequest(const std::string& sessionId, const std::string& functionName);
        virtual ~ServiceRequest();
        const std::string functionName() const;
        const std::vector<std::string>& parameterNames() const;
        const std::string parameterValue(const std::string& name) const;
        void setParameter(const std::string& name, const std::string& value);
        const std::string location() const;
        void setLocation(const std::string& location);
    private:
        std::string _location;
        std::string _functionName;
        std::vector<std::string> _paramNames;
        std::map<std::string, std::string> _params; 
};

class ServiceResponse : public CoreResponse {
    public:
        ServiceResponse(const std::string& sessionId, const std::string& guid, bool hasData);
        virtual ~ServiceResponse();
        const bool hasData() const;
    private:
        bool _hasData;
};

class DataResponse : public CoreResponse {
    public:
        DataResponse(const std::string& sessionId, const std::string& guid, const std::string& data);
        virtual ~DataResponse();
        const std::string data() const;
    private:
        std::string _data; 
};


#endif /* end of include guard: COREMESSAGE_H_WBV376I1 */

